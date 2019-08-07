
#pragma once

#include "basic.h"

#include <math.h>
#include <random>

#ifdef _MSC_VER
#pragma warning(disable : 4201)
#endif

std::random_device rd;
std::mt19937 rand_gen;
std::uniform_real_distribution<f32> dis;

#define PI32 3.14159265358979323846264338327950288f
#define RADIANS(v) (v * (PI32 / 180.0f)) 
#define DEGREES(v) (v * (180.0f / PI32)) 

union v3 {
	struct {
		f32 x, y, z;
	};
	f32 a[3] = {};

	void operator+=(const v3 v) {x += v.x; y += v.y; z += v.z;}
	void operator-=(const v3 v) {x -= v.x; y -= v.y; z -= v.z;}
	void operator*=(const v3 v) {x *= v.x; y *= v.y; z *= v.z;}
	void operator/=(const v3 v) {x /= v.x; y /= v.y; z /= v.z;}
	void operator*=(f32 s) {x *= s; y *= s; z *= s;}
	void operator/=(f32 s) {x /= s; y /= s; z /= s;}
	f32& operator[](i32 idx) {return a[idx];}
	v3 operator-() {return {-x,-y,-z};}

	v3() {}
	v3(f32 _x) {x = _x; y = _x; z = _x;}
	v3(f32 _x, f32 _y, f32 _z) {x = _x; y = _y; z = _z;}
	v3(i32 _x, i32 _y, i32 _z) {x = (f32)_x; y = (f32)_y; z = (f32)_z;}
	v3(const v3& v) {*this = v;}
	v3(const v3&& v) {*this = v;}
	v3& operator=(const v3& v) {x = v.x; y = v.y; z = v.z; return *this;}
	v3& operator=(const v3&& v) {x = v.x; y = v.y; z = v.z; return *this;}
};
static_assert(sizeof(v3) == 12, "sizeof(v3) != 12");

v3 operator+(const v3 l, const v3 r) {
	return {l.x + r.x, l.y + r.y, l.z + r.z};	
}
v3 operator+(const v3 l, const f32 r) {
	return {l.x + r, l.y + r, l.z + r};	
}
v3 operator+(const f32 l, const v3 r) {
	return {l + r.x, l + r.y, l + r.z};	
}
v3 operator-(const v3 l, const v3 r) {
	return {l.x - r.x, l.y - r.y, l.z - r.z};
}
v3 operator-(const v3 l, const f32 r) {
	return {l.x - r, l.y - r, l.z - r};	
}
v3 operator-(const f32 l, const v3 r) {
	return {l - r.x, l - r.y, l - r.z};	
}
v3 operator*(const v3 l, const v3 r) {
	return {l.x * r.x, l.y * r.y, l.z * r.z};
}
v3 operator*(const v3 l, const f32 r) {
	return {l.x * r, l.y * r, l.z * r};
}
v3 operator*(const f32 l, const v3 r) {
	return {r.x * l, r.y * l, r.z * l};
}
v3 operator/(const v3 l, const v3 r) {
	return {l.x / r.x, l.y / r.y, l.z / r.z};
}
v3 operator/(const v3 l, const f32 r) {
	return {l.x / r, l.y / r, l.z / r};
}
v3 operator/(const f32 l, const v3 r) {
	return {l / r.x, l / r.y, l / r.z};
}
bool operator==(const v3 l, const v3 r) {
	return l.x == r.x && l.y == r.y && l.z == r.z;
}
bool operator!=(const v3 l, const v3 r) {
	return l.x != r.x || l.y != r.y || l.z != r.z;
}
bool close(const f32 l, const f32 r, const f32 e) {
	return (l >= r - e) && (l <= r + e);
}
bool close(const v3 l, const v3 r, f32 e) {
	return close(l.x,r.x,e) && close(l.y,r.y,e) && close(l.z,r.z,e);
}
v3 pow(const v3 v, const f32 r) {
	return {powf(v.x,r),powf(v.y,r),powf(v.z,r)};
}

f32 dot(const v3 l, const v3 r) {
	return l.x * r.x + l.y * r.y + l.z * r.z;
}
const v3 reflect(const v3 v, const v3 n) {
	return v - 2.0f * dot(n, v) * n;
}
const f32 lensq(const v3 v) {
	return dot(v, v);
}
const f32 len(const v3 v) {
	return sqrtf(lensq(v));
}
v3 norm(const v3 v) {
	return v / len(v);
}
v3 cross(const v3 l, const v3 r) {
	return {l.y * r.z - l.z * r.y, l.z * r.x - l.x * r.z, l.x * r.y - l.y * r.x};
}
const v3 lerp(v3 min, v3 max, f32 dist) {
	return (max - min) * dist + min;
}

std::ostream& operator<<(std::ostream& out, const v3 r) {
	out << "{" << r.x << "," << r.y << "," << r.z << "}";
	return out;
}

void seed_rand() {
	rand_gen.seed(rd());
}
const f32 randf_cpp() {
	return dis(rand_gen);
}
const v3 random_leunit() {
	v3 v;
	do {
		v = 2.0f * v3(randf_cpp(),randf_cpp(),randf_cpp()) - v3(1.0f);
	} while(lensq(v) >= 1.0f);
	return v;
}
const v3 random_ledisk() {
	v3 v;
	do {
		v = 2.0f * v3(randf_cpp(),randf_cpp(),0.0f) - v3(1.0f,1.0f,0.0f);
	} while(lensq(v) >= 1.0f);
	return v;	
}

struct ray {
	v3 pos, dir;
	v3 get(f32 t) {return pos + t * dir;}
};

#include "math_simd.h"
