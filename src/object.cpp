
#include "object.h"

#include <algorithm>

i32 bvh::node::populate(vec<object> list, vec<object>& objs, vec<node>& nodes, 
						f32 tmin, f32 tmax, i32 leaf_span, std::function<object(vec<object>)> create_leaf) {

	i32 axis = (i32)(randomf() * 3.0f);

	std::sort(list.begin(), list.end(), 
		[axis, tmin, tmax](const object& l, const object& r) -> bool {
		aabb lbox = l.box(tmin, tmax);
		aabb rbox = r.box(tmin, tmax);
		return lbox.min[axis] < rbox.min[axis];
	});

	node ret;

	assert(!list.empty());

	if(list.size <= leaf_span) {

		ret.type_ = type::leaf;

		objs.push(create_leaf(list));
		ret.left = objs.size - 1;
		ret.box_ = objs[ret.left].box(tmin, tmax);

	} else {
		
		ret.type_ = type::node;

		vec<object>::split split = list.halves();

		ret.left = populate(split.l, objs, nodes, tmin, tmax, leaf_span, create_leaf);
		ret.right = populate(split.r, objs, nodes, tmin, tmax, leaf_span, create_leaf);
		
		ret.box_ = aabb::enclose(nodes[ret.left].box_, nodes[ret.right].box_);
	}

	nodes.push(ret);
	return nodes.size - 1;
}

bvh bvh::make(vec<object> objs, f32 tmin, f32 tmax) {

	assert(!objs.empty());

	bvh ret;
	ret.root = node::populate(objs, ret.objects, ret.nodes, tmin, tmax, 1, 
		[](vec<object> list) -> object {
			return list[0];
		});

	return ret;
}

bvh bvh::make(vec<object> objs, f32 tmin, f32 tmax, i32 leaf_span,
			  std::function<object(vec<object>)> create_leaf) {

	assert(!objs.empty());

	bvh ret;
	ret.root = node::populate(objs, ret.objects, ret.nodes, tmin, tmax, leaf_span, create_leaf);

	return ret;	
}

void bvh::destroy() {
	objects.destroy();
	nodes.destroy();
	root = -1;
}

aabb bvh::box(f32, f32) const {
	assert(root >= 0 && root < nodes.size);
	return nodes[root].box_;
}

trace bvh::hit(const ray& ray, f32 tmin, f32 tmax) const {

	return hit_recurse(ray, root, tmin, tmax);
}

trace bvh::hit_recurse(const ray& ray, i32 idx, f32 tmin, f32 tmax) const {

	node current = nodes[idx];

	if(current.box_.hit(ray, tmin, tmax)) {

		trace l, r;
		if(current.type_ == node::type::node) {
			l = hit_recurse(ray, current.left, tmin, tmax);
			r = hit_recurse(ray, current.right, tmin, tmax);
		} else {
			return objects[current.left].hit(ray, tmin, tmax);
		}

		if(l.hit && r.hit) {
			if(l.t < r.t) return l;
			return r;
		}

		if(l.hit) return l;
		if(r.hit) return r;
	}
	return {};
}

aabb aabb::enclose(const aabb& l, const aabb& r) {
	return {vmin(l.min,r.min),vmax(l.max,r.max)};
}

bool aabb::hit(const ray& r, f32 tmin, f32 tmax) const {

	v3 inv_dir = 1.0f / r.dir;

	v3 _0 = (min - r.pos) * inv_dir;
	v3 _1 = (max - r.pos) * inv_dir;

	v3 t0 = vmax(vmin(_0,_1),tmin);
	v3 t1 = vmin(vmax(_0,_1),tmax);

	for(i32 i = 0; i < 3; i++) {
		tmin = fmaxf(t0[i], tmin);
		tmax = fminf(t1[i], tmax);
		if(tmax <= tmin) return false;
	}
	return true;
}

sphere sphere::make(v3 p, f32 r, i32 m) {
	sphere ret;
	ret.pos = p;
	ret.rad = r;
	ret.mat = m;
	return ret;
}

