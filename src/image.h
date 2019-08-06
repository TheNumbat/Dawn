
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
	
	object(object& o) {
		type = o.type;
		switch(type) {
		case obj::sphere: s = o.s; break;
		}
	}
	object(object&& o) {
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
};

v3 compute(ray r, object_list& list) {

	trace t = list.hit(r, {0.0f, FLT_MAX});
	if(t.hit) {
		return 0.5f * (t.normal + v3(1.0f));
	}

	v3 unit = norm(r.dir);
	f32 fade = 0.5f * (unit.y + 1.0f);
	return lerp(v3(0.5f,0.7f,1.0f), v3(1.0f), fade);
}

struct image {
	
	u64 render() {
		u64 start = SDL_GetPerformanceCounter();

		assert(_valid);
		u32* pixel = _data;

		f32 ar = (f32)_width / _height;
		v3 lower_left(-ar,1.0f,-1.0f);
		v3 horz(2.0f*ar,0.0f,0.0f);
		v3 vert(0.0f,-2.0f,0.0f);
		v3 origin;

		object_list list;
		list.objects.push_back(object::sphere({0.0f,0.0f,-1.0f},0.5f));
		list.objects.push_back(object::sphere({0.0f,-100.5f,-1.0f},100.0f));

		for(u32 y = 0; y < _height; y++) {
			f32 v = (f32)y / _height;

			for(u32 x = 0; x < _width; x++) {
				f32 u = (f32)x / _width;

				ray r{origin, lower_left + u*horz + v*vert};
				v3 col = compute(r, list);

				(*pixel++) = (0xff << 24) | 
							 ((u32)(255.0f * col.z) << 16) |
							 ((u32)(255.0f * col.y) << 8) |
							  (u32)(255.0f * col.x);
			}
		}

		u64 end = SDL_GetPerformanceCounter();
		return end - start;
	}



	image() {}
	static image make(u32 w, u32 h) {
		return image(w,h);
	}
	~image() {
		delete[] _data;
		_data = null;
		if(_handle) glDeleteTextures(1, &_handle);
		_width = _height = _handle = 0;
		_valid = _committed = false;
	}
	void operator=(image&& other) {
		this->~image();
		_valid = other._valid;
		_width = other._width;
		_height = other._height;
		_data = other._data;
		_handle = other._handle;
		other._width = other._height = other._handle = 0;
		other._valid = other._committed = false;
		other._data = null;
	}

	void commit() {
		assert(_valid);
		glBindTexture(GL_TEXTURE_2D, _handle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		_committed = true;
	}
	GLuint handle() {
		assert(_valid && _committed);
		return _handle;
	}

private:
	bool _valid = false, _committed = false;
	u32 _width = 0, _height = 0;
	u32* _data = nullptr;
	GLuint _handle = 0;

	image(u32 w, u32 h) : _width(w), _height(h), _data(new u32[_width*_height]()), _valid(true) {
		glGenTextures(1, &_handle);
	}
};
