
#include "texture.h"
#include <stb_image.h>

constant constant::make(v3 c) {
	constant ret;
	ret.color = c;
	return ret;
}

v3 constant::sample(v2, v3) const {
	return color;
}

checkerboard checkerboard::make(texture* o, texture* e) {
	checkerboard ret;
	ret.odd = o;
	ret.even = e;
	return ret;
}

v3 checkerboard::sample(v2 uv, v3 p) const {

	f32 sin = sinf(10.0f * p.x) * sinf(10.0f * p.y) * sinf(10.0f * p.z);
	
	if(sin < 0.0f)
		return odd->sample(uv,p);
	return even->sample(uv,p);
}

noise noise::make(v3 loc, f32 scale) {
	noise ret;
	ret.scale = scale;
	ret.loc = loc;
	return ret;
}

v3 noise::sample(v2, v3 p) const {
	p += loc;
	return 0.5f * (1.0f + sin(scale * p.x + 10.0f * g_perlin.turb(p, 7)));
}

image image::make(std::string file) {

	image ret;
	ret.data = stbi_load(file.c_str(), &ret.w, &ret.h, null, 4);

	if(!ret.data) {
		std::cout << "Failed to load image from file " << file << "!" << std::endl;
	}

	return ret;
}

void image::destroy() {

	stbi_image_free(data);
	data = null;
	w = h = 0;
}

v3 image::sample(v2 uv, v3) const {

	// TODO(max): texture filtering / mip-mapping?
	// TODO(max): transparency?

	i32 tx = (i32)clamp(uv.x * w, 0.0f, (f32)(w - 1));
	i32 ty = (i32)clamp((1.0f - uv.y) * h, 0.0f, (f32)(h - 1));

	f32 r = data[4*ty*w + 4*tx] / 255.0f;
	f32 g = data[4*ty*w + 4*tx + 1] / 255.0f;
	f32 b = data[4*ty*w + 4*tx + 2] / 255.0f;

	return pow(v3(r,g,b), 2.2f);
}
