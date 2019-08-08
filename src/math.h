
#pragma once

#include "basic.h"

#include <math.h>
#include <immintrin.h>
#include <xmmintrin.h>
#include <random>

#ifdef _MSC_VER
#pragma warning(disable : 4201)
#define VEC __vectorcall
#else
#define VEC
#endif

#define PI32 3.14159265358979323846264338327950288f
#define RADIANS(v) (v * (PI32 / 180.0f)) 
#define DEGREES(v) (v * (180.0f / PI32)) 

// Technically a v4, and wasting a float. But at least the ops are somewhat accelerated.
// The real speed-up comes from using the float and v3 lanes from math_simd, as these can
// be AVX2 8x as well as SoA vector calculations and such.
union v3 {
	struct {
		f32 x, y, z, _w;
	};
	__m128 v;
	f32 a[4] = {};

	void VEC operator+=(const v3 o) {v = _mm_add_ps(v,o.v);}
	void VEC operator-=(const v3 o) {v = _mm_sub_ps(v,o.v);}
	void VEC operator*=(const v3 o) {v = _mm_mul_ps(v,o.v);}
	void VEC operator/=(const v3 o) {v = _mm_div_ps(v,o.v);}
	void operator*=(f32 s) {v = _mm_mul_ps(v,_mm_set1_ps(s));}
	void operator/=(f32 s) {v = _mm_div_ps(v,_mm_set1_ps(s));}
	f32& operator[](i32 idx) {return a[idx];}
	v3 VEC operator-() const {return _mm_sub_ps(_mm_setzero_ps(),v);}

	v3() {}
	v3(f32 _x) {v = _mm_set1_ps(_x);}
	v3(__m128 _v) {v = _v;}
	v3(f32 _x, f32 _y, f32 _z) {v = _mm_set_ps(0.0f, _z, _y, _x);}
	v3(i32 _x, i32 _y, i32 _z) {v = _mm_set_ps(0.0f, (f32)_z, (f32)_y, (f32)_x);}
	v3(const v3& o) {v = o.v;}
	v3(const v3&& o) {v = o.v;}
	v3& VEC operator=(const v3& o) {v = o.v; return *this;}
	v3& VEC operator=(const v3&& o) {v = o.v; return *this;}
};
static_assert(sizeof(v3) == 16, "sizeof(v3) != 16");

inline v3 VEC operator+(const v3 l, const v3 r) {
	return {_mm_add_ps(l.v,r.v)};
}
inline v3 VEC operator+(const v3 l, const f32 r) {
	return {_mm_add_ps(l.v,_mm_set1_ps(r))};
}
inline v3 VEC operator+(const f32 l, const v3 r) {
	return {_mm_add_ps(_mm_set1_ps(l),r.v)};
}
inline v3 VEC operator-(const v3 l, const v3 r) {
	return {_mm_sub_ps(l.v,r.v)};
}
inline v3 VEC operator-(const v3 l, const f32 r) {
	return {_mm_sub_ps(l.v,_mm_set1_ps(r))};
}
inline v3 VEC operator-(const f32 l, const v3 r) {
	return {_mm_sub_ps(_mm_set1_ps(l),r.v)};
}
inline v3 VEC operator*(const v3 l, const v3 r) {
	return {_mm_mul_ps(l.v,r.v)};
}
inline v3 VEC operator*(const v3 l, const f32 r) {
	return {_mm_mul_ps(l.v,_mm_set1_ps(r))};
}
inline v3 VEC operator*(const f32 l, const v3 r) {
	return {_mm_mul_ps(_mm_set1_ps(l),r.v)};
}
inline v3 VEC operator/(const v3 l, const v3 r) {
	return {_mm_div_ps(l.v,r.v)};
}
inline v3 VEC operator/(const v3 l, const f32 r) {
	return {_mm_div_ps(l.v,_mm_set1_ps(r))};
}
inline v3 VEC operator/(const f32 l, const v3 r) {
	return {_mm_div_ps(_mm_set1_ps(l),r.v)};
}
inline bool VEC operator==(const v3 l, const v3 r) {
	return (_mm_movemask_ps(_mm_cmpeq_ps(l.v, r.v)) & 0b111) == 0b111;
}
inline bool VEC operator!=(const v3 l, const v3 r) {
	return (_mm_movemask_ps(_mm_cmpeq_ps(l.v, r.v)) & 0b111) != 0b111;
}
inline bool close(const f32 l, const f32 r, const f32 e) {
	return (l >= r - e) && (l <= r + e);
}
inline bool VEC close(const v3 l, const v3 r, f32 e) {
	return close(l.x,r.x,e) && close(l.y,r.y,e) && close(l.z,r.z,e);
}

//TODO(max): SIMD - _mm_pow_ps needs Intel SVML or another library for log/exp 
inline v3 VEC pow(const v3 v, const f32 r) {
	return {pow(v.x,r),pow(v.y,r),pow(v.z,r)};
}
inline f32 dot(const v3 l, const v3 r) {
	return v3{_mm_dp_ps(l.v, r.v, 0b01110001)}.x;
}

