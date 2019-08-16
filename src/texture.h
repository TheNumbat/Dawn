
#pragma once

#include "lib/math.h"

enum class tex : u8 {
	none = 0,
	constant,
	checkerboard,
	noise,
	image
};

struct texture;

struct constant {
	
	static constant make(v3 c);
	void destroy() {}

	v3 sample(v2 uv, v3 p) const;
	
private:
	v3 color;
};

struct checkerboard {

	static checkerboard make(texture* o, texture* e);
	void destroy() {odd = even = null;}

	v3 sample(v2 uv, v3 p) const;

private:
	texture *odd = null, *even = null;
};

struct noise {

	static noise make(v3 loc, f32 scale);
	void destroy() {}

	v3 sample(v2 uv, v3 p) const;

private:
	f32 scale = 1.0f;
	v3 loc;
};

struct image {

	static image make(std::string file);
	void destroy();

	v3 sample(v2 uv, v3 p) const;

private:
	u8* data = null;
	i32 w = 0, h = 0;
};

struct texture {
	tex type = tex::none;
	union {
		constant c;
		checkerboard cb;
		noise n;
		image i;
	};
	static texture constant(v3 color) {
		texture ret;
		ret.type = tex::constant;
		ret.c = constant::make(color);
		return ret;
	}
	static texture checkerboard(texture* odd, texture* even) {
		texture ret;
		ret.type = tex::checkerboard;
		ret.cb = checkerboard::make(odd, even);
		return ret;
	}
	static texture noise(v3 loc, f32 scale) {
		texture ret;
		ret.type = tex::noise;
		ret.n = noise::make(loc, scale);
		return ret;
	}
	static texture image(std::string file) {
		texture ret;
		ret.type = tex::image;
		ret.i = image::make(file);
		return ret;
	}
	v3 sample(v2 uv, v3 p) const {
		switch(type) {
		case tex::constant: return c.sample(uv,p);
		case tex::checkerboard: return cb.sample(uv,p);
		case tex::noise: return n.sample(uv,p);
		case tex::image: return i.sample(uv,p);
		default: assert(false);
		}
		return {};
	}
	
	texture(const texture& o) {memcpy(this,&o,sizeof(texture));}
	texture(const texture&& o) {memcpy(this,&o,sizeof(texture));}
	void operator=(const texture& o) {memcpy(this,&o,sizeof(texture));}
	void operator=(const texture&& o) {memcpy(this,&o,sizeof(texture));}
	texture() {memset(this,0,sizeof(texture));}
	void destroy() {
		switch(type) {
		case tex::constant: c.destroy(); break;
		case tex::checkerboard: cb.destroy(); break;
		case tex::noise: n.destroy(); break;
		case tex::image: i.destroy(); break;
		default: assert(false);
		}
	}
};
