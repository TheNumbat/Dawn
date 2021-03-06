
#include "object.h"

#include <algorithm>

void trace::transform(m4 trans, m4 norm) {

	pos = (trans * v4(pos, 1.0f)).xyz;
	normal = (norm * v4(normal, 0.0f)).xyz;
}

trace trace::min(const trace& l, const trace& r) {

	if(l.hit && r.hit) {
		if(l.t < r.t) return l;
		return r;
	}
	if(l.hit) return l;
	if(r.hit) return r;
	return {};
}

box box::make(i32 mat, v3 min, v3 max) {

	box ret;
	ret.min = min;
	ret.max = max;

	ret.sides[0] = rect::make(mat, plane::xy, {min.x, max.x}, {min.y, max.y}, max.z);
	ret.sides[1] = rect::make(mat, plane::xy, {min.x, max.x}, {min.y, max.y}, min.z);

	ret.sides[2] = rect::make(mat, plane::zx, {min.z, max.z}, {min.x, max.x}, max.y);
	ret.sides[3] = rect::make(mat, plane::zx, {min.z, max.z}, {min.x, max.x}, min.y);

	ret.sides[4] = rect::make(mat, plane::yz, {min.y, max.y}, {min.z, max.z}, max.x);
	ret.sides[5] = rect::make(mat, plane::yz, {min.y, max.y}, {min.z, max.z}, min.x);

	return ret;
}

aabb box::bbox(v2) const {

	return {min, max};
}

trace box::hit(const ray& r, v2 t) const {

	trace ret;
	f32 closest = t.y;
	for(const rect& re : sides) {
		trace next = re.hit(r, {t.x, closest});
		if(next.hit) {
			ret = next;
			closest = next.t;
		}
	}
	return ret;	
}

volume volume::make(i32 phase_mat, f32 density, object* bound) {
	volume ret;
	ret.phase_mat = phase_mat;
	ret.density = density;
	ret.bound = bound;
	return ret;
}

aabb volume::bbox(v2 t) const {
	return bound->bbox(t);
}

trace volume::hit(const ray& r, v2 t) const {

	trace ret;

	trace b0 = bound->hit(r, {-FLT_MAX, FLT_MAX});
	if(b0.hit) {

		trace b1 = bound->hit(r, {b0.t + 0.0001f, FLT_MAX});
		if(b1.hit) {

			b0.t = max1(b0.t, t.x);
			b1.t = min1(b1.t, t.y);
			if(b0.t >= b1.t) return ret;

			b0.t = max1(b0.t, 0.0f);
			
			f32 dlen = len(r.dir);
			f32 d = (b1.t - b0.t) * dlen;
			f32 h = - (1.0f / density) * logf(randomf());

			if(h < d) {

				ret.hit = true;
				ret.t = b0.t + h / dlen;
				ret.pos = r.get(ret.t);
				ret.mat = phase_mat;
			}
		}
	}

	return ret;
}

rect rect::make(i32 mat, plane type, v2 u, v2 v, f32 w) {
	rect ret;
	ret.type = type;
	ret.mat = mat;
	ret.u = u;
	ret.v = v;
	ret.w = w;
	return ret;
}

aabb rect::bbox(v2) const {

	switch(type) {
	case plane::yz: {
		return {{w - 0.0001f, u.x, v.x},
				{w + 0.0001f, u.y, v.y}};
	} break;
	case plane::zx: {
		return {{v.x, w - 0.0001f, u.x},
				{v.y, w + 0.0001f, u.y}};
	} break;
	case plane::xy: {
		return {{u.x, v.x, w - 0.0001f},
				{u.y, v.y, w + 0.0001f}};
	} break;

	default: assert(false);
	}
	return {};
}

trace rect::hit(const ray& r, v2 t) const {

	trace ret;

	u8 w_idx = (u8)type;
	u8 u_idx = ((u8)type + 1) % 3;
	u8 v_idx = ((u8)type + 2) % 3;

	f32 t_pos = (w - r.pos[w_idx]) / r.dir[w_idx];

	if(t_pos < t.x || t_pos > t.y) return ret;

	v3 at = r.get(t_pos);

	if(at[u_idx] < u.x || at[u_idx] > u.y || at[v_idx] < v.x || at[v_idx] > v.y) return ret;

	ret.hit = true;
	ret.t = t_pos;
	ret.mat = mat;
	ret.uv = {(at[u_idx] - u.x) / (u.y - u.x), (at[v_idx] - v.x) / (v.y - v.x)};
	ret.pos = at;

	// NOTE(max): double sided plane
	ret.normal[w_idx] = 1.0f;
	ret.normal[w_idx] = dot(ret.normal, r.dir) < 0.0f ? 1.0f : -1.0f;

	return ret;
}

