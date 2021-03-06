
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
	sphere_lane,
	rect,
	box,
	volume
};

enum class plane : u8 {
	yz = 0,
	zx = 1,
	xy = 2
};

struct trace {
	bool hit = false;
	f32 t = 0.0f;
	i32 mat = 0;
	
	v2 uv;
	v3 pos, normal;

	static trace min(const trace& l, const trace& r);
	void transform(m4 trans, m4 norm);
};

struct aabb {

	v3 min, max;

	static aabb enclose(const aabb& l, const aabb& r);
	bool hit(const ray& incoming, v2 t) const;
	void transform(m4 trans);
};

struct volume {

	static volume make(i32 phase_mat, f32 density, object* bound);
	void destroy() {}

	aabb bbox(v2 t) const;
	trace hit(const ray& r, v2 t) const;

private:
	object* bound = null;
	f32 density = 0.0f;
	i32 phase_mat = 0;
};

struct rect {

	static rect make(i32 mat, plane type, v2 u, v2 v, f32 w);
	void destroy() {}

	aabb bbox(v2 t) const;
	trace hit(const ray& r, v2 t) const;

private:
	v2 u, v;
	f32 w = 0.0f;
	i32 mat = 0;
	plane type = plane::xy;
};

struct box {

	static box make(i32 mat, v3 min, v3 max);
	void destroy() {}

	aabb bbox(v2 t) const;
	trace hit(const ray& r, v2 t) const;

private:
	v3 min, max;
	rect sides[6];
};

struct bvh {

	static bvh make(const vec<object>& objs, v2 t);
	static bvh make(const vec<object>& objs, v2 t, i32 leaf_span,
					std::function<object(vec<object>)> create_leaf);
	void destroy();

	aabb bbox(v2 t) const;
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

	aabb bbox(v2 t) const;
	trace hit(const ray& r, v2 t) const;

private:
	
	v2 uv(const trace& info) const;

	v3 pos;
	f32 rad = 0.0f;
	i32 mat = 0;

	friend struct sphere_lane_builder;
};

struct sphere_moving {

	static sphere_moving make(v3 p0, v3 p1, f32 r, i32 m, v2 t);
	void destroy() {}

	aabb bbox(v2 t) const;
	trace hit(const ray& r, v2 t) const;

private:

	v2 uv(const trace& info) const;
	v3 center(f32 t) const;

	v3 pos0, pos1;
	f32 rad = 0.0f;
	i32 mat = 0;
	v2 time;
};

struct sphere_lane {

	static sphere_lane make(v3_lane p, f32_lane r, f32_lane m);
	void destroy() {}

	aabb bbox(v2 t) const;
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

	aabb bbox(v2 t) const;
	trace hit(const ray& r, v2 t) const;

private:
	vec<object> objects;
};

struct object {
	obj type = obj::none;

	m4 trans, itrans;
	bool do_trans = false;

	union {
		bvh b;
		object_list l;
		sphere s;
		sphere_moving sm;
		sphere_lane sl;
		rect re;
		box bx;
		volume v;
	};

	// NOTE(max): takes ownership
	static object list(vec<object>& objs, m4 t = m4::I) {
		object ret(obj::list, t);
		ret.l = object_list::make(objs);
		return ret;
	}
	static object box(i32 mat, v3 min, v3 max, m4 t = m4::I) {
		object ret(obj::box, t);
		ret.bx = box::make(mat, min, max);
		return ret;
	}
	static object rect(i32 mat, plane type, v2 u, v2 v, f32 w, m4 t = m4::I) {
		object ret(obj::rect, t);
		ret.re = rect::make(mat, type, u, v, w);
		return ret;
	}
	static object bvh(vec<object> objs, v2 t, m4 tr = m4::I) {
		object ret(obj::bvh, tr);
		ret.b = bvh::make(objs, t);
		return ret;
	}
	static object bvh(vec<object> objs, v2 t, 
					  i32 leaf_span, std::function<object(vec<object>)> create_leaf,
					  m4 tr = m4::I) {
		object ret(obj::bvh, tr);
		ret.b = bvh::make(objs, t, leaf_span, create_leaf);
		return ret;
	}
	static object sphere(i32 mat, v3 pos, f32 rad, m4 t = m4::I) {
		object ret(obj::sphere, t);
		ret.s = sphere::make(pos, rad, mat);
		return ret;
	}
	static object sphere_moving(i32 mat, v3 pos0, v3 pos1, f32 rad, v2 t, m4 tr = m4::I) {
		object ret(obj::sphere_moving, tr);
		ret.sm = sphere_moving::make(pos0,pos1,rad,mat,t);
		return ret;
	}
	static object sphere_lane(const f32_lane& mat, const v3_lane& pos, const f32_lane& rad, m4 t = m4::I) {
		object ret(obj::sphere_lane, t);
		ret.sl = sphere_lane::make(pos,rad,mat);
		return ret;
	}
	static object volume(i32 phase_mat, f32 density, object* bound, m4 t = m4::I) {
		object ret(obj::volume, t);
		ret.v = volume::make(phase_mat, density, bound);
		return ret;
	}

	trace hit(ray r, v2 t) const {

		if(do_trans) r.transform(itrans);

		trace ret;

		switch(type) {
		case obj::bvh: ret = b.hit(r, t); break;
		case obj::box: ret = bx.hit(r, t); break;
		case obj::list: ret = l.hit(r, t); break;
		case obj::rect: ret = re.hit(r, t); break;
		case obj::sphere: ret = s.hit(r, t); break;
		case obj::volume: ret = v.hit(r, t); break;
		case obj::sphere_lane: ret = sl.hit(r, t); break;
		case obj::sphere_moving: ret = sm.hit(r, t); break;
		default: assert(false);
		}

		if(do_trans) ret.transform(trans, transpose(itrans));

		return ret;
	}
	aabb bbox(v2 t) const {

		aabb ret;
		switch(type) {
		case obj::bvh: ret = b.bbox(t); break;
		case obj::box: ret = bx.bbox(t); break;
		case obj::list: ret = l.bbox(t); break;
		case obj::rect: ret = re.bbox(t); break;
		case obj::sphere: ret = s.bbox(t); break;
		case obj::volume: ret = v.bbox(t); break;
		case obj::sphere_lane: ret = sl.bbox(t); break;
		case obj::sphere_moving: ret = sm.bbox(t); break;
		default: assert(false);
		}

		if(do_trans) ret.transform(trans);

		return ret;
	}
	
	object(obj o, m4 t) {
		type = o;
		trans = t;
		do_trans = t != m4::I;
		if(do_trans) itrans = inverse_transform(t);
	}
	object() {memset(this,0,sizeof(object));}
	object(const object& o) {memcpy(this,&o,sizeof(object));}
	object(const object&& o) {memcpy(this,&o,sizeof(object));}
	object& operator=(const object& o) {memcpy(this,&o,sizeof(object)); return *this;}
	object& operator=(const object&& o) {memcpy(this,&o,sizeof(object)); return *this;}
	void destroy() {
		switch(type) {
		case obj::bvh: b.destroy(); break;
		case obj::list: l.destroy(); break;
		case obj::box: bx.destroy(); break;
		case obj::rect: re.destroy(); break;
		case obj::sphere: s.destroy(); break;
		case obj::volume: v.destroy(); break;
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