aabb sphere::box(f32, f32) const {
	return aabb{pos - rad, pos + rad};
}

trace sphere::hit(const ray& r, f32 tmin, f32 tmax) const {
	
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

sphere_moving sphere_moving::make(v3 p0, v3 p1, f32 r, i32 m, f32 t0, f32 t1) {
	sphere_moving ret;
	ret.pos0 = p0;
	ret.pos1 = p1;
	ret.rad  = r;
	ret.mat  = m;
	ret.start = t0;
	ret.duration = t1-t0;
	return ret;
}

aabb sphere_moving::box(f32 t0, f32 t1) const {
	v3 p0 = lerp(pos0, pos1, (t0 - start) / duration);
	v3 p1 = lerp(pos0, pos1, (t1 - start) / duration);
	return aabb::enclose(sphere::make(p0,rad,mat).box(t0,t1),sphere::make(p1,rad,mat).box(t0,t1));
}

trace sphere_moving::hit(const ray& r, f32 tmin, f32 tmax) const {

	v3 pos = lerp(pos0, pos1, (r.t - start) / duration);
	sphere s = sphere::make(pos, rad, mat);
	return s.hit(r, tmin, tmax);
}	

sphere_lane sphere_lane::make(v3_lane p, f32_lane r, f32_lane m) {
	sphere_lane ret;
	ret.pos = p;
	ret.rad = r;
	ret.mat = m;
	return ret;
}

aabb sphere_lane::box(f32, f32) const {
	v3_lane min = pos - rad;
	v3_lane max = pos + rad;
	return {hmin(min), hmax(max)};
}

trace sphere_lane::hit(const ray& r, f32 tmin, f32 tmax) const {
	
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
	f32_lane pos_t_mask = (pos_t <= tmax) & (pos_t >= tmin);
	f32_lane neg_t_mask = (neg_t <= tmax) & (neg_t >= tmin);
	
	f32_lane hit_mask = pos_t_mask | neg_t_mask;

	if(none(hit_mask)) return ret;

	f32_lane t_max{tmax};
	f32_lane t = vmin(select(pos_t,t_max,pos_t_mask), select(neg_t,t_max,neg_t_mask));

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

object_list object_list::make(vec<object>& objs) {
	object_list ret;
	ret.objects = vec<object>::take(objs);
	return ret;
}

void object_list::destroy() {
	objects.destroy();
}

aabb object_list::box(f32 t0, f32 t1) const {
	
	assert(!objects.empty());

	aabb ret = objects[0].box(t0, t1);
	for(const object& o : objects) {
		ret = aabb::enclose(ret,o.box(t0, t1));
	}
	return ret;
}

trace object_list::hit(const ray& r, f32 tmin, f32 tmax) const {
	
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

void sphere_lane_builder::clear() {
	idx = 0;
	rad = mat = {0.0f};
	pos = v3{0.0f};
}

void sphere_lane_builder::push(i32 m, v3 p, f32 r) {
	assert(idx < LANE_WIDTH);
	pos.set(idx, p);
	rad.f[idx] = r;
	mat.i[idx] = m;
	idx++;
}

void sphere_lane_builder::push(object o) {
	assert(idx < LANE_WIDTH);
	assert(o.type == obj::sphere);
	pos.set(idx, o.s.pos);
	rad.f[idx] = o.s.rad;
	mat.i[idx] = o.s.mat;
	idx++;
}

bool sphere_lane_builder::done() {
	return idx == LANE_WIDTH;
}

bool sphere_lane_builder::not_empty() {
	return idx > 0;
}

void sphere_lane_builder::fill() {
	assert(not_empty());
	while(idx < LANE_WIDTH) {
		pos.set(idx, pos[idx - 1]);
		rad.f[idx] = rad.f[idx - 1];
		mat.i[idx] = mat.i[idx - 1];
		idx++;
	}
}

object sphere_lane_builder::finish() {

	std::cout << idx << " " << std::endl;

	fill();
	assert(done());

	object lane = object::sphere_lane(mat,pos,rad);
	clear();
	return lane;
}

