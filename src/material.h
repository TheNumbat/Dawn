
#pragma once

#include "lib/math.h"
#include "object.h"
#include "texture.h"

enum class mat : u8 {
	none = 0,
	lambertian,
	metal,
	dielectric,
	diffuse,
	isotropic
};

struct scatter {
	ray out;
	bool absorbed = false;
	v3 attenuation, emitted;
};

struct isotropic {

	static isotropic make(texture t);
	void destroy() {tex.destroy();}

	scatter bsdf(const ray& incoming, const trace& surface) const;

private:
	texture tex;
};

struct diffuse {

	static diffuse make(texture t);
	void destroy() {tex.destroy();}

	scatter bsdf(const ray& incoming, const trace& surface) const;

private:
	texture tex;
};

struct lambertian {

	static lambertian make(texture t);
	void destroy() {tex.destroy();}

	scatter bsdf(const ray& incoming, const trace& surface) const;

private:
	texture tex;
};

struct metal {

	static metal make(v3 p, f32 r);
	void destroy() {}

	scatter bsdf(const ray& incoming, const trace& surface) const;
	
private:
	v3 albedo;
	f32 rough = 0.0f;
};

struct dielectric {

	static dielectric make(f32 i);
	void destroy() {}

	scatter bsdf(const ray& incoming, const trace& surface) const;

private:
	f32 index = 1.0f;

	struct refract_ {
		f32 internal = true;
		v3 out;
	};
	static refract_ refract(v3 v, v3 n, f32 iout_iin);

	f32 schlick(f32 cos) const;
};

struct material {
	mat type = mat::none;
	union {
		lambertian l;
		metal m;
		dielectric d;
		diffuse df;
		isotropic iso;
	};
	static material diffuse(texture t) {
		material ret;
		ret.type = mat::diffuse;
		ret.df = diffuse::make(t);
		return ret;
	}
	static material lambertian(texture t) {
		material ret;
		ret.type = mat::lambertian;
		ret.l = lambertian::make(t);
		return ret;
	}
	static material metal(v3 albedo, f32 rough) {
		material ret;
		ret.type = mat::metal;
		ret.m = metal::make(albedo, rough);
		return ret;
	}
	static material dielectric(f32 index) {
		material ret;
		ret.type = mat::dielectric;
		ret.d = dielectric::make(index);
		return ret;
	}
	static material isotropic(texture t) {
		material ret;
		ret.type = mat::isotropic;
		ret.iso = isotropic::make(t);
		return ret;
	}

	scatter bsdf(const ray& incoming, const trace& surface) const {
		switch(type) {
		case mat::metal: return m.bsdf(incoming, surface);
		case mat::diffuse: return df.bsdf(incoming, surface);
		case mat::lambertian: return l.bsdf(incoming, surface);
		case mat::dielectric: return d.bsdf(incoming, surface);
		case mat::isotropic: return iso.bsdf(incoming, surface);
		default: assert(false);
		}
		return {};
	}
	
	material(const material& o) {memcpy(this,&o,sizeof(material));}
	material(const material&& o) {memcpy(this,&o,sizeof(material));}
	material& operator=(const material& o) {memcpy(this,&o,sizeof(material)); return *this;}
	material& operator=(const material&& o) {memcpy(this,&o,sizeof(material)); return *this;}
	material() {memset(this,0,sizeof(material));}
	void destroy() {
		switch(type) {
		case mat::metal: m.destroy(); break;
		case mat::diffuse: df.destroy(); break;
		case mat::dielectric: d.destroy(); break;
		case mat::lambertian: l.destroy(); break;
		case mat::isotropic: iso.destroy(); break;
		default: assert(false);
		}
	}
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
