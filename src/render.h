
#pragma once

#include <SDL2/SDL.h>
#include <glad.h>
#include <stb_image_write.h>

#include "lib/thread_pool.h"
#include "scene.h"

struct thread_data {
	u32* data = null;
	scene const* sc = null;
	i32 x,y,w,h,s;
	i32 total_w, total_h;
};

bool render_thread(thread_data data);

struct renderer {
	
	i32 width = 0, height = 0, samples = 0;
	u32* data = nullptr;

	bool ogl = true;
	GLuint handle = 0;

	static const i32 Block_Size = 32;
	std::atomic<i32> tasks_complete = -1;
	i32 total_tasks = 0;
	thread_pool pool;

	u64 begin_render(const scene& s);
	bool finish();
	bool in_progress();
	f32 progress();

	void init(i32 w, i32 h, i32 samples, bool use_ogl = true);
	void destroy();
	~renderer();

	void write_to_file(std::string file);
	void clear();
	void commit();
};
