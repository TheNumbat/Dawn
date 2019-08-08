
#pragma once

#include "image.h"

bool render_thread(thread_data data) {

	for(int y = data.y; y < data.y + data.h; y++) {
		f32 v = (f32)y / data.total_h;

		for(int x = data.x; x < data.x + data.w; x++) {
			f32 u = (f32)x / data.total_w;

			v3 col = data.s->pixel(u,v);

			data.data[y * data.total_w + x] = 
				(0xff << 24)                 | 
			   ((u32)(255.0f * col.z) << 16) |
			   ((u32)(255.0f * col.y) << 8)  |
				(u32)(255.0f * col.x);
		}
	}

	return true;
}

u64 image::begin_render(const scene& s) {

	u64 start = SDL_GetPerformanceCounter();

	clear();

	i32 w_blocks = width / Block_Size;
	i32 h_blocks = height / Block_Size;
	i32 w_remaining = width % Block_Size;
	i32 h_remaining = height % Block_Size;

	tasks_complete = 0;
	total_tasks = 0;

	for(i32 y = 0; y <= h_blocks; y++) {
		for(i32 x = 0; x <= w_blocks; x++) {

			thread_data t = {data, &s, x * Block_Size, y * Block_Size, 
							 x == w_blocks ? w_remaining : Block_Size, 
							 y == h_blocks ? h_remaining : Block_Size, 
							 width, height};

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

bool image::finish() {
	commit();
	if(tasks_complete.load() == total_tasks) {
		tasks_complete = -1;
		return true;
	}
	return false;
}

bool image::in_progress() {
	return tasks_complete.load() != -1;
}

f32 image::progress() {
	return (f32)tasks_complete.load() / total_tasks;
}

void image::init(u32 w, u32 h) {
	width = w;
	height = h;
	data = new u32[width*height]();
	glGenTextures(1, &handle);
	commit();
	pool.start(SDL_GetCPUCount());
}

void image::destroy() {
	pool.finish();
	delete[] data;
	data = null;
	if(handle) glDeleteTextures(1, &handle);
	width = height = handle = 0;
}

image::~image() {
	pool.kill(); 
	destroy();
}

void image::write_to_file(std::string file) {
	stbi_write_png(file.c_str(), width, height, 4, data, width * sizeof(u32));
}

void image::clear() {
	memset(data, 0, width * height * sizeof(u32));	
}

void image::commit() {
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}
