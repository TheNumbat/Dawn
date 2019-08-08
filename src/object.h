
#pragma once

#include "math.h"

#include <vector>

struct trace {
	bool hit = false;
	f32 t = 0.0f;
	i32 mat = 0;
	v3 pos, normal;
};

struct aabb {

	v3 min, max;

	static aabb enclose(const aabb& l, const aabb& r);
	bool hit(const ray& incoming, f32 tmin, f32 tmax) const;
};

struct sphere {

	v3 pos;
	f32 rad = 0.0f;
	i32 mat = 0;

	aabb box(f32, f32) const;
	trace hit(const ray& r, f32 tmin, f32 tmax) const;
};

struct sphere_moving {

	v3 pos0, pos1;
	f32 rad = 0.0f;
	i32 mat = 0;
	f32 start = 0.0f, duration = 0.0f;

	aabb box(f32 t0, f32 t1) const;
	trace hit(const ray& r, f32 tmin, f32 tmax) const;
};

struct sphere_lane {

	v3_lane pos;
	f32_lane rad;
	f32_lane mat;

	aabb box(f32, f32) const;
	trace hit(const ray& r, f32 tmin, f32 tmax) const;
};

enum class obj : u8 {
	none,
	list,
	sphere,
	sphere_moving,
	sphere_lane
};

struct object;
struct object_list {

	std::vector<object> objects;

	void destroy();

	aabb box(f32 t0, f32 t1) const;
	trace hit(const ray& r, f32 tmin, f32 tmax) const;
};

struct object {
	obj type = obj::none;
	union {
		object_list l;
		sphere s;
		sphere_moving sm;
		sphere_lane sl;
	};
	static object list(const std::vector<object>& l) {
		object ret;
		ret.type = obj::list;
		ret.l = {l};
		return ret;
	}
	static object sphere(i32 mat, v3 pos, f32 rad) {
		object ret;
		ret.type = obj::sphere;
		ret.s = {pos,rad,mat};
		return ret;
	}
	static object sphere_moving(i32 mat, v3 pos0, v3 pos1, f32 rad, f32 t0, f32 t1) {
		object ret;
		ret.type = obj::sphere_moving;
		ret.sm = {pos0,pos1,rad,mat,t0,t1-t0};
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
		case obj::list: return l.hit(r,tmin,tmax);
		case obj::sphere: return s.hit(r,tmin,tmax);
		case obj::sphere_lane: return sl.hit(r,tmin,tmax);
		case obj::sphere_moving: return sm.hit(r,tmin,tmax);
		default: assert(false);
		}
		return {};
	}
	aabb box(f32 t0, f32 t1) const {
		switch(type) {
		case obj::list: return l.box(t0, t1);
		case obj::sphere: return s.box(t0, t1);
		case obj::sphere_lane: return sl.box(t0, t1);
		case obj::sphere_moving: return sm.box(t0, t1);
		default: assert(false);
		}
		return {};
	}
	
	object(const object& o) {memcpy(this,&o,sizeof(object));}
	object(const object&& o) {memcpy(this,&o,sizeof(object));}
	void operator=(const object& o) {memcpy(this,&o,sizeof(object));}
	void operator=(const object&& o) {memcpy(this,&o,sizeof(object));}
	object() {}
	~object() { destroy(); }
	void destroy() {
		switch(type) {
		case obj::list: l.destroy(); break;
		case obj::sphere: break;
		case obj::sphere_lane: break;
		case obj::sphere_moving: break;
		default: assert(false);
		}
	}
};

struct sphere_lane_builder {

	v3_lane pos;
	f32_lane rad, mat;
	i32 idx = 0;

	void clear();
	void push(i32 m, v3 p, f32 r);
	bool done();
	bool not_empty();
	object finish();
};

