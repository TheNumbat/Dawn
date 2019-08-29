
#include "material.h"

isotropic isotropic::make(texture t) {
	isotropic ret;
	ret.tex = t;
	return ret;
}

scatter isotropic::bsdf(const ray& incoming, const trace& surface) const {
	scatter ret;
	ret.out = {surface.pos, random_leunit(), incoming.t};
	ret.attenuation = tex.sample(surface.uv, surface.pos);
	return ret;
}

diffuse diffuse::make(texture t) {
	diffuse ret;
	ret.tex = t;
	return ret;
}

scatter diffuse::bsdf(const ray&, const trace& surface) const {
	scatter ret;
	ret.emitted = tex.sample(surface.uv, {});
	ret.attenuation = {1.0f};
	ret.absorbed = true;
	return ret;
}

lambertian lambertian::make(texture t) {
	lambertian ret;
	ret.tex = t;
	return ret;
}

scatter lambertian::bsdf(const ray& incoming, const trace& surface) const {
	scatter ret;
	v3 out = surface.pos + surface.normal + random_leunit();
	ret.out = {surface.pos, out - surface.pos, incoming.t};
	ret.attenuation = tex.sample(surface.uv, surface.pos);
	return ret;
}

metal metal::make(v3 p, f32 r) {
	metal ret;
	ret.albedo = p;
	ret.rough = r;
	return ret;
}

scatter metal::bsdf(const ray& incoming, const trace& surface) const {
	scatter ret;
	v3 r = reflect(norm(incoming.dir), surface.normal);
	ret.out = {surface.pos, r + rough * random_leunit(), incoming.t};
	ret.absorbed = dot(r, surface.normal) <= 0.0f;
	ret.attenuation = albedo;
	return ret;
}

dielectric dielectric::make(f32 i) {
	dielectric ret;
	ret.index = i;
	return ret;
}

dielectric::refract_ dielectric::refract(v3 v, v3 n, f32 iout_iin) {
	refract_ ret;
	v3 in = norm(v);
	f32 ct = dot(in, n);
	f32 d = 1.0f - iout_iin*iout_iin*(1.0f - ct*ct);
	if(d > 0.0f) {
		ret.internal = false;
		ret.out = iout_iin*(in - ct*n) - n*sqrtf(d);
	}
	return ret;
}

f32 dielectric::schlick(f32 cos) const {
	f32 r = (1.0f - index) / (1.0f + index);
	r *= r;
	return r + (1.0f - r) * pow(1.0f - cos, 5.0f);
}

scatter dielectric::bsdf(const ray& incoming, const trace& surface) const {
	
	scatter ret;
	v3 reflected = reflect(incoming.dir, surface.normal);
	
	v3 n_out;
	f32 iout_iin, cos;

	f32 idn = dot(norm(incoming.dir), surface.normal);
	if(idn > 0.0f) {
		iout_iin = index;
		n_out = -surface.normal;
		
		cos = idn;
		cos = sqrtf(1.0f - index * index * (1.0f - cos * cos));

	} else {
		iout_iin = 1.0f / index;
		n_out = surface.normal;
		cos = -idn;
	}

	f32 reflect_prob;
	refract_ refracted = refract(incoming.dir, n_out, iout_iin);
	if(!refracted.internal) {
		reflect_prob = schlick(cos);
	} else {
		reflect_prob = 1.0f;
	}

	if(randomf() < reflect_prob) {
		ret.out = {surface.pos, reflected, incoming.t};
	} else {
		ret.out = {surface.pos, refracted.out, incoming.t};
	}
	ret.attenuation = v3(1.0f);
	return ret;
}

materal_cache::materal_cache() {
	clear();
}

void materal_cache::clear() {
	mats.clear(); 
	add(material::lambertian(texture::constant(v3(1.0f,0.0f,0.0f))));
	next_id = 1;
}

void materal_cache::destroy() {
	mats.destroy();
	next_id = 0;
}

materal_cache::~materal_cache() {
	destroy();
}

mat_id materal_cache::add(material m) {
	mats.push(m);
	return next_id++;
}

material* materal_cache::get(mat_id id) const {
	return mats.at(id);
}