inline v3 VEC reflect(const v3 v, const v3 n) {
	return v - 2.0f * dot(n, v) * n;
}
inline f32 lensq(const v3 v) {
	return dot(v, v);
}
inline f32 len(const v3 v) {
	return sqrtf(lensq(v));
}

inline v3 VEC norm(const v3 v) {
	return v / len(v);
}
inline v3 VEC cross(v3 l, v3 r) {
	__m128 ret = _mm_sub_ps(
		_mm_mul_ps(r.v, _mm_shuffle_ps(l.v, l.v, _MM_SHUFFLE(3, 0, 2, 1))), 
		_mm_mul_ps(l.v, _mm_shuffle_ps(r.v, r.v, _MM_SHUFFLE(3, 0, 2, 1))));
	return {_mm_shuffle_ps(ret, ret, _MM_SHUFFLE(3, 0, 2, 1 ))};
}

inline v3 VEC lerp(v3 min, v3 max, f32 dist) {
	return (max - min) * dist + min;
}

std::ostream& operator<<(std::ostream& out, const v3 r) {
	out << "{" << r.x << "," << r.y << "," << r.z << "}";
	return out;
}

f32 safe(f32 f) {
	return isnan(f) ? 0.0f : f;
}
v3 safe(const v3 v) {
	return (isnan(v.x) || isnan(v.y) || isnan(v.z)) ? v3{} : v;
}

struct rand_state {
	u32 x = 123456789;
	u32 y = 362436069;
	u32 z = 521288629;
};
static rand_state __state;
inline u32 randomu() {
	u32 t;
	__state.x ^= __state.x << 16;
	__state.x ^= __state.x >> 5;
	__state.x ^= __state.x << 1;
	t = __state.x;
	__state.x = __state.y;
	__state.y = __state.z;
	__state.z = t ^ __state.x ^ __state.y;
	return __state.z;
}
inline f32 randomf() {
	return (f32)randomu() / UINT32_MAX;
}
inline v3 random_leunit() {
	v3 v;
	do {
		v = 2.0f * v3(randomf(),randomf(),randomf()) - v3(1.0f);
	} while(lensq(v) >= 1.0f);
	return v;
}
inline v3 random_ledisk() {
	v3 v;
	do {
		v = 2.0f * v3(randomf(),randomf(),0.0f) - v3(1.0f,1.0f,0.0f);
	} while(lensq(v) >= 1.0f);
	return v;	
}

struct ray {
	v3 pos, dir;
	f32 t = 0.0f;
	v3 get(f32 d) const {return pos + d * dir;}
};


void test_math() {
	{
		v3 _0(0.5f,1.0f,2.0f);
		v3 _1(3.0f,4.0f,5.0f);

		std::cout << _0 << std::endl << _1 << std::endl;

		std::cout << "add: " << _0 + _1 << std::endl;
		std::cout << "add: " << _1 + _0 << std::endl;
		std::cout << "sub: " << _0 - _1 << std::endl;
		std::cout << "sub: " << _1 - _0 << std::endl;
		std::cout << "mul: " << _0 * _1 << std::endl;
		std::cout << "mul: " << _1 * _0 << std::endl;
		std::cout << "div: " << _0 / _1 << std::endl;
		std::cout << "div: " << _1 / _0 << std::endl;
		std::cout << "eq:  " << (_0 == _0) << std::endl;
		std::cout << "eq:  " << (_0 == _1) << std::endl;
		std::cout << "eq:  " << (_1 == _0) << std::endl;
		std::cout << "neq: " << (_0 != _0) << std::endl;
		std::cout << "neq: " << (_0 != _1) << std::endl;
		std::cout << "neq: " << (_1 != _0) << std::endl;
		std::cout << "pow: " << pow(_0,0.0f) << std::endl;
		std::cout << "pow: " << pow(_0,0.5f) << std::endl;
		std::cout << "pow: " << pow(_0,1.0f) << std::endl;
		std::cout << "pow: " << pow(_0,3.0f) << std::endl;
		std::cout << "dot: " << dot(_0,_1) << std::endl;
		std::cout << "cross: " << cross(_0,_1) << std::endl;
		std::cout << "cross: " << cross(_1,_0) << std::endl;
		std::cout << "lensq: " << lensq(_0) << std::endl;
		std::cout << "lensq: " << lensq(_1) << std::endl;
		std::cout << "len: " << len(_0) << std::endl;
		std::cout << "len: " << len(_1) << std::endl;
		std::cout << "norm: " << norm(_0) << std::endl;
		std::cout << "norm: " << norm(_1) << std::endl;
		std::cout << "lerp: " << lerp(_0,_1,0.5f) << std::endl;
		std::cout << "lerp: " << lerp(_1,_0,0.5f) << std::endl;
	}
}

#include "math_simd.h"
