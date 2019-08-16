
#pragma once

#include "math.h"
#include "object.h"
#include "material.h"

struct random_bvh_scene {
	
	object init(v2 t);

	void destroy() {mats.destroy();}
	materal_cache mats;

private:
	texture even, odd;
	i32 lamb0 = 0, lamb1 = 0, met0 = 0, dia0 = 0;
};

struct basic_scene {
	
	object init(v2 t);

	void destroy() {mats.destroy();}
	materal_cache mats;

private:
	i32 lamb0 = 0, lamb1 = 0, met0 = 0, dia0 = 0;
};

struct noise_scene {
	
	object init(v2 t);
	
	void destroy() {mats.destroy();}
	materal_cache mats;

private:
	i32 lamb = 0;
};

struct planet_scene {
	
	object init(v2 t);
	
	void destroy() {mats.destroy();}
	materal_cache mats;

private:
	i32 lamb = 0, light = 0, flat = 0;
};


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

struct scene {

	void init(i32 w, i32 h);
	void destroy();
	~scene();

	v3 compute(const ray& into) const;
	v3 sample(v2 uv) const;

private:
	object scene_obj;
	camera cam;
	i32 max_depth = 8;

	planet_scene def;
};