i16 bvh::node::populate(const vec<object>& list, vec<object>& objs, vec<node>& nodes, 
						v2 t, i32 leaf_span, std::function<object(vec<object>)> create_leaf) {

	i32 axis = randomu() % 3;

	std::sort(list.begin(), list.end(), 
		[axis, t](const object& l, const object& r) -> bool {
		aabb lbox = l.bbox(t);
		aabb rbox = r.bbox(t);
		return lbox.min[axis] < rbox.min[axis];
	});

	node ret;
	i16 idx = 0;

	assert(!list.empty() && list.size < UINT16_MAX);

	if(list.size <= leaf_span) {

		ret.type_ = type::leaf;

		objs.push(create_leaf(list));
		ret.left = (i16)(objs.size - 1);
		ret.box_ = objs[ret.left].bbox(t);

		nodes.push(ret);
		idx = (i16)(nodes.size - 1);

	} else {
		
		ret.type_ = type::node;

		vec<object>::split split = list.halves();

		ret.left = populate(split.l, objs, nodes, t, leaf_span, create_leaf);
		ret.right = populate(split.r, objs, nodes, t, leaf_span, create_leaf);
		ret.box_ = aabb::enclose(nodes[ret.left].box_, nodes[ret.right].box_);

		// TODO(max): can we make this a complete tree with implicit parent/children position?
		nodes.push(ret);
		idx = (i16)(nodes.size - 1);
		nodes[ret.left].parent = idx;
		nodes[ret.right].parent = idx;
	}

	return idx;
}

bvh bvh::make(const vec<object>& objs, v2 t) {

	assert(!objs.empty());

	bvh ret;
	ret.root = node::populate(objs, ret.objects, ret.nodes, t, 1, 
		[](vec<object> list) -> object {
			return list[0];
		});

	return ret;
}

bvh bvh::make(const vec<object>& objs, v2 t, i32 leaf_span,
			  std::function<object(vec<object>)> create_leaf) {

	assert(!objs.empty());

	bvh ret;
	ret.root = node::populate(objs, ret.objects, ret.nodes, t, leaf_span, create_leaf);

	return ret;	
}

void bvh::destroy() {
	objects.destroy();
	nodes.destroy();
	root = -1;
}

aabb bvh::bbox(v2) const {
	assert(root >= 0 && root < nodes.size);
	return nodes[root].box_;
}

// NOTE(max): http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.445.7529&rep=rep1&type=pdf
trace bvh::hit(const ray& r, v2 t) const {

	assert(root >= 0 && root < nodes.size);

	trace result;
	
	// Current node index
	i16 idx = nodes[root].left;

	// Where we are visiting this node from
	state s = state::parent;

	for(;;) {
		switch(s) {

		// Visit left nodes from their parent
		case state::parent: {
			
			node current = nodes[idx];

			if(current.box_.hit(r, t)) {
				if(current.type_ == node::type::leaf) {
					
					result = trace::min(result, objects[current.left].hit(r, t));
				
					// Finished, traverse right subtree
					idx = nodes[current.parent].right;
					s = state::sibling;

				} else {

					// Traverse left subtree
					idx = current.left;
					s = state::parent;
				}
			} else {

				// Finished, traverse right subtree
				idx = nodes[current.parent].right;
				s = state::sibling;
			}
		} break;

		// Visit right nodes from their left node sibling
		case state::sibling: {

			node current = nodes[idx];

			if(current.box_.hit(r, t)) {
				if(current.type_ == node::type::leaf) {

					result = trace::min(result, objects[current.left].hit(r, t));

					// Finished, go up
					idx = current.parent;
					s = state::child;

				} else {

					// Traverse left subtree
					idx = current.left;
					s = state::parent;
				}
			} else {

				// Fished, go up
				idx = current.parent;
				s = state::child;
			}
		} break;

		// Visit all nodes from their children when traversing back up the tree
		case state::child: {
			
			// Traversal complete
			if(idx == root) return result;
			
			i16 p_idx = nodes[idx].parent;
			node parent = nodes[p_idx];

			// Traverse right subtree
			if(idx == parent.left) {

				idx = parent.right;
				s = state::sibling;

			// Finished, go up
			} else {

				idx = p_idx;
				s = state::child;
			}
		} break;
		}
	}
}

void aabb::transform(m4 trans) {

	v3 amin = min, amax = max;
	min = max = trans[3].xyz;

	for(i32 i = 0; i < 3; i++) {
		for(i32 j = 0; j < 3; j++) {

			f32 a = trans[j][i] * amin[j];
			f32 b = trans[j][i] * amax[j];

			if(a < b) {
				min[i] += a; 
				max[i] += b;
			} else {
				min[i] += b; 
				max[i] += a;
			}
		}
	}
}

aabb aabb::enclose(const aabb& l, const aabb& r) {
	return {vmin(l.min,r.min),vmax(l.max,r.max)};
}

bool aabb::hit(const ray& r, v2 t) const {

	v3 inv_dir = 1.0f / r.dir;

	v3 _0 = (min - r.pos) * inv_dir;
	v3 _1 = (max - r.pos) * inv_dir;

	v3 t0 = vmax(vmin(_0,_1),t.x);
	v3 t1 = vmin(vmax(_0,_1),t.y);

	for(i32 i = 0; i < 3; i++) {
		t.x = t0[i] < t.x ? t.x : t0[i];
		t.y = t1[i] < t.y ? t1[i] : t.y;
		if(t.y <= t.x) return false;
	}
	return true;
}

