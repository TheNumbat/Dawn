
#pragma once

#include <vector>
#include <random>

#include "math.h"

std::random_device rd;
std::mt19937 rand_gen;
std::uniform_real_distribution<f32> dis;

void seed_rand() {
	rand_gen.seed(rd());
}
f32 randf_cpp() {
	return dis(rand_gen);
}
v3 random_leunit() {
	v3 v;
	do {
		v = 2.0f * v3(randf_cpp(),randf_cpp(),randf_cpp()) - v3(1.0f);
	} while(lensq(v) >= 1.0f);
	return v;
}

// TODO:
	// new vec3 class? will need for SIMD

struct ray {
	v3 pos, dir;
	v3 get(f32 t) {return pos + t * dir;}
};

struct material;
struct trace {
	bool hit = false;
	f32 t = 0.0f;
	v3 pos, normal;
	material* mat = null;
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
	none,
	sphere
};

struct object {
	obj type = obj::none;
	material* mat = null;
	union {
		sphere s;
	};
	static object sphere(material* mat, v3 pos, f32 rad) {
		object ret;
		ret.mat = mat;
		ret.type = obj::sphere;
		ret.s = {pos,rad};
		return ret;
	}
	trace hit(ray& r, v2 t) {
		trace ret;
		switch(type) {
		case obj::sphere: ret = s.hit(r,t); break;
		default: assert(false);
		}
		ret.mat = mat;
		return ret;
	}
	
	object(const object& o) {memcpy(this,&o,sizeof(object));}
	object(const object&& o) {memcpy(this,&o,sizeof(object));}
	void operator=(const object& o) {memcpy(this,&o,sizeof(object));}
	void operator=(const object&& o) {memcpy(this,&o,sizeof(object));}
	object() {}
};

struct object_list {
	std::vector<object> objects;

	void destroy() {objects.clear();}
	~object_list() {destroy();}
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
	iv2 dim;
	v3 pos, lower_left, horz_step, vert_step;

	void init(iv2 d) {
		dim = d;
		ar = (f32)dim.x / dim.y;
		lower_left = v3(-ar,1.0f,-1.0f);
		horz_step = v3(2.0f*ar,0.0f,0.0f);
		vert_step = v3(0.0f,-2.0f,0.0f);
		pos = v3();
	}
	ray get_ray(v2 uv, v2 jitter = {}) {
		jitter.x /= (f32)dim.x;
		jitter.y /= (f32)dim.y;
		uv += jitter;
		return {pos, lower_left + uv.x*horz_step + uv.y*vert_step - pos};
	}
};

struct scatter {
	bool absorbed = false;
	ray out;
	v3 attenuation;
};

struct lambertian {
	v3 albedo;

	scatter bsdf(ray& incoming, trace& surface) {
		scatter ret;
		v3 out = surface.pos + surface.normal + random_leunit();
		ret.out = {surface.pos, out - surface.pos};
		ret.attenuation = albedo;
		ret.absorbed = false;
		return ret;
	}
};

struct metal {
	v3 albedo;
	f32 rough = 0.0f;

	scatter bsdf(ray& incoming, trace& surface) {
		scatter ret;
		v3 r = reflect(norm(incoming.dir),surface.normal);
		ret.out = {surface.pos, r + rough * random_leunit()};
		ret.attenuation = albedo;
		ret.absorbed = dot(r, surface.normal) <= 0.0f;
		return ret;
	}
};

enum class mat : u8 {
	none,
	lambertian,
	metal
};

struct material {
	mat type = mat::none;
	union {
		lambertian l;
		metal m;
	};
	static material lambertian(v3 albedo) {
		material ret;
		ret.type = mat::lambertian;
		ret.l = {albedo};
		return ret;
	}
	static material metal(v3 albedo, f32 rough) {
		material ret;
		ret.type = mat::metal;
		ret.m = {albedo, rough};
		return ret;
	}
	scatter bsdf(ray& incoming, trace& surface) {
		switch(type) {
		case mat::lambertian: return l.bsdf(incoming,surface);
		case mat::metal: return m.bsdf(incoming,surface);
		default: assert(false);
		}
		return {};
	}
	
	material(const material& o) {memcpy(this,&o,sizeof(material));}
	material(const material&& o) {memcpy(this,&o,sizeof(material));}
	void operator=(const material& o) {memcpy(this,&o,sizeof(material));}
	void operator=(const material&& o) {memcpy(this,&o,sizeof(material));}
	material() {}
};

struct scene {

	object_list list;
	camera cam;
	i32 samples = 1, max_depth = 50;

	material lamb0, lamb1;
	material met0, met1;

	scene() {}
	void init(iv3 dim) {
		samples = dim.z;
		cam.init(dim.xy);

		lamb0 = material::lambertian(v3(0.8f,0.3f,0.3f));
		lamb1 = material::lambertian(v3(0.8f,0.8f,0.0f));

		met0 = material::metal(v3(0.8f,0.6f,0.2f),0.3f);
		met1 = material::metal(v3(0.8f,0.8f,0.8f),1.0f);

		list.push(object::sphere(&lamb0, {0.0f,0.0f,-1.0f},0.5f));
		list.push(object::sphere(&lamb1, {0.0f,-100.5f,-1.0f},100.0f));
		list.push(object::sphere(&met0, {1.0f,0.0f,-1.0f},0.5f));
		list.push(object::sphere(&met1, {-1.0f,0.0f,-1.0f},0.5f));
	}
	void destroy() {
		cam = {};
		list.destroy();
		samples = 1;
	}
	~scene() {destroy();}

	v3 compute(ray r, i32 depth = 0) {
		trace t = list.hit(r, {0.001f, FLT_MAX});
		if(t.hit) {
			if(depth >= max_depth) return v3();

			scatter s = t.mat->bsdf(r, t);

			if(s.absorbed) return v3();

			return s.attenuation * compute(s.out, depth+1);
		}

		v3 unit = norm(r.dir);
		f32 fade = 0.5f * (unit.y + 1.0f);
		return lerp(v3(0.5f,0.7f,1.0f), v3(1.0f), fade);
	}
	v3 pixel(v2 uv) {
		v3 result;
		for(i32 s = 0; s < samples; s++) {
			v2 jitter = v2(randf_cpp(),randf_cpp());
			ray r = cam.get_ray(uv, jitter);
			result += compute(r);
		}
		result /= (f32)samples;
		return pow(result, 1.0f / 2.2f);
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

				v3 col = s.pixel({u,v});

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
