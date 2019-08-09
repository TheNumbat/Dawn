
#pragma once

#include "object.h"

aabb aabb::enclose(const aabb& l, const aabb& r) {
	return {vmin(l.min,r.min),vmax(l.max,r.max)};
}

bool aabb::hit(const ray& incoming, f32 tmin, f32 tmax) const {

	v3 inv_dir = 1.0f / incoming.dir;

	v3 _0 = (min - incoming.pos) * inv_dir;
	v3 _1 = (max - incoming.pos) * inv_dir;

	v3 t0 = vmin(vmin(_0,_1),tmin);
	v3 t1 = vmax(vmax(_0,_1),tmax);

	return none(t1 <= t0);
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

aabb sphere_moving::box(f32 t0, f32 t1) const {
	v3 p0 = lerp(pos0, pos1, (t0 - start) / duration);
	v3 p1 = lerp(pos0, pos1, (t1 - start) / duration);
	return aabb::enclose(sphere{p0,rad,mat}.box(t0,t1),sphere{p1,rad,mat}.box(t0,t1));
}

trace sphere_moving::hit(const ray& r, f32 tmin, f32 tmax) const {

	v3 pos = lerp(pos0, pos1, (r.t - start) / duration);
	sphere s{pos, rad, mat};
	return s.hit(r, tmin, tmax);
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

void object_list::destroy() {
	objects.destroy();
}

aabb object_list::box(f32, f32) const {
	assert(false);
	return {};
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

bool sphere_lane_builder::done() {
	return idx == LANE_WIDTH;
}

bool sphere_lane_builder::not_empty() {
	return idx > 0;
}

object sphere_lane_builder::finish() {
	assert(not_empty());
	object lane = object::sphere_lane(mat,pos,rad);
	clear();
	return lane;
}

