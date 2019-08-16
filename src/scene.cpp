
#include "scene.h"
#include "lib/vec.h"

void camera::init(v3 p, v3 l, i32 w, i32 h, f32 f, f32 ap, v2 t) {
	wid = w;
	hei = h;
	pos = p;
	time = t;
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

ray camera::get_ray(v2 uv, v2 jit) const {
	
	jit.x /= (f32)wid;
	jit.y /= (f32)hei;
	uv += jit;

	f32 ray_time = lerp(time.x, time.y, randomf());
	v3 lens_pos = aperture * random_ledisk();
	v3 offset = pos + right * lens_pos.x + up * lens_pos.y;
	return {offset, lower_left + uv.x*horz_step + uv.y*vert_step - offset, ray_time};
}

scene::~scene() {
	destroy();
}

void scene::init(i32 w, i32 h) {

	g_perlin.init();
	cam.init({13.0f, 2.0f, 3.0f}, {}, w, h, 60.0f, 0.1f, {0.0f, 1.0f});

	scene_obj = def.init(cam.time);
}

void scene::destroy() {
	cam = {};
	scene_obj.destroy();
	def.destroy();
}

v3 scene::compute(const ray& r_) const {
	
	ray r = r_;
	i32 depth = 0;

	v3 accum(0.0f), attn(1.0f);
	
	while(depth < max_depth) {
		
		trace t = scene_obj.hit(r, {0.001f, FLT_MAX});
		if(t.hit) {

			scatter s = def.mats.get(t.mat)->bsdf(r, t);

			if(s.absorbed) {
				return s.emitted;
			} else {
				accum += attn * s.emitted;
				attn *= s.attenuation;
				r = s.out;
			}

		} else {

			return accum;
		}

		depth++;
	}

	return accum;
}

v3 scene::sample(v2 uv) const {
		
	v2 jit = {randomf(), randomf()};
	ray r = cam.get_ray(uv, jit);
		
	v3 result = safe(compute(r));

	// TODO(max): effect/HDR/Gamma pipeline
	return pow(result, 1.0f / 2.2f);
}

object random_bvh_scene::init(v2 t) {
	
	mats.clear();

	even = texture::constant({0.2f, 0.3f, 0.1f});
	odd = texture::constant({0.9f});
	lamb0 = mats.add(material::lambertian(texture::checkerboard(&even, &odd)));
	lamb1 = mats.add(material::lambertian(texture::constant({0.4f, 0.2f, 0.1f})));
	met0 = mats.add(material::metal({0.7f, 0.6f, 0.5f}, 0.0f));
	dia0 = mats.add(material::dielectric(1.5f));

	vec<object> objs;

	objs.push(object::sphere(lamb0, {0.0f, -1000.0f, 0.0f}, 1000.0f));
	objs.push(object::sphere(dia0, {0.0f, 1.0f, 0.0f}, 1.0f));
	objs.push(object::sphere(lamb1, {-4.0f, 1.0f, 0.0f}, 1.0f));
	objs.push(object::sphere(met0, {4.0f, 1.0f, 0.0f}, 1.0f));

	for (i32 a = -16; a < 16; a++) {
		for (i32 b = -16; b < 16; b++) {

			f32 choose_mat = randomf();
			v3 center(a+0.9f*randomf(),0.2f,b+0.9f*randomf()); 

			if (len(center - v3{4.0f, 0.2f, 0.0f}) > 0.9f) { 

				if (choose_mat < 0.8f) {
					
					mat_id id = mats.add(material::lambertian(
						texture::constant({randomf()*randomf(), 
										   randomf()*randomf(), 
										   randomf()*randomf()})));

					objs.push(object::sphere(id, center, 0.2f));

				} else if (choose_mat < 0.95) {

					mat_id id = mats.add(material::metal({0.5f * (1.0f + randomf()), 
														  0.5f * (1.0f + randomf()), 
														  0.5f * (1.0f + randomf())}, 
														  0.5f*randomf()));
					
					objs.push(object::sphere(id, center, 0.2f));

				} else {

					objs.push(object::sphere(dia0, center, 0.2f));
				}
			}
		}
	}

	object ret = object::bvh(objs, t, LANE_WIDTH, [](vec<object> list) -> object {

		sphere_lane_builder builder;

		for(const object& o : list) {
			builder.push(o);
		}

		return builder.finish();
	});

	objs.destroy();
	return ret;
}

object basic_scene::init(v2) {

	mats.clear();

	lamb0 = mats.add(material::lambertian(texture::constant({0.6f})));
	lamb1 = mats.add(material::lambertian(texture::image("mars.jpg")));
	met0 = mats.add(material::metal({0.7f, 0.6f, 0.5f}, 0.0f));
	dia0 = mats.add(material::dielectric(1.5f));

	sphere_lane_builder builder;

	builder.push(object::sphere(lamb0, {0.0f, -1000.0f, 0.0f}, 1000.0f));
	builder.push(object::sphere(dia0, {0.0f, 1.0f, 0.0f}, 1.0f));
	builder.push(object::sphere(met0, {-4.0f, 1.0f, 0.0f}, 1.0f));
	builder.push(object::sphere(lamb1, {4.0f, 1.0f, 0.0f}, 1.0f));

	return builder.finish();
}

object noise_scene::init(v2) {

	mats.clear();

	lamb = mats.add(material::lambertian(texture::noise({}, 4.0f)));

	vec<object> objs;

	objs.push(object::sphere(lamb, {0.0f, -1000.0f, 0.0f}, 1000.0f));
	objs.push(object::sphere(lamb, {0.0f, 2.0f, 0.0f}, 2.0f));

	return object::list(objs);
}

object planet_scene::init(v2) {

	mats.clear();
	flat = mats.add(material::lambertian(texture::constant({0.6f})));
	lamb = mats.add(material::lambertian(texture::image("mars.jpg")));
	light = mats.add(material::diffuse(texture::constant({8.0f})));

	vec<object> objs;

	objs.push(object::sphere(flat, {0.0f, -1000.0f, 0.0f}, 1000.0f));
	objs.push(object::sphere(lamb, {0.0f, 2.0f, 0.0f}, 2.0f));
	objs.push(object::rect(light, plane::xy, {3.0f, 5.0f}, {1.0f, 3.0f}, -2.0f));

	return object::list(objs);
}
