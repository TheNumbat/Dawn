
#pragma once

#include "lib/math.h"
#include "lib/vec.h"

#include <vector>
#include <functional>

struct object;

enum class obj : u8 {
	none = 0,
	bvh,
	list,
	sphere,
	sphere_moving,
	sphere_lane
};

struct trace {
	bool hit = false;
	f32 t = 0.0f;
	i32 mat = 0;
	v3 pos, normal;

	static trace min(const trace& l, const trace& r);
};

struct aabb {

	v3 min, max;

	static aabb enclose(const aabb& l, const aabb& r);
	bool hit(const ray& incoming, f32 tmin, f32 tmax) const;
};

struct bvh {

	static bvh make(const vec<object>& objs, f32 tmin, f32 tmax);
	static bvh make(const vec<object>& objs, f32 tmin, f32 tmax, i32 leaf_span,
					std::function<object(vec<object>)> create_leaf);
	void destroy();

	aabb box(f32 tmin, f32 tmax) const;
	trace hit(const ray& r, f32 tmin, f32 tmax) const;

private:

	enum class state : u8 {
		parent,
		sibling,
		child
	};

	struct node {
		enum class type : u8 {
			node,
			leaf
		};

		static i32 populate(const vec<object>& list, vec<object>& objs, vec<node>& nodes, 
							f32 tmin, f32 tmax, i32 leaf_span, std::function<object(vec<object>)> create_leaf);

		aabb box_;
		type type_ = type::node;

		// NOTE(max): note-> both populated with bvh::nodes, leaf-> left populated with object, right ignored 
		// (leaves just wrap single objects)
		i32 left = 0, right = 0, parent = 0;
	};

	i32 root = -1;
	vec<object> objects;
	vec<node> nodes;
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

	friend struct sphere_lane_builder;
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

struct object_list {

	// NOTE(max): takes ownership
	static object_list make(vec<object>& objs);
	void destroy();

	aabb box(f32 t0, f32 t1) const;
	trace hit(const ray& r, f32 tmin, f32 tmax) const;

private:
	vec<object> objects;
};

struct object {
	obj type = obj::none;
	union {
		bvh b;
		object_list l;
		sphere s;
		sphere_moving sm;
		sphere_lane sl;
	};

	// NOTE(max): takes ownership
	static object list(vec<object>& objs) {
		object ret;
		ret.type = obj::list;
		ret.l = object_list::make(objs);
		return ret;
	}
	static object bvh(vec<object> objs, f32 tmin, f32 tmax) {
		object ret;
		ret.type = obj::bvh;
		ret.b = bvh::make(objs, tmin, tmax);
		return ret;
	}
	static object bvh(vec<object> objs, f32 tmin, f32 tmax, 
					  i32 leaf_span, std::function<object(vec<object>)> create_leaf) {
		object ret;
		ret.type = obj::bvh;
		ret.b = bvh::make(objs, tmin, tmax, leaf_span, create_leaf);
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
		case obj::sphere_lane: sl.destroy(); break;
		case obj::sphere_moving: sm.destroy(); break;
		default: assert(false);
		}
	}
};

struct sphere_lane_builder {

	void clear();
	void push(i32 m, v3 p, f32 r);
	void push(object s);
	void fill();
	bool done();
	bool not_empty();
	object finish();

private:
	v3_lane pos;
	f32_lane rad, mat;
	i32 idx = 0;
};

