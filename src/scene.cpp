
#include "scene.h"
#include "lib/vec.h"

void camera::init(v3 p, v3 l, i32 w, i32 h, f32 f, f32 ap, f32 t0, f32 t1) {
	wid = w;
	hei = h;
	pos = p;
	start_time = t0;
	end_time = t1;
	aperture = ap / 2.0f;
	look = l;
	fov = RADIANS(f);
	ar = (f32)hei / wid;
	
	update();
}

void camera::update() {
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

ray camera::get_ray(f32 u, f32 v, f32 jitx, f32 jity) const {
	
	jitx /= (f32)wid;
	jity /= (f32)hei;
	u += jitx; v += jity;

	f32 time = lerp(start_time, end_time, randomf());
	v3 lens_pos = aperture * random_ledisk();
	v3 offset = pos + right * lens_pos.x + up * lens_pos.y;
	return {offset, lower_left + u*horz_step + v*vert_step - offset, time};
}

scene::~scene() {
	destroy();
}

void scene::init(i32 w, i32 h) {
	cam.init(v3(13.0f,2.0f,3.0f), {}, w, h, 60.0f, 0.1f, 0.0f, 1.0f);

	scene_obj = def.init(cam.start_time, cam.end_time);
}

void scene::destroy() {
	cam = {};
	scene_obj.destroy();
	def.destroy();
}

v3 scene::compute(const ray& r_) const {
	
	ray r = r_;
	i32 depth = 0;
	v3 accum(1.0f);	
	
	while(depth < max_depth) {
		
		trace t = scene_obj.hit(r, 0.001f, FLT_MAX);
		if(t.hit) {

			scatter s = def.get(t.mat)->bsdf(r, t);

			if(s.absorbed) {
				accum *= v3{}; 
				break;
			} else {
				accum *= s.attenuation;
				r = s.out;
			}

		} else {

			v3 unit = norm(r.dir);
			f32 fade = 0.5f * (unit.y + 1.0f);
			accum *= lerp(v3(0.5f,0.7f,1.0f), v3(1.0f), fade);
			break;
		}

		depth++;
	}

	return accum;
}

v3 scene::sample(f32 u, f32 v) const {
		
	f32 jitx = randomf(), jity = randomf();
	ray r = cam.get_ray(u,v, jitx,jity);
		
	v3 result = safe(compute(r));

	// TODO(max): effect/HDR/Gamma pipeline
	return pow(result, 1.0f / 2.2f);
}

object random_bvh_scene::init(f32 tmin, f32 tmax) {
	
	mats.clear();

	even = texture::constant(v3(0.2f, 0.3f, 0.1f));
	odd = texture::constant(v3(0.9f));
	lamb0 = mats.add(material::lambertian(texture::checkerboard(&even, &odd)));
	lamb1 = mats.add(material::lambertian(texture::constant(v3(0.4f, 0.2f, 0.1f))));
	met0 = mats.add(material::metal(v3(0.7f, 0.6f, 0.5f), 0.0f));
	dia0 = mats.add(material::dielectric(1.5f));

	vec<object> objs;

	objs.push(object::sphere(lamb0, v3(0.0f, -1000.0f, 0.0f), 1000.0f));
	objs.push(object::sphere(dia0, v3(0.0f, 1.0f, 0.0f), 1.0f));
	objs.push(object::sphere(lamb1, v3(-4.0f, 1.0f, 0.0f), 1.0f));
	objs.push(object::sphere(met0, v3(4.0f, 1.0f, 0.0f), 1.0f));

	for (i32 a = -16; a < 16; a++) {
		for (i32 b = -16; b < 16; b++) {
			f32 choose_mat = randomf();
			v3 center(a+0.9f*randomf(),0.2f,b+0.9f*randomf()); 

			if (len(center-v3(4.0f,0.2f,0.0f)) > 0.9f) { 
				if (choose_mat < 0.8f) {
					mat_id id = mats.add(material::lambertian(texture::constant(v3(randomf()*randomf(), randomf()*randomf(), randomf()*randomf()))));
					objs.push(object::sphere(id, center, 0.2f));
				} else if (choose_mat < 0.95) {
					mat_id id = mats.add(material::metal(v3(0.5f*(1.0f + randomf()), 0.5f*(1.0f + randomf()), 0.5f*(1.0f + randomf())), 0.5f*randomf()));
					objs.push(object::sphere(id, center, 0.2f));
				} else {
					objs.push(object::sphere(dia0, center, 0.2f));
				}
			}
		}
	}

	object ret = object::bvh(objs, tmin, tmax, LANE_WIDTH, [](vec<object> list) -> object {

		sphere_lane_builder builder;

		for(const object& o : list) {
			builder.push(o);
		}

		return builder.finish();
	});

	objs.destroy();
	return ret;
}

void random_bvh_scene::destroy() {

	mats.destroy();
	lamb0 = lamb1 = met0 = dia0 = 0;
}

material* random_bvh_scene::get(i32 idx) const {
	return mats.get(idx);
}

object basic_scene::init(f32, f32) {

	mats.clear();

	lamb0 = mats.add(material::lambertian(texture::constant(v3(0.6f))));
	lamb1 = mats.add(material::lambertian(texture::constant(v3(0.4f,0.2f,0.1f))));
	met0 = mats.add(material::metal(v3(0.7f,0.6f,0.5f), 0.0f));
	dia0 = mats.add(material::dielectric(1.5f));

	vec<object> objs;

	objs.push(object::sphere(lamb0, v3(0.0f, -1000.0f, 0.0f), 1000.0f));
	objs.push(object::sphere(dia0, v3(0.0f, 1.0f, 0.0f), 1.0f));
	objs.push(object::sphere(lamb1, v3(-4.0f, 1.0f, 0.0f), 1.0f));
	objs.push(object::sphere(met0, v3(4.0f, 1.0f, 0.0f), 1.0f));

	return object::list(objs);
}

material* basic_scene::get(i32 idx) const {
	return mats.get(idx);
}

object noise_scene::init(f32, f32) {

	mats.clear();

	texture t = texture::noise();
	lamb = mats.add(material::lambertian(t));

	vec<object> objs;

	objs.push(object::sphere(lamb, v3(0.0f, -1000.0f, 0.0f), 1000.0f));
	objs.push(object::sphere(lamb, v3(0.0f, 2.0f, 0.0f), 2.0f));

	return object::list(objs);
}

material* noise_scene::get(i32 idx) const {
	return mats.get(idx);
}
