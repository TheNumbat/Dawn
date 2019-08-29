
#include "render.h"

bool render_thread(thread_data data) {

	for(i32 y = data.y; y < data.y + data.h; y++) {
		f32 v = (f32)y / data.total_h;

		for(i32 x = data.x; x < data.x + data.w; x++) {
			f32 u = (f32)x / data.total_w;

			v3 col;

			for(i32 s = 0; s < data.s; s++) {
				col += data.sc->sample({u,v});	
			}

			// TODO(max): tone mapping
			col = pow(clamp(col / (f32)data.s, 0.0f, 1.0f), 1.0f / 2.2f);

			u8 r = (u8)(col.x * 255.0f);
			u8 g = (u8)(col.y * 255.0f);
			u8 b = (u8)(col.z * 255.0f);

			data.data[y * data.total_w + x] = (0xff << 24) | (b << 16) | (g << 8) | r;
		}
	}

	return true;
}

u64 renderer::begin_render(const scene& s) {

	u64 start = SDL_GetPerformanceCounter();

	clear();

	i32 w = width;
	i32 h = height;
	i32 x0 = 0, y0 = 0;
	
	if(region) {
		w = r_w;
		h = r_h;
		x0 = r_x;
		y0 = r_y;
	}

	i32 w_blocks = w / Block_Size;
	i32 h_blocks = h / Block_Size;
	i32 w_remaining = w % Block_Size;
	i32 h_remaining = h % Block_Size;

	tasks_complete = 0;
	total_tasks = 0;

	for(i32 y = 0; y <= h_blocks; y++) {
		for(i32 x = 0; x <= w_blocks; x++) {

			thread_data t = {data, &s, x0 + x * Block_Size, y0 + y * Block_Size,
							 x == w_blocks ? w_remaining : Block_Size, 
							 y == h_blocks ? h_remaining : Block_Size,
							 samples, width, height};

			total_tasks++;

#ifdef USE_THREADING
			pool.enqueue([t,this] {render_thread(t); tasks_complete++;});
#else
			render_thread(t);
			tasks_complete++;
#endif
		}
	}

	return start;
}

bool renderer::finish() {
	commit();
	if(tasks_complete.load() == total_tasks) {
		tasks_complete = -1;
		return true;
	}
	return false;
}

bool renderer::in_progress() {
	return tasks_complete.load() != -1;
}

f32 renderer::progress() {
	return (f32)tasks_complete.load() / total_tasks;
}

void renderer::set_region(bool enable, i32 x, i32 y, i32 w, i32 h) {

	assert(x + w <= width && y + h <= height);

	region = enable;
	r_x = x;
	r_y = y;
	r_w = w;
	r_h = h;
}

void renderer::init(i32 w, i32 h, i32 s, bool use_ogl) {
	
	width = w;
	height = h;
	samples = s;

	data = new u32[width*height]();

	ogl = use_ogl;
	if(ogl) {
		glGenTextures(1, &handle);
		commit();
	}

	pool.start(SDL_GetCPUCount());
}

void renderer::destroy() {
	pool.finish();
	delete[] data;
	data = null;
	if(ogl && handle) glDeleteTextures(1, &handle);
	width = height = handle = 0;
}

renderer::~renderer() {
	pool.kill(); 
	destroy();
}

void renderer::write_to_file(std::string file) {
	stbi_write_png(file.c_str(), width, height, 4, data, width * sizeof(u32));
}

void renderer::clear() {
	memset(data, 0, width * height * sizeof(u32));	
}

void renderer::commit() {
	if(!ogl) return;
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}
