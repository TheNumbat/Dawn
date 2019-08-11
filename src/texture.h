
#pragma once

#include "lib/math.h"

enum class tex : u8 {
	none = 0,
	constant,
	checkerboard
};

struct texture;

struct constant {
	
	static constant make(v3 c);
	void destroy() {}

	v3 sample(f32 u, f32 v, v3 p) const;
	
private:
	v3 color;
};

struct checkerboard {

	static checkerboard make(texture* o, texture* e);
	void destroy();

	v3 sample(f32 u, f32 v, v3 p) const;

private:
	texture *odd = null, *even = null;
};

struct texture {
	tex type = tex::none;
	union {
		constant c;
		checkerboard cb;
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
	v3 sample(f32 u, f32 v, v3 p) const {
		switch(type) {
		case tex::constant: return c.sample(u,v,p);
		case tex::checkerboard: return cb.sample(u,v,p);
		default: assert(false);
		}
		return {};
	}
	
	texture(const texture& o) {memcpy(this,&o,sizeof(texture));}
	texture(const texture&& o) {memcpy(this,&o,sizeof(texture));}
	void operator=(const texture& o) {memcpy(this,&o,sizeof(texture));}
	void operator=(const texture&& o) {memcpy(this,&o,sizeof(texture));}
	texture() {memset(this,0,sizeof(texture));}
};
