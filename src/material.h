
#pragma once

#include "math.h"
#include "object.h"

enum class mat : u8 {
	none = 0,
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

	scatter bsdf(const ray& incoming, const trace& surface) const;
};

struct metal {
	v3 albedo;
	f32 rough = 0.0f;

	scatter bsdf(const ray& incoming, const trace& surface) const;
};

struct dielectric {

	f32 index = 1.0f;

	struct refract_ {
		f32 internal = true;
		v3 out;
	};
	static refract_ refract(v3 v, v3 n, f32 iout_iin);

	f32 schlick(f32 cos) const;

	scatter bsdf(const ray& incoming, const trace& surface) const;
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
	scatter bsdf(const ray& incoming, const trace& surface) const {
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
	material() {memset(this,0,sizeof(material));}
};

typedef i32 mat_id;

struct materal_cache {
	
	materal_cache();
	void clear();
	void destroy();
	~materal_cache();

	mat_id add(material m);
	// NOTE(max): unstable when mats grows!!
	material* get(mat_id id) const;

private:
	vec<material> mats;
	mat_id next_id = 0;
};
