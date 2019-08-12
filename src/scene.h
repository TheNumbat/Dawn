
#pragma once

#include "math.h"
#include "object.h"
#include "material.h"

struct camera {

	// params
	v3 pos, look;
	
	f32 ar = 0.0f;
	i32 wid = 0, hei = 0;
	
	f32 fov = 0.0f, aperture = 0.0f;
	v2 time;

	void init(v3 p, v3 l, i32 w, i32 h, f32 f, f32 ap, v2 t);
	void update();

	ray get_ray(v2 uv, v2 jit) const;

private:
	v3 forward, right, up;
	v3 lower_left, horz_step, vert_step;
};

struct random_bvh_scene {
	
	object init(v2 t);
	void destroy();
	material* get(i32 idx) const;

private:
	texture even, odd;
	i32 lamb0 = 0, lamb1 = 0, met0 = 0, dia0 = 0;
	materal_cache mats;
};

struct basic_scene {
	
	object init(v2 t);
	void destroy() {}
	material* get(i32 idx) const;

private:
	i32 lamb0 = 0, lamb1 = 0, met0 = 0, dia0 = 0;
	materal_cache mats;
};

struct noise_scene {
	
	object init(v2 t);
	void destroy() {}
	material* get(i32 idx) const;

private:
	i32 lamb = 0;
	materal_cache mats;
};

struct scene {

	void init(i32 w, i32 h);
	void destroy();
	~scene();

	v3 compute(const ray& into) const;
	v3 sample(v2 uv) const;

private:
	object scene_obj;
	camera cam;
	i32 max_depth = 5;

	noise_scene def;
};