v2 sphere::map(v3 pos) {

	f32 phi = atan2(pos.z, pos.x);
	f32 theta = asin(pos.y);
	return {1.0f - (phi + PI32) / TAU32, (theta + PI32 / 2.0f) / PI32};
}

v2 sphere::uv(const trace& info) const {
	return map((pos - info.pos) / rad);
}

sphere sphere::make(v3 p, f32 r, i32 m) {
	sphere ret;
	ret.pos = p;
	ret.rad = r;
	ret.mat = m;
	return ret;
}

aabb sphere::bbox(v2) const {
	return aabb{pos - rad, pos + rad};
}

trace sphere::hit(const ray& r, v2 t) const {
	
	trace ret;
	v3 rel_pos = r.pos - pos;
	f32 a = lensq(r.dir);
	f32 b = 2.0f * dot(rel_pos, r.dir);
	f32 c = lensq(rel_pos) - rad*rad;
	f32 d = b*b - 4*a*c;

	if(d <= 0.0f) return ret;

	f32 sqd = sqrtf(d);

	f32 result = (-b - sqd) / (2.0f * a);
	if(result <= t.y && result >= t.x) {
		ret.hit = true;
		ret.mat = mat;
		ret.t = result;
		ret.pos = r.get(result);
		ret.normal = (ret.pos - pos) / rad;
		ret.uv = map(-ret.normal);
		return ret;
	} 
	
	result = (-b + sqd) / (2.0f * a);
	if(result <= t.y && result >= t.x) {
		ret.hit = true;
		ret.mat = mat;
		ret.t = result;
		ret.pos = r.get(result);
		ret.normal = (ret.pos - pos) / rad;
		ret.uv = map(-ret.normal);
	}
	return ret;
}

sphere_moving sphere_moving::make(v3 p0, v3 p1, f32 r, i32 m, v2 t) {
	sphere_moving ret;
	ret.pos0 = p0;
	ret.pos1 = p1;
	ret.rad  = r;
	ret.mat  = m;
	ret.time.x = t.x;
	ret.time.y = t.y - t.x;
	return ret;
}

v3 sphere_moving::center(f32 t) const {

	f32 dist = clamp((t - time.x) / time.y, 0.0f, 1.0f);
	return lerp(pos0, pos1, dist);
}

v2 sphere_moving::uv(const trace& info) const {
	
	return sphere::map((center(info.t) - info.pos) / rad);
}

aabb sphere_moving::bbox(v2 t) const {

	return aabb::enclose(sphere::make(center(t.x),rad,mat).bbox(t),
						 sphere::make(center(t.y),rad,mat).bbox(t));
}

trace sphere_moving::hit(const ray& r, v2 t) const {

	sphere s = sphere::make(center(r.t), rad, mat);
	return s.hit(r, t);
}	

sphere_lane sphere_lane::make(v3_lane p, f32_lane r, f32_lane m) {
	sphere_lane ret;
	ret.pos = p;
	ret.rad = r;
	ret.mat = m;
	return ret;
}

aabb sphere_lane::bbox(v2) const {
	v3_lane min = pos - rad;
	v3_lane max = pos + rad;
	return {hmin(min), hmax(max)};
}

trace sphere_lane::hit(const ray& r, v2 t) const {
	
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
	f32_lane pos_t_mask = (pos_t <= t.y) & (pos_t >= t.x);
	f32_lane neg_t_mask = (neg_t <= t.y) & (neg_t >= t.x);
	
	f32_lane hit_mask = pos_t_mask | neg_t_mask;

	if(none(hit_mask)) return ret;

	f32_lane t_max{t.y};
	f32_lane _t = vmin(select(pos_t,t_max,pos_t_mask), select(neg_t,t_max,neg_t_mask));

	ret.hit = true;
	ret.t = hmin(_t);
	ret.pos = r.get(ret.t);

	// TODO(max): any way to not do that?
	i32 idx = 0;
	for(i32 i = 0; i < LANE_WIDTH; i++) {
		if(_t.f[i] == ret.t) {
			idx = i;
			break;
		}
	}
	ret.normal = (ret.pos - pos[idx]) / rad.f[idx];
	ret.uv = sphere::map(-ret.normal);
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

aabb object_list::bbox(v2 t) const {
	
	assert(!objects.empty());

	aabb ret = objects[0].bbox(t);
	for(const object& o : objects) {
		ret = aabb::enclose(ret,o.bbox(t));
	}
	return ret;
}

trace object_list::hit(const ray& r, v2 t) const {
	
	trace ret;
	f32 closest = t.y;		
	for(const object& o : objects) {
		trace next = o.hit(r,{t.x,closest});
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

	fill();
	assert(done());

	object lane = object::sphere_lane(mat,pos,rad);
	clear();
	return lane;
}

