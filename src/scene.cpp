
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

	scene_obj = def.init(w, h);
}

void scene::destroy() {
	scene_obj.destroy();
	def.destroy();
}

v3 scene::compute(const ray& r_) const {
	
	ray r = r_;
	i32 depth = 0;

	v3 accum(0.0f), attn(1.0f);
	
	while(depth < max_depth) {
		
		trace t = scene_obj.hit(r, {0.0001f, FLT_MAX});
		if(t.hit) {

			scatter s = def.mats.get(t.mat)->bsdf(r, t);

			accum += attn * s.emitted;
			attn *= s.attenuation;
			r = s.out;

			if(s.absorbed) {
				return accum;
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
	ray r = def.cam.get_ray(uv, jit);
		
	v3 result = safe(compute(r));

	return result;
}

void random_bvh_scene::destroy() {
	mats.destroy();
	cam = {};
	even = odd = {};
	lamb0 = lamb1 = met0 = dia0 = 0;
}

object random_bvh_scene::init(i32 w, i32 h) {
	
	cam.init({13.0f, 2.0f, 3.0f}, {}, w, h, 60.0f, 0.1f, {0.0f, 1.0f});
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

	object ret = object::bvh(objs, cam.time, LANE_WIDTH, [](vec<object> list) -> object {

		sphere_lane_builder builder;

		for(const object& o : list) {
			builder.push(o);
		}

		return builder.finish();
	});

	objs.destroy();
	return ret;
}

object basic_scene::init(i32 w, i32 h) {

	cam.init({13.0f, 2.0f, 3.0f}, {}, w, h, 60.0f, 0.1f, {0.0f, 1.0f});
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

	vec<object> objs;

	objs.push(builder.finish());

	light = mats.add(material::diffuse(texture::constant({7.0f})));
	objs.push(object::rect(light, plane::zx, {-3.0f, 3.0f}, {-3.0f, 3.0f}, 6.0f));

	bound = object::sphere(0, {}, 5000.0f);
	vol = mats.add(material::isotropic(texture::constant({1.0f})));
	objs.push(object::volume(vol, 0.01f, &bound));

	return object::list(objs);
}

void basic_scene::destroy() {
	mats.destroy();
	cam = {};
	lamb0 = lamb1 = met0 = dia0 = 0;
}

object cornell_box::init(i32 w, i32 h) {

	cam.init({278.0f, 278.0f, -800.0f}, {278.0f, 278.0f, 0.0f}, w, h, 50.0f, 0.0f, {0.0f, 1.0f});
	mats.clear();

	red = mats.add(material::lambertian(texture::constant({0.65f, 0.05f, 0.05f})));
	white = mats.add(material::lambertian(texture::constant({0.73f, 0.73f, 0.73f})));
	green = mats.add(material::lambertian(texture::constant({0.12f, 0.45f, 0.15f})));
	light = mats.add(material::diffuse(texture::constant({7.0f})));

	vec<object> objs;

	objs.push(object::rect(green, plane::yz, {0.0f, 555.0f}, {0.0f, 555.0f}, 555.0f));
	objs.push(object::rect(red, plane::yz, {0.0f, 555.0f}, {0.0f, 555.0f}, 0.0f));
	objs.push(object::rect(light, plane::zx, {113.0f, 443.0f}, {127.0f, 432.0f}, 554.0f));
	
	objs.push(object::rect(white, plane::zx, {0.0f, 555.0f}, {0.0f, 555.0f}, 555.0f));
	objs.push(object::rect(white, plane::zx, {0.0f, 555.0f}, {0.0f, 555.0f}, 0.0f));
	objs.push(object::rect(white, plane::xy, {0.0f, 555.0f}, {0.0f, 555.0f}, 555.0f));

	dial = mats.add(material::dielectric(1.5f));

	box0 = object::box(dial, {}, {165.0f, 165.0f, 165.0f}, translate({130.0f, 0.0f, 65.0f}) * rotate(-18.0f, {0.0f, 1.0f, 0.0f}));
	box1 = object::box(white, {}, {165.0f, 330.0f, 165.0f}, translate({265.0f, 0.0f, 295.0f}) * rotate(15.0f, {0.0f, 1.0f, 0.0f}));

	white_vol = mats.add(material::isotropic(texture::constant({1.0f})));
	black_vol = mats.add(material::isotropic(texture::constant({})));

	objs.push(box0);

	objs.push(object::volume(white_vol, 0.01f, &box0));
	objs.push(object::volume(black_vol, 0.01f, &box1));

	object ret = object::bvh(objs, cam.time);
	objs.destroy();
	return ret;
}

void cornell_box::destroy() {
	mats.destroy();
	cam = {};
	red = white = green = light = 0;
}

object planet_scene::init(i32 w, i32 h) {

	cam.init({13.0f, 2.0f, 3.0f}, {}, w, h, 60.0f, 0.1f, {0.0f, 1.0f});
	mats.clear();
	flat = mats.add(material::lambertian(texture::constant({0.6f})));
	lamb = mats.add(material::lambertian(texture::image("mars.jpg")));
	light = mats.add(material::diffuse(texture::constant({8.0f})));

	vec<object> objs;

	objs.push(object::sphere(flat, {0.0f, -1000.0f, 0.0f}, 1000.0f));
	objs.push(object::sphere(lamb, {0.0f, 2.0f, 0.0f}, 2.0f));
	objs.push(object::rect(light, plane::yz, {3.0f, 5.0f}, {1.0f, 3.0f}, -2.0f));
	objs.push(object::rect(light, plane::xy, {3.0f, 5.0f}, {1.0f, 3.0f}, -2.0f));

	object ret = object::bvh(objs, cam.time);
	objs.destroy();
	return ret;
}

void planet_scene::destroy() {
	mats.destroy();
	cam = {};
	flat = lamb = light = 0;
}

object ps_showcase::init(i32 w, i32 h) {

	cam.init({278.0f, 278.0f, -800.0f}, {278.0f, 278.0f, 0.0f}, w, h, 50.0f, 0.0f, {0.0f, 1.0f});
	mats.clear();

	vec<object> objs;

	white = mats.add(material::lambertian(texture::constant({0.73f, 0.73f, 0.73f})));
	ground = mats.add(material::lambertian(texture::constant({0.48f, 0.83f, 0.53f})));

	for(i32 i = 0; i < 20; i++) {
		for(i32 j = 0; j < 20; j++) {
			
			f32 d = 100.0f;
			v3 xyz = {-1000.0f + i * d, 0.0f, -1000.0f + j * d};
			v3 xyz1 = {xyz.x + d, 100.0f * (randomf() + 0.01f), xyz.z + d};

			objs.push(object::box(ground, xyz, xyz1));
		}
	}

	light = mats.add(material::diffuse(texture::constant({7.0f})));

	objs.push(object::rect(light, plane::zx, {147.0f, 412.0f}, {123.0f, 423.0f}, 554.0f));

	v3 center(400.0f, 400.0f, 200.0f);

	moving = mats.add(material::lambertian(texture::constant({0.7f, 0.3f, 0.1f})));
	mtl    = mats.add(material::metal({0.8f, 0.8f, 0.9f}, 10.0f));
	dial   = mats.add(material::dielectric(1.5f));

	objs.push(object::sphere_moving(moving, center, center + v3{30.0f, 0.0f, 0.0f}, 50.0f, {0.0f, 1.0f}));

	objs.push(object::sphere(dial, {260.0f, 150.0f, 45.0f}, 50.0f));
	objs.push(object::sphere(mtl, {0.0f, 150.0f, 145.0f}, 50.0f));

	bound0 = object::sphere(dial, {360.0f, 150.0f, 145.0f}, 70.0f);
	bound1 = object::sphere(0, {}, 5000.0f);

	objs.push(bound0);

	vol0 = mats.add(material::isotropic(texture::constant({0.2f, 0.4f, 0.9f})));
	vol1 = mats.add(material::isotropic(texture::constant({1.0f})));

	objs.push(object::volume(vol0, 0.2f, &bound0));
	objs.push(object::volume(vol1, 0.0001f, &bound1));

	mars  = mats.add(material::lambertian(texture::image("mars.jpg")));
	noise = mats.add(material::lambertian(texture::noise({}, 0.1f)));

	objs.push(object::sphere(mars, {400.0f, 200.0f, 400.0f}, 100.0f));
	objs.push(object::sphere(noise, {220.0f, 280.0f, 300.0f}, 80.0f));

	vec<object> spheres;

	for(i32 i = 0; i < 1000; i++) {
		spheres.push(object::sphere(white, 165.0f * abs(randomvec()), 10.0f));
	}

	objs.push(object::bvh(spheres, cam.time, LANE_WIDTH, [](vec<object> list) -> object {

		sphere_lane_builder builder;

		for(const object& o : list) {
			builder.push(o);
		}

		return builder.finish();
	}, translate({-100.0f, 270.0f, 395.0f}) * rotate(15.0f, {0.0f, 1.0f, 0.0f})));

	object ret = object::bvh(objs, cam.time);

	spheres.destroy();
	objs.destroy();
	return ret;
}

void ps_showcase::destroy() {
	mats.destroy();
	cam = {};
	white = ground = light = moving = dial = mtl = 0;
	vol0 = vol1 = mars = noise = 0;
	bound0 = bound1 = {};
}
