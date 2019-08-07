
#pragma once

#include "math.h"

#include <vector>

struct trace_lane {
	f32_lane t, hit;
	v3_lane pos, normal;
	i32 mat = 0;

	void take_on(trace_lane o) {
		hit |= o.hit;
		
		t   &= ~o.hit;
		pos &= ~o.hit;
		normal &= ~o.hit;

		t |= o.t & o.hit;
		pos |= o.pos & o.hit;
		normal |= o.normal & o.hit;

		mat = o.mat;
	}
};

struct sphere {

	v3 pos;
	f32 rad = 0.0f;

	trace_lane hit(ray_lane& r, f32_lane tmin, f32_lane tmax) {
		
		trace_lane ret;
		v3_lane rel_pos = r.pos - pos;

		f32_lane a = lensq(r.dir);
		f32_lane b = 2.0f * dot(rel_pos, r.dir);
		f32_lane c = lensq(rel_pos) - rad*rad;
		f32_lane d = b*b - 4*a*c;
		
		f32_lane pos_mask = d > 0.0f;
		f32_lane neg_mask = ~pos_mask;

		f32_lane sqd = sqrt(d);
		f32_lane a2 = 2.0f * a;
		
		f32_lane result, mask;
		{ // d > 0.0f
			f32_lane temp = ((-b - sqd) / a2);
			f32_lane t_mask = pos_mask & (temp <= tmax) & (temp >= tmin);
			result |= t_mask & temp;
			mask   |= t_mask;
		}
		{
			f32_lane temp = ((-b + sqd) / a2);
			f32_lane t_mask = neg_mask & (temp <= tmax) & (temp >= tmin);	
			
			result |= t_mask & temp;
			mask   |= t_mask;
		}

		ret.hit    = mask;
		ret.t 	   = result;
		ret.pos    = r.get(result);
		ret.normal = (ret.pos - pos) / rad;

		return ret;
	}
};

enum class obj : u8 {
	none,
	sphere
};

struct object {
	obj type = obj::none;
	i32 mat = 0;
	union {
		sphere s;
	};
	static object sphere(i32 mat, v3 pos, f32 rad) {
		object ret;
		ret.mat = mat;
		ret.type = obj::sphere;
		ret.s = {pos,rad};
		return ret;
	}
	trace_lane hit(ray_lane& r, f32_lane tmin, f32_lane tmax) {
		trace_lane ret;
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
	trace_lane hit(ray_lane r, f32_lane tmin, f32_lane tmax) {
		
		trace_lane ret;
		f32_lane closest = tmax;

		for(object& o : objects) {

			trace_lane next = o.hit(r,tmin,closest);
			
			ret.take_on(next);
			closest &= ~next.hit;
			closest |= next.t & next.hit;
		}
		return ret;
	}
	void push(object o) {objects.push_back(o);}
	
private:
	std::vector<object> objects;
};
