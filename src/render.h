
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
	
	void init(i32 w, i32 h, i32 samples, bool use_ogl = true);
	void destroy();
	~renderer();

	void set_region(bool enable, i32 x, i32 y, i32 w, i32 h);
	u64 begin_render(const scene& s);
	bool finish();
	bool in_progress();
	f32 progress();

	void write_to_file(std::string file);
	void clear();
	void commit();

	GLuint handle = 0;

private:
	i32 width = 0, height = 0, samples = 0;
	u32* data = nullptr;

	bool region = false;
	i32 r_x = 0, r_y = 0, r_w = 0, r_h = 0;

	bool ogl = true;

	static const i32 Block_Size = 32;
	std::atomic<i32> tasks_complete = -1;
	i32 total_tasks = 0;
	thread_pool pool;

};
