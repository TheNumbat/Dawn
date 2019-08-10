
#pragma once

#include "lib/math.h"
#include "lib/vec.h"

#include <vector>
#include <functional>

class object;

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

struct bvh_node {

	static bvh_node make(vec<object> objs, f32 tmin, f32 tmax);
	
	aabb box(f32 tmin, f32 tmax) const;
	trace hit(const ray& r, f32 tmin, f32 tmax) const;		
	
private:
	aabb box_;
	object* left = null;
	object* right = null;
};

struct bvh {

	bvh_node root;
	
	// TODO(max): is indexing by ID faster than pointer?
	vec<object> objects;

	// NOTE(max): takes ownership
	static bvh make(vec<object>& objs, f32 tmin, f32 tmax);
	void destroy();

	aabb box(f32 tmin, f32 tmax) const;
	trace hit(const ray& r, f32 tmin, f32 tmax) const;
};

struct sphere {

	static sphere make(v3 p, f32 r, i32 m);
	void destroy() {}

	aabb box(f32 tmin, f32 tmax) const;
	trace hit(const ray& r, f32 tmin, f32 tmax) const;

private:
	v3 pos;
	f32 rad = 0.0f;
	i32 mat = 0;
};

struct sphere_moving {

	static sphere_moving make(v3 p0, v3 p1, f32 r, i32 m, f32 t0, f32 t1);
	void destroy() {}

	aabb box(f32 t0, f32 t1) const;
	trace hit(const ray& r, f32 tmin, f32 tmax) const;

private:
	v3 pos0, pos1;
	f32 rad = 0.0f;
	i32 mat = 0;
	f32 start = 0.0f, duration = 0.0f;
};

struct sphere_lane {

	static sphere_lane make(v3_lane p, f32_lane r, f32_lane m);
	void destroy() {}

	aabb box(f32 t0, f32 t1) const;
	trace hit(const ray& r, f32 tmin, f32 tmax) const;

private:
	v3_lane pos;
	f32_lane rad;
	f32_lane mat;
};

enum class obj : u8 {
	none = 0,
	bvh,
	bvh_node,
	list,
	sphere,
	sphere_moving,
	sphere_lane
};

struct object_list {

	// NOTE(max): takes ownership
	static object_list make(vec<object>& objs);
	void destroy();

	aabb box(f32 t0, f32 t1) const;
	trace hit(const ray& r, f32 tmin, f32 tmax) const;

private:
	vec<object> objects;
};

class object {
	union {
		bvh b;
		bvh_node n;
		object_list l;
		sphere s;
		sphere_moving sm;
		sphere_lane sl;
	};

public:
	obj type = obj::none;

	// NOTE(max): takes ownership
	static object list(vec<object>& objs) {
		object ret;
		ret.type = obj::list;
		ret.l = object_list::make(objs);
		return ret;
	}
	// NOTE(max): takes ownership
	static object bvh(vec<object>& objs, f32 tmin, f32 tmax) {
		object ret;
		ret.type = obj::bvh;
		ret.b = bvh::make(objs, tmin, tmax);
		return ret;
	}
	static object bvh_node(vec<object> objs, f32 tmin, f32 tmax) {
		object ret;
		ret.type = obj::bvh_node;
		ret.n = bvh_node::make(objs, tmin, tmax);
		return ret;
	}
	static object sphere(i32 mat, v3 pos, f32 rad) {
		object ret;
		ret.type = obj::sphere;
		ret.s = sphere::make(pos, rad, mat);
		return ret;
	}
	static object sphere_moving(i32 mat, v3 pos0, v3 pos1, f32 rad, f32 t0, f32 t1) {
		object ret;
		ret.type = obj::sphere_moving;
		ret.sm = sphere_moving::make(pos0,pos1,rad,mat,t0,t1);
		return ret;
	}
	static object sphere_lane(const f32_lane& mat, const v3_lane& pos, const f32_lane& rad) {
		object ret;
		ret.type = obj::sphere_lane;
		ret.sl = sphere_lane::make(pos,rad,mat);
		return ret;
	}
	trace hit(const ray& r, f32 tmin, f32 tmax) const {
		switch(type) {
		case obj::bvh: return b.hit(r, tmin, tmax);
		case obj::list: return l.hit(r, tmin, tmax);
		case obj::sphere: return s.hit(r, tmin, tmax);
		case obj::bvh_node: return b.hit(r, tmin, tmax);
		case obj::sphere_lane: return sl.hit(r, tmin, tmax);
		case obj::sphere_moving: return sm.hit(r, tmin, tmax);
		default: assert(false);
		}
		return {};
	}
	aabb box(f32 t0, f32 t1) const {
		switch(type) {
		case obj::bvh: return b.box(t0, t1);
		case obj::list: return l.box(t0, t1);
		case obj::sphere: return s.box(t0, t1);
		case obj::bvh_node: return b.box(t0, t1);
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
	object() {memset(this,0,sizeof(object));}
	void destroy() {
		switch(type) {
		case obj::bvh: b.destroy(); break;
		case obj::list: l.destroy(); break;
		case obj::sphere: s.destroy(); break;
		case obj::bvh_node: b.destroy(); break;
		case obj::sphere_lane: sl.destroy(); break;
		case obj::sphere_moving: sm.destroy(); break;
		default: assert(false);
		}
	}
};

struct sphere_lane_builder {

	void clear();
	void push(i32 m, v3 p, f32 r);
	bool done();
	bool not_empty();
	object finish();

private:
	v3_lane pos;
	f32_lane rad, mat;
	i32 idx = 0;
};

