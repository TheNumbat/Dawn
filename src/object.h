
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
	bool hit(const ray& incoming, v2 t) const;
};

struct bvh {

	static bvh make(const vec<object>& objs, v2 t);
	static bvh make(const vec<object>& objs, v2 t, i32 leaf_span,
					std::function<object(vec<object>)> create_leaf);
	void destroy();

	aabb box(v2 t) const;
	trace hit(const ray& r, v2 t) const;

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

		static i16 populate(const vec<object>& list, vec<object>& objs, vec<node>& nodes, 
							v2 t, i32 leaf_span, std::function<object(vec<object>)> create_leaf);

		aabb box_;
		type type_ = type::node;

		// NOTE(max): note-> both populated with bvh::nodes, leaf-> left populated with object, right ignored 
		// (leaves just wrap single objects)
		i16 left = 0, right = 0, parent = 0;
	};

	i16 root = -1;
	vec<object> objects;
	vec<node> nodes;
};

struct sphere {

	static sphere make(v3 p, f32 r, i32 m);
	void destroy() {}

	static v2 map(v3 pos);
	v2 uv(v3 pos);

	aabb box(v2 t) const;
	trace hit(const ray& r, v2 t) const;

private:
	v3 pos;
	f32 rad = 0.0f;
	i32 mat = 0;

	friend struct sphere_lane_builder;
};

struct sphere_moving {

	static sphere_moving make(v3 p0, v3 p1, f32 r, i32 m, v2 t);
	void destroy() {}

	aabb box(v2 t) const;
	trace hit(const ray& r, v2 t) const;

private:
	v3 pos0, pos1;
	f32 rad = 0.0f;
	i32 mat = 0;
	v2 t;
};

struct sphere_lane {

	static sphere_lane make(v3_lane p, f32_lane r, f32_lane m);
	void destroy() {}

	aabb box(v2 t) const;
	trace hit(const ray& r, v2 t) const;

private:
	v3_lane pos;
	f32_lane rad;
	f32_lane mat;
};

struct object_list {

	// NOTE(max): takes ownership
	static object_list make(vec<object>& objs);
	void destroy();

	aabb box(v2 t) const;
	trace hit(const ray& r, v2 t) const;

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
	static object bvh(vec<object> objs, v2 t) {
		object ret;
		ret.type = obj::bvh;
		ret.b = bvh::make(objs, t);
		return ret;
	}
	static object bvh(vec<object> objs, v2 t, 
					  i32 leaf_span, std::function<object(vec<object>)> create_leaf) {
		object ret;
		ret.type = obj::bvh;
		ret.b = bvh::make(objs, t, leaf_span, create_leaf);
		return ret;
	}
	static object sphere(i32 mat, v3 pos, f32 rad) {
		object ret;
		ret.type = obj::sphere;
		ret.s = sphere::make(pos, rad, mat);
		return ret;
	}
	static object sphere_moving(i32 mat, v3 pos0, v3 pos1, f32 rad, v2 t) {
		object ret;
		ret.type = obj::sphere_moving;
		ret.sm = sphere_moving::make(pos0,pos1,rad,mat,t);
		return ret;
	}
	static object sphere_lane(const f32_lane& mat, const v3_lane& pos, const f32_lane& rad) {
		object ret;
		ret.type = obj::sphere_lane;
		ret.sl = sphere_lane::make(pos,rad,mat);
		return ret;
	}
	trace hit(const ray& r, v2 t) const {
		switch(type) {
		case obj::bvh: return b.hit(r, t);
		case obj::list: return l.hit(r, t);
		case obj::sphere: return s.hit(r, t);
		case obj::sphere_lane: return sl.hit(r, t);
		case obj::sphere_moving: return sm.hit(r, t);
		default: assert(false);
		}
		return {};
	}
	aabb box(v2 t) const {
		switch(type) {
		case obj::bvh: return b.box(t);
		case obj::list: return l.box(t);
		case obj::sphere: return s.box(t);
		case obj::sphere_lane: return sl.box(t);
		case obj::sphere_moving: return sm.box(t);
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

