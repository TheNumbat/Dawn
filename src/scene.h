
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
	
	object init(i32 w, i32 h);
	void destroy();

	camera cam;
	materal_cache mats;

private:
	texture even, odd;
	i32 lamb0 = 0, lamb1 = 0, met0 = 0, dia0 = 0;
};

struct basic_scene {
	
	object init(i32 w, i32 h);
	void destroy();
	
	camera cam;
	materal_cache mats;

private:
	i32 lamb0 = 0, lamb1 = 0, met0 = 0, dia0 = 0, light = 0, vol = 0;
	object bound;
};

struct cornell_box {
	
	object init(i32 w, i32 h);
	void destroy();

	camera cam;
	materal_cache mats;

private:
	i32 red = 0, white = 0, green = 0, light = 0, dial = 0;
	i32 white_vol = 0, black_vol = 0;
	object box0, box1;
};

struct ps_showcase {

	object init(i32 w, i32 h);
	void destroy();

	camera cam;
	materal_cache mats;

private:
	i32 white = 0, ground = 0, light = 0, moving = 0, dial = 0, mtl = 0;
	i32 vol0 = 0, vol1 = 0, mars = 0, noise = 0;
	object bound0, bound1;
};

struct planet_scene {
	
	object init(i32 w, i32 h);
	void destroy();

	camera cam;
	materal_cache mats;

private:
	i32 lamb = 0, light = 0, flat = 0;
};

struct scene {

	void init(i32 w, i32 h);
	void destroy();
	~scene();

	v3 compute(const ray& into) const;
	v3 sample(v2 uv) const;

private:
	object scene_obj;
	i32 max_depth = 16;

	ps_showcase def;
};
