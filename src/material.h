
#pragma once

#include "math.h"
#include "object.h"

enum class mat : u8 {
	none,
	lambertian,
	metal,
	dielectric
};

struct scatter_lane {
	f32_lane absorbed;
	ray_lane out;
	v3 attenuation;
};

struct lambertian {
	v3 albedo;

	scatter_lane bsdf(const ray_lane&, const trace_lane& surface) const {
		scatter_lane ret;
		v3_lane out = surface.pos + surface.normal + random_leunit();
		ret.out = {surface.pos, out - surface.pos};
		ret.attenuation = albedo;
		return ret;
	}
};

struct metal {
	v3 albedo;
	f32 rough = 0.0f;

	scatter_lane bsdf(const ray_lane& incoming, const trace_lane& surface) const {
		scatter_lane ret;
		v3_lane r = reflect(norm(incoming.dir),surface.normal);
		ret.out = {surface.pos, r + rough * random_leunit()};
		ret.absorbed = dot(r, surface.normal) <= 0.0f;
		ret.attenuation = albedo;
		return ret;
	}
};

struct dielectric {

	f32 index = 1.0f;

	struct refract_ {
		f32_lane internal;
		v3_lane out;
	};
	static refract_ refract(const v3_lane& v, const v3_lane& n, const f32_lane& iout_iin) {
		refract_ ret;
		v3_lane in = norm(v);
		f32_lane ct = dot(in, n);
		f32_lane d = 1.0f - iout_iin*iout_iin*(1.0f - ct*ct);
		
		ret.internal = d <= 0.0f;
		ret.out = iout_iin*(in - ct*n) - n*sqrt(d);
		return ret;
	}

	f32_lane schlick(const f32_lane& cos) const {
		f32_lane r = (1.0f - index) / (1.0f + index);
		r *= r;
		return r + (1.0f - r) * pow(1.0f - cos, 5.0f);
	}

	scatter_lane bsdf(const ray_lane& incoming, const trace_lane& surface) const {
		
		scatter_lane ret;
		v3_lane reflected = reflect(incoming.dir,surface.normal);
		
		v3_lane n_out;
		f32_lane iout_iin, cos;

		f32_lane idn = dot(norm(incoming.dir),surface.normal);
		
		f32_lane pos_mask = idn > 0.0f;
		f32_lane neg_mask = ~pos_mask;
		{
			iout_iin |= pos_mask & f32_lane{index};
			cos      |= pos_mask & (index * idn);
			n_out 	 |= pos_mask & (-surface.normal);
		}
		{
			iout_iin |= neg_mask & f32_lane{1.0f / index};
			cos      |= neg_mask & (-idn);
			n_out    |= neg_mask & (surface.normal);
		}

		refract_ refracted = refract(incoming.dir,n_out,iout_iin);

		f32_lane refract_prob{1.0f};
		refract_prob &= refracted.internal;
		refract_prob |= (~refracted.internal) & schlick(cos);

		f32_lane reflect_mask = randf_lane() < refract_prob;
		
		ret.out.pos  = surface.pos;
		ret.out.dir |= reflect_mask & reflected;
		ret.out.dir |= (~reflect_mask) & refracted.out;

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
	scatter_lane bsdf(const ray_lane& incoming, const trace_lane& surface) const {
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

typedef i32 mat_id;

struct materal_cache {
	
	materal_cache() {clear();}
	void clear() {
		mats.clear(); 
		add(material::lambertian(v3(1.0f,0.0f,0.0f)));
		next_id = 1;
	}
	void destroy() {next_id = 1;}
	~materal_cache() {destroy();}

	mat_id add(material m) {
		mats.push_back(m);
		return next_id++;
	}
	// NOTE(max): unstable when mats grows!!
	const material* get(mat_id id) const {
		return &mats[id];
	}

private:
	std::vector<material> mats;
	mat_id next_id = 0;
};
