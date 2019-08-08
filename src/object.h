
#pragma once

#include "math.h"

#include <vector>

struct trace {
	bool hit = false;
	f32 t = 0.0f;
	i32 mat = 0;
	v3 pos, normal;
};

struct sphere {

	v3 pos;
	f32 rad = 0.0f;
	i32 mat = 0;

	trace hit(const ray& r, f32 tmin, f32 tmax) const {
		
		trace ret;
		v3 rel_pos = r.pos - pos;
		f32 a = lensq(r.dir);
		f32 b = 2.0f * dot(rel_pos, r.dir);
		f32 c = lensq(rel_pos) - rad*rad;
		f32 d = b*b - 4*a*c;

		if(d <= 0.0f) return ret;

		f32 sqd = sqrtf(d);

		f32 result = (-b - sqd) / (2.0f * a);
		if(result <= tmax && result >= tmin) {
			ret.hit = true;
			ret.mat = mat;
			ret.t = result;
			ret.pos = r.get(result);
			ret.normal = (ret.pos - pos) / rad;
			return ret;
		} 
		
		result = (-b + sqd) / (2.0f * a);
		if(result <= tmax && result >= tmin) {
			ret.hit = true;
			ret.mat = mat;
			ret.t = result;
			ret.pos = r.get(result);
			ret.normal = (ret.pos - pos) / rad;
		}
		return ret;
	}
};

struct sphere_lane {

	v3_lane pos;
	f32_lane rad;
	f32_lane mat;

	trace hit(const ray& r, f32 tmin, f32 tmax) const {
		
		trace ret;
		
		v3_lane rel_pos = r.pos - pos;
		f32_lane a = lensq(r.dir);
		f32_lane b = 2.0f * dot(rel_pos, r.dir);
		f32_lane c = lensq(rel_pos) - rad*rad;
		f32_lane d = b*b - 4.0f*a*c;

		f32_lane pos_mask = d > 0.0f;
		if(none(pos_mask)) return ret;

		f32_lane sqd = sqrt(d);

		f32_lane pos_t = (-b - sqd) / (2.0f * a);
		f32_lane neg_t = (-b + sqd) / (2.0f * a);
		f32_lane pos_t_mask = pos_mask & (pos_t <= tmax) & (pos_t >= tmin);
		f32_lane neg_t_mask = pos_mask & (neg_t <= tmax) & (neg_t >= tmin);
		
		f32_lane hit_mask = pos_t_mask | neg_t_mask;

		if(none(hit_mask)) return ret;

		f32_lane t_max{tmax};
		f32_lane t = min(select(pos_t,t_max,pos_t_mask), select(neg_t,t_max,neg_t_mask));

		ret.hit = true;
		ret.t = hmin(t);
		ret.pos = r.get(ret.t);

		// TODO(max): any way to not do that?
		i32 idx = 0;
		for(i32 i = 0; i < LANE_WIDTH; i++) {
			if(t.f[i] == ret.t) {
				idx = i;
				break;
			}
		}
		ret.normal = (ret.pos - pos[idx]) / rad.f[idx];
		ret.mat = mat.i[idx];

		return ret;
	}
};

enum class obj : u8 {
	none,
	sphere,
	sphere_lane
};

struct object {
	obj type = obj::none;
	union {
		sphere s;
		sphere_lane sl;
	};
	static object sphere(i32 mat, v3 pos, f32 rad) {
		object ret;
		ret.type = obj::sphere;
		ret.s = {pos,rad,mat};
		return ret;
	}
	static object sphere_lane(const f32_lane& mat, const v3_lane& pos, const f32_lane& rad) {
		object ret;
		ret.type = obj::sphere_lane;
		ret.sl = {pos,rad,mat};
		return ret;
	}
	trace hit(const ray& r, f32 tmin, f32 tmax) const {
		switch(type) {
		case obj::sphere: return s.hit(r,tmin,tmax);
		case obj::sphere_lane: return sl.hit(r,tmin,tmax);
		default: assert(false);
		}
		return {};
	}
	
	object(const object& o) {memcpy(this,&o,sizeof(object));}
	object(const object&& o) {memcpy(this,&o,sizeof(object));}
	void operator=(const object& o) {memcpy(this,&o,sizeof(object));}
	void operator=(const object&& o) {memcpy(this,&o,sizeof(object));}
	object() {}
};

struct sphere_lane_builder {

	v3_lane pos;
	f32_lane rad, mat;
	i32 idx = 0;

	void clear() {
		idx = 0;
		rad = mat = {0.0f};
		pos = v3{0.0f};
	}
	void push(i32 m, v3 p, f32 r) {
		assert(idx < LANE_WIDTH);
		pos.set(idx, p);
		rad.f[idx] = r;
		mat.i[idx] = m;
		idx++;
	}
	bool done() {
		return idx == LANE_WIDTH;
	}
	object finish() {
		object lane = object::sphere_lane(mat,pos,rad);
		clear();
		return lane;
	}
};

struct object_list {

	void destroy() {objects.clear();}
	~object_list() {destroy();}
	trace hit(const ray& r, f32 tmin, f32 tmax) const {
		
		trace ret;
		f32 closest = tmax;		
		for(const object& o : objects) {
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
