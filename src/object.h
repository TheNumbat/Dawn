
#pragma once

#include "math.h"

#include <vector>

typedef i32 mat_id;
struct trace {
	bool hit = false;
	f32 t = 0.0f;
	v3 pos, normal;
	mat_id mat = 0;
};

struct sphere {

	v3 pos;
	f32 rad = 0.0f;

	trace hit(ray& r, f32 tmin, f32 tmax) {
		trace ret;
		v3 rel_pos = r.pos - pos;
		f32 a = lensq(r.dir);
		f32 b = 2.0f * dot(rel_pos, r.dir);
		f32 c = lensq(rel_pos) - rad*rad;
		f32 d = b*b - 4*a*c;
		if(d > 0.0f) {
			f32 sqd = sqrtf(d);
			f32 result = (-b - sqd) / (2.0f * a);
			if(result <= tmax && result >= tmin) {
				ret.hit = true;
				ret.t = result;
				ret.pos = r.get(result);
				ret.normal = (ret.pos - pos) / rad;
			} else {
				result = (-b + sqd) / (2.0f * a);
				if(result <= tmax && result >= tmin) {
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
	mat_id mat = 0;
	union {
		sphere s;
	};
	static object sphere(mat_id mat, v3 pos, f32 rad) {
		object ret;
		ret.mat = mat;
		ret.type = obj::sphere;
		ret.s = {pos,rad};
		return ret;
	}
	trace hit(ray& r, f32 tmin, f32 tmax) {
		trace ret;
		switch(type) {
		case obj::sphere: ret = s.hit(r,tmin,tmax); break;
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

	void destroy() {objects.clear();}
	~object_list() {destroy();}
	trace hit(ray r, f32 tmin, f32 tmax) {
		trace ret;
		f32 closest = tmax;		
		for(object& o : objects) {
			trace next = o.hit(r,tmin,closest);
			if(next.hit) {
				ret = next;
				closest = next.t;
			}
		}
		return ret;
	}
	void push(object o) {objects.push_back(o);}
	
private:
	std::vector<object> objects;
};
