
#pragma once

#include "math.h"
#include <vector>

// TODO:
	// new vec3 class? will need for SIMD

struct ray {
	v3 pos, dir;
	v3 get(f32 t) {return pos + t * dir;}
};

struct trace {
	bool hit = false;
	f32 t = 0.0f;
	v3 pos, normal;
};

struct sphere {

	v3 pos;
	f32 rad = 0.0f;

	trace hit(ray& r, v2 t) {
		trace ret;
		v3 rel_pos = r.pos - pos;
		f32 a = lensq(r.dir);
		f32 b = 2.0f * dot(rel_pos, r.dir);
		f32 c = lensq(rel_pos) - rad*rad;
		f32 d = b*b - 4*a*c;
		if(d > 0.0f) {
			f32 sqd = sqrtf(d);
			f32 result = (-b - sqd) / (2.0f * a);
			if(result <= t.y && result >= t.x) {
				ret.hit = true;
				ret.t = result;
				ret.pos = r.get(result);
				ret.normal = (ret.pos - pos) / rad;
			} else {
				result = (-b + sqd) / (2.0f * a);
				if(result <= t.y && result >= t.x) {
					ret.hit = true;
					ret.t = result;
					ret.pos = r.get(result);
					ret.normal = (ret.pos - pos) / rad;
				}
			}
		}
		return ret;
	}
};

enum class obj : u8 {
	sphere
};

struct object {
	obj type;
	union {
		sphere s;
	};
	static object sphere(v3 pos, f32 rad) {
		object ret;
		ret.type = obj::sphere;
		ret.s = {pos,rad};
		return ret;
	}
	trace hit(ray& r, v2 t) {
		switch(type) {
		case obj::sphere: return s.hit(r,t);
		}
		return {};
	}
	
	object(const object& _o) {
		object& o = const_cast<object&>(_o);
		type = o.type;
		switch(type) {
		case obj::sphere: s = o.s; break;
		}
	}
	object(const object&& _o) {
		object& o = const_cast<object&>(_o);
		type = o.type;
		switch(type) {
		case obj::sphere: s = o.s; break;
		}
	}
	object& operator=(object& o) {
		type = o.type;
		switch(type) {
		case obj::sphere: s = o.s; break;
		}
		return *this;
	}
	object& operator=(object&& o) {
		type = o.type;
		switch(type) {
		case obj::sphere: s = o.s; break;
		}
		return *this;
	}
private:
	object() {}
};

struct object_list {
	std::vector<object> objects;
	trace hit(ray r, v2 t) {
		trace ret;
		f32 closest = t.y;		
		for(object& o : objects) {
			trace next = o.hit(r,v2(t.x,closest));
			if(next.hit) {
				ret = next;
				closest = next.t;
			}
		}
		return ret;
	}
	void push(object o) {objects.push_back(o);}
};

struct camera {

	f32 ar;
	i32 width, height;
	v3 pos, lower_left, horz_step, vert_step;

	void init(i32 w, i32 h) {
		width = w; 
		height = h;
		ar = (f32)width / height;
		lower_left = v3(-ar,1.0f,-1.0f);
		horz_step = v3(2.0f*ar,0.0f,0.0f);
		vert_step = v3(0.0f,-2.0f,0.0f);
		pos = v3();
	}
	ray get_ray(f32 u, f32 v) {
		return {pos, lower_left + u*horz_step + v*vert_step - pos};
	}
};

struct scene {

	object_list list;
	camera cam;

	void init(i32 w, i32 h) {
		cam.init(w,h);
		list.push(object::sphere({0.0f,0.0f,-1.0f},0.5f));
		list.push(object::sphere({0.0f,-100.5f,-1.0f},100.0f));
	}
	v3 compute_pixel(f32 u, f32 v) {

		ray r = cam.get_ray(u,v);

		trace t = list.hit(r, {0.0f, FLT_MAX});
		if(t.hit) {
			return 0.5f * (t.normal + v3(1.0f));
		}

		v3 unit = norm(r.dir);
		f32 fade = 0.5f * (unit.y + 1.0f);
		return lerp(v3(0.5f,0.7f,1.0f), v3(1.0f), fade);
	}
};

struct image {
	
	u64 render(scene& s) {
		u64 start = SDL_GetPerformanceCounter();

		u32* pixel = data;

		for(u32 y = 0; y < height; y++) {
			f32 v = (f32)y / height;

			for(u32 x = 0; x < width; x++) {
				f32 u = (f32)x / width;

				v3 col = s.compute_pixel(u,v);

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
