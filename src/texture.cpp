
#include "texture.h"

constant constant::make(v3 c) {
	constant ret;
	ret.color = c;
	return ret;
}

v3 constant::sample(f32, f32, v3) const {
	return color;
}

checkerboard checkerboard::make(texture* o, texture* e) {
	checkerboard ret;
	ret.odd = o;
	ret.even = e;
	return ret;
}

void checkerboard::destroy() {
	odd = even = null;
}

v3 checkerboard::sample(f32 u, f32 v, v3 p) const {

	f32 sin = sinf(10.0f * p.x) * sinf(10.0f * p.y) * sinf(10.0f * p.z);
	
	if(sin < 0.0f)
		return odd->sample(u,v,p);
	return even->sample(u,v,p);
}
