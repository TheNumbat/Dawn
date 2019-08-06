
#pragma once

#include <SDL2/SDL.h>
#include "scene.h"
#include "threadpool.h"

struct thread_data {
	u32* data = null;
	scene* s = null;
	i32 x,y,w,h;
	i32 total_w, total_h;
};

bool render_thread(thread_data data) {

	std::cout << "block:(" << data.x << "," << data.y << "):(" << data.x + data.w << "," << data.y + data.h << ")" << std::endl;

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

struct image {
	
	i32 width = 0, height = 0;
	u32* data = nullptr;
	GLuint handle = 0;

	static const i32 Block_Size = 64;
	std::vector<std::vector<std::future<void>>> blocks;
	std::atomic<i32> tasks;
	ThreadPool pool;

	image() : tasks(-1), pool(SDL_GetCPUCount()) {}
	u64 begin_render(scene& s) {

		u64 start = SDL_GetPerformanceCounter();

		clear();
		blocks.clear();

		i32 w_blocks = width / Block_Size;
		i32 h_blocks = height / Block_Size;
		blocks.resize(h_blocks + 1);
		for(auto& row : blocks) {
			row.resize(w_blocks + 1);
		}

		i32 w_remaining = width % Block_Size;
		i32 h_remaining = height % Block_Size;

		std::cout << w_blocks + 1 << "x" << h_blocks + 1 << std::endl;

		tasks++;
		for(i32 y = 0; y <= h_blocks; y++) {
			for(i32 x = 0; x <= w_blocks; x++) {

				thread_data t = {data, &s, x * Block_Size, y * Block_Size, 
								 x == w_blocks ? w_remaining : Block_Size, 
								 y == h_blocks ? h_remaining : Block_Size, 
								 width, height};

				tasks++;
				blocks[y][x] = pool.enqueue([t,this] {render_thread(t); tasks--;});
			}
		}

		return start;
	}
	bool finish() {
		commit();
		if(tasks.load() == 0) {
			tasks--;
			return true;
		}
		return false;
	}

	void init(u32 w, u32 h) {
		width = w;
		height = h;
		data = new u32[width*height]();
		glGenTextures(1, &handle);
		commit();
	}
	void destroy() {
		delete[] data;
		data = null;
		if(handle) glDeleteTextures(1, &handle);
		width = height = handle = 0;
		blocks.clear();
	}
	~image() { destroy();}

	void clear() {
		memset(data, 0, width * height * sizeof(u32));	
	}
	void commit() {
		glBindTexture(GL_TEXTURE_2D, handle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
};
