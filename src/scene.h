
#pragma once

#include "math.h"
#include "object.h"
#include "material.h"

struct camera {

	// params
	v3 pos, look;
	
	f32 ar = 0.0f;
	i32 wid = 0, hei = 0;
	
	f32 fov = 0.0f, aperture = 0.0f, start_time = 0.0f, end_time = 0.0f;

	void init(v3 p, v3 l, i32 w, i32 h, f32 f, f32 ap, f32 s, f32 d);
	void update();

	ray get_ray(f32 u, f32 v, f32 jitx, f32 jity) const;

private:
	v3 forward, right, up;
	v3 lower_left, horz_step, vert_step;
};

struct scene {

	object scene_obj;
	camera cam;
	i32 max_depth = 8;

	i32 lamb0 = 0, lamb1 = 0, met0 = 0, dia0 = 0;
	materal_cache mats;

	void init(i32 w, i32 h);
	void destroy();
	~scene();

	v3 compute(const ray& r, i32 depth = 0) const;
	v3 sample(f32 u, f32 v) const;

	object random_scene();
	object basic_scene();
};
