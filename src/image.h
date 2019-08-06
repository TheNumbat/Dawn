
#pragma once

#include <SDL2/SDL.h>
#include "scene.h"

struct image {
	
	u64 render(scene& s) {
		u64 start = SDL_GetPerformanceCounter();

		u32* pixel = data;

		for(u32 y = 0; y < height; y++) {
			f32 v = (f32)y / height;

			for(u32 x = 0; x < width; x++) {
				f32 u = (f32)x / width;

				v3 col = s.pixel(u,v);

				(*pixel++) = (0xff << 24) | 
							 ((u32)(255.0f * col.z) << 16) |
							 ((u32)(255.0f * col.y) << 8) |
							  (u32)(255.0f * col.x);
			}
		}

		commit();
		u64 end = SDL_GetPerformanceCounter();
		return end - start;
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
	}
	~image() { destroy();}

	void commit() {
		glBindTexture(GL_TEXTURE_2D, handle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	u32 width = 0, height = 0;
	u32* data = nullptr;
	GLuint handle = 0;
};
