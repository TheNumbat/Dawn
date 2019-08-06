
#pragma once

#include "math.h"

enum class mat : u8 {
	none,
	lambertian,
	metal,
	dielectric
};

struct scatter {
	bool absorbed = false;
	ray out;
	v3 attenuation;
};

struct lambertian {
	v3 albedo;

	scatter bsdf(ray&, trace& surface) {
		scatter ret;
		v3 out = surface.pos + surface.normal + random_leunit();
		ret.out = {surface.pos, out - surface.pos};
		ret.attenuation = albedo;
		return ret;
	}
};

struct metal {
	v3 albedo;
	f32 rough = 0.0f;

	scatter bsdf(ray& incoming, trace& surface) {
		scatter ret;
		v3 r = reflect(norm(incoming.dir),surface.normal);
		ret.out = {surface.pos, r + rough * random_leunit()};
		ret.attenuation = albedo;
		ret.absorbed = dot(r, surface.normal) <= 0.0f;
		return ret;
	}
};

struct dielectric {

	f32 index = 1.0f;

	struct refract_ {
		bool internal = true;
		v3 out;
	};
	refract_ refract(v3 v, v3 n, f32 iout_iin) {
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

	f32 schlick(f32 cos) {
		f32 r = (1.0f - index) / (1.0f + index);
		r *= r;
		return r + (1.0f - r) * pow(1.0f - cos, 5.0f);
	}

	scatter bsdf(ray& incoming, trace& surface) {
		
		scatter ret;
		v3 reflected = reflect(incoming.dir,surface.normal);
		
		v3 n_out;
		f32 iout_iin, cos;

		f32 idn = dot(norm(incoming.dir),surface.normal);
		if(idn > 0.0f) {
			iout_iin = index;
			n_out = -surface.normal;
			cos = index * idn;
		} else {
			iout_iin = 1.0f / index;
			n_out = surface.normal;
			cos = -idn;
		}

		f32 refract_prob;
		refract_ refracted = refract(incoming.dir,n_out,iout_iin);
		if(!refracted.internal) {
			refract_prob = schlick(cos);
		} else {
			refract_prob = 1.0f;
		}

		if(randf_cpp() < refract_prob) {
			ret.out = {surface.pos, reflected};
		} else {
			ret.out = {surface.pos, refracted.out};
		}
		ret.attenuation = v3(1.0f);
		return ret;
	}
};

struct material {
	mat type = mat::none;
	union {
		lambertian l;
		metal m;
		dielectric d;
	};
	static material lambertian(v3 albedo) {
		material ret;
		ret.type = mat::lambertian;
		ret.l = {albedo};
		return ret;
	}
	static material metal(v3 albedo, f32 rough) {
		material ret;
		ret.type = mat::metal;
		ret.m = {albedo, rough};
		return ret;
	}
	static material dielectric(f32 index) {
		material ret;
		ret.type = mat::dielectric;
		ret.d = {index};
		return ret;
	}
	scatter bsdf(ray& incoming, trace& surface) {
		switch(type) {
		case mat::lambertian: return l.bsdf(incoming,surface);
		case mat::metal: return m.bsdf(incoming,surface);
		case mat::dielectric: return d.bsdf(incoming,surface);
		default: assert(false);
		}
		return {};
	}
	
	material(const material& o) {memcpy(this,&o,sizeof(material));}
	material(const material&& o) {memcpy(this,&o,sizeof(material));}
	void operator=(const material& o) {memcpy(this,&o,sizeof(material));}
	void operator=(const material&& o) {memcpy(this,&o,sizeof(material));}
	material() {}
};