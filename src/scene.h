
#pragma once

#include "math.h"
#include "object.h"
#include "material.h"

struct camera {

	// params
	f32 ar, fov, aperture;
	i32 wid,hei;
	v3 pos, look;

	// calculated
	v3 forward, right, up;
	v3 lower_left, horz_step, vert_step;

	void init(v3 p, v3 l, i32 w, i32 h, f32 f, f32 ap) {
		wid = w;
		hei = h;
		pos = p;
		aperture = ap / 2.0f;
		look = l;
		fov = RADIANS(f);
		ar = (f32)hei / wid;
		
		update();
	}
	void update() {
		f32 half_w = tan(fov / 2.0f);
		f32 half_h = ar * half_w;

		forward = pos-look;
		f32 focus = len(forward);
		forward /= focus;

		right = cross(forward,v3(0.0f,1.0f,0.0f));
		up = cross(right,forward);

		lower_left = pos - half_w*focus*right + half_h*focus*up - focus*forward;

		horz_step = 2.0f*half_w*focus*right;
		vert_step = -2.0f*half_h*focus*up;
	}

	ray get_ray(f32 u, f32 v, f32 jitx, f32 jity) {
		
		jitx /= (f32)wid;
		jity /= (f32)hei;
		// u += jitx; v += jity;

		v3 lens_pos;// = aperture * random_ledisk();
		v3 offset = pos + right * lens_pos.x + up * lens_pos.y;
		return {offset, lower_left + u*horz_step + v*vert_step - offset};
	}
};

struct scene {

	object_list list;
	camera cam;
	i32 samples = 1, max_depth = 5;

	i32 lamb0 = 0, lamb1 = 0, met0 = 0, dia0 = 0;
	materal_cache mats;

	scene() {}
	void init(i32 w, i32 h, i32 s) {
		samples = s;
		cam.init(v3(13.0f,2.0f,3.0f), {}, w, h, 60.0f, 0.1f);

		mats.clear();
		lamb0 = mats.add(material::lambertian(v3(0.5f)));
		lamb1 = mats.add(material::lambertian(v3(0.4f,0.2f,0.1f)));
		met0 = mats.add(material::metal(v3(0.7f,0.6f,0.5f), 0.0f));
		dia0 = mats.add(material::dielectric(1.5f));

		list.push(object::sphere(lamb0, v3(0.0f,-1000.0f,0.0f), 1000.0f));
		for (i32 a = -11; a < 11; a++) {
			for (i32 b = -11; b < 11; b++) {
				f32 choose_mat = randf_cpp();
				v3 center(a+0.9f*randf_cpp(),0.2f,b+0.9f*randf_cpp()); 

				if (len(center-v3(4.0f,0.2f,0.0f)) > 0.9f) { 
					if (choose_mat < 0.8f) {
						mat_id id = mats.add(material::lambertian(v3(randf_cpp()*randf_cpp(), randf_cpp()*randf_cpp(), randf_cpp()*randf_cpp())));
						list.push(object::sphere(id, center, 0.2f));
					} else if (choose_mat < 0.95) {
						mat_id id = mats.add(material::metal(v3(0.5f*(1.0f + randf_cpp()), 0.5f*(1.0f + randf_cpp()), 0.5f*(1.0f + randf_cpp())), 0.5f*randf_cpp()));
						list.push(object::sphere(id, center, 0.2f));
					} else {
						list.push(object::sphere(dia0, center, 0.2f));
					}
				}
			}
		}

		list.push(object::sphere(dia0, v3(0, 1, 0), 1.0));
		list.push(object::sphere(lamb1, v3(-4, 1, 0), 1.0));
		list.push(object::sphere(met0, v3(4, 1, 0), 1.0));
	}
	void destroy() {
		cam = {};
		list.destroy();
		mats.clear();
		samples = 1;
	}
	~scene() {destroy();}

	v3 compute(ray r, i32 depth = 0) {
		trace t = list.hit(r, 0.001f, FLT_MAX);
		if(t.hit) {
			if(depth >= max_depth) return v3();

			scatter s = mats.get(t.mat)->bsdf(r, t);

			if(s.absorbed) return v3();

			return s.attenuation * compute(s.out, depth+1);
		}

		v3 unit = norm(r.dir);
		f32 fade = 0.5f * (unit.y + 1.0f);
		return lerp(v3(0.5f,0.7f,1.0f), v3(1.0f), fade);
	}
	v3 pixel(f32 u, f32 v) {
		v3 result;
		for(i32 s = 0; s < samples; s++) {
			f32 jitx = randf_cpp(), jity = randf_cpp();
			ray r = cam.get_ray(u,v, jitx,jity);
			result += compute(r);
		}
		result /= (f32)samples;
		return pow(result, 1.0f / 2.2f);
	}
};
