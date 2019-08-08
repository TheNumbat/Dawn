
#pragma once

#include "basic.h"

#include <iostream>

#include <math.h>
#include <immintrin.h>
#include <xmmintrin.h>

#define LANE_WIDTH 8

#if LANE_WIDTH==8
#define __lane __m256
#define __add_ps _mm256_add_ps
#define __sub_ps _mm256_sub_ps
#define __mul_ps _mm256_mul_ps
#define __div_ps _mm256_div_ps
#define __or_ps _mm256_or_ps
#define __and_ps _mm256_and_ps
#define __set1_ps _mm256_set1_ps
#define __sqrt_ps _mm256_sqrt_ps
#define __setzero_ps _mm256_setzero_ps
#define __casti_ps _mm256_castsi256_ps
#define __cmp_ps _mm256_cmp_ps
#define __set1_epi32 _mm256_set1_epi32
#define __xor_ps _mm256_xor_ps
#define __hadd_ps _mm256_hadd_ps
#define __blendv_ps _mm256_blendv_ps
#define __movemask_ps _mm256_movemask_ps
#define __min_ps _mm256_min_ps
#define __shuffle_ps _mm256_shuffle_ps
#define __min_ps _mm256_min_ps
#define __max_ps _mm256_max_ps
#elif LANE_WIDTH==4
#define __lane __m128
#define __add_ps _mm_add_ps
#define __sub_ps _mm_sub_ps
#define __mul_ps _mm_mul_ps
#define __div_ps _mm_div_ps
#define __or_ps _mm_or_ps
#define __and_ps _mm_and_ps
#define __set1_ps _mm_set1_ps
#define __sqrt_ps _mm_sqrt_ps
#define __setzero_ps _mm_setzero_ps
#define __casti_ps _mm_castsi128_ps
#define __cmp_ps _mm_cmp_ps
#define __set1_epi32 _mm_set1_epi32
#define __xor_ps _mm_xor_ps
#define __hadd_ps _mm_hadd_ps
#define __blendv_ps _mm_blendv_ps
#define __movemask_ps _mm_movemask_ps
#define __min_ps _mm_min_ps
#define __shuffle_ps _mm_shuffle_ps
#define __min_ps _mm_min_ps
#define __max_ps _mm_max_ps
#else
#error "LANE_WIDTH not 4 or 8"
#endif

#ifdef _MSC_VER
#pragma warning(push)
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

union f32_lane {
	__lane v;
	i32 i[LANE_WIDTH];
	f32 f[LANE_WIDTH] = {};

	void operator+=(const f32_lane& x) {v = __add_ps(v, x.v);}
	void operator-=(const f32_lane& x) {v = __sub_ps(v, x.v);}
	void operator*=(const f32_lane& x) {v = __mul_ps(v, x.v);}
	void operator/=(const f32_lane& x) {v = __div_ps(v, x.v);}

	void operator*=(f32 _s) {__lane s = __set1_ps(_s);
							 v = __mul_ps(v, s);}
	void operator/=(f32 _s) {__lane s = __set1_ps(_s);
							 v = __div_ps(v, s);}

	void operator|=(const f32_lane& x) {v = __or_ps(v,x.v);}
	void operator&=(const f32_lane& x) {v = __and_ps(v,x.v);}
	inline f32_lane operator~() {return {__xor_ps(v,__casti_ps(__set1_epi32(0xffffffff)))};}

	inline f32_lane operator-() {__lane _z = __setzero_ps();
						  return {__sub_ps(_z,v)};}

	void set_all_int(i32 _i) {v = __casti_ps(__set1_epi32(_i));}

	f32_lane() {}
	f32_lane(f32 _v) {v = __set1_ps(_v);}
	f32_lane(i32 _v) {v = __casti_ps(__set1_epi32(_v));}
	f32_lane(__lane _v) {v = _v;}

	f32_lane(const f32_lane& o) {memcpy(this,&o,sizeof(f32_lane));}
	f32_lane(const f32_lane&& o) {memcpy(this,&o,sizeof(f32_lane));}
	inline f32_lane VEC operator=(const f32_lane& o) {memcpy(this,&o,sizeof(f32_lane)); return *this;}
	inline f32_lane VEC operator=(const f32_lane&& o) {memcpy(this,&o,sizeof(f32_lane)); return *this;}
};
static_assert(sizeof(f32_lane) == LANE_WIDTH * 4, "sizeof(f32_lane) != LANE_WIDTH * 4");

union v3_lane {
	struct {
		__lane x, y, z;
	};
	struct {
		f32 xf[LANE_WIDTH], yf[LANE_WIDTH], zf[LANE_WIDTH];
	};
	f32 af[3 * LANE_WIDTH];
	f32_lane v[3];
	__lane a[3] = {};

	void VEC operator+=(const v3_lane& o) {x = __add_ps(x, o.x);
						   		y = __add_ps(y, o.y);
						   		z = __add_ps(z, o.z);}
	void VEC operator-=(const v3_lane& o) {x = __sub_ps(x, o.x);
						   		y = __sub_ps(y, o.y);
						   		z = __sub_ps(z, o.z);}
	void VEC operator*=(const v3_lane& o) {x = __mul_ps(x, o.x);
						   		y = __mul_ps(y, o.y);
						   		z = __mul_ps(z, o.z);}
	void VEC operator/=(const v3_lane& o) {x = __div_ps(x, o.x);
						   		y = __div_ps(y, o.y);
						   		z = __div_ps(z, o.z);}
	void VEC operator*=(f32_lane s) {
							x = __mul_ps(x, s.v);
						    y = __mul_ps(y, s.v);
						    z = __mul_ps(z, s.v);}
	void VEC operator/=(f32_lane s) {
							x = __div_ps(x, s.v);
						    y = __div_ps(y, s.v);
						    z = __div_ps(z, s.v);}
	void operator*=(f32 s) {__lane _s = __set1_ps(s);
							x = __mul_ps(x, _s);
						    y = __mul_ps(y, _s);
						    z = __mul_ps(z, _s);}
	void operator/=(f32 s) {__lane _s = __set1_ps(s);
							x = __div_ps(x, _s);
						    y = __div_ps(y, _s);
						    z = __div_ps(z, _s);}

	void VEC operator|=(f32_lane o) {x = __or_ps(x,o.v);
								 y = __or_ps(y,o.v);
								 z = __or_ps(z,o.v);}
	void VEC operator&=(f32_lane o) {x = __and_ps(x,o.v);
								 y = __and_ps(y,o.v);
								 z = __and_ps(z,o.v);}
	void VEC operator|=(const v3_lane& o) {x = __or_ps(x,o.x);
								y = __or_ps(y,o.y);
								z = __or_ps(z,o.z);}
	void VEC operator&=(const v3_lane& o) {x = __and_ps(x,o.x);
								y = __and_ps(y,o.y);
								z = __and_ps(z,o.z);}

	v3_lane VEC operator-() const {__lane _z = __setzero_ps();
					return {__sub_ps(_z,x),
							__sub_ps(_z,y),
							__sub_ps(_z,z)};}

	v3 operator[](i32 idx) const {return {xf[idx],yf[idx],zf[idx]};}
	void set(i32 idx, v3 o) {xf[idx] = o.x; yf[idx] = o.y; zf[idx] = o.z;}

	v3_lane() {}
	v3_lane(f32 _x) {x = y = z = __set1_ps(_x);}
	v3_lane(const f32_lane& _x) {x = y = z = _x.v;}
	v3_lane(f32 _x, f32 _y, f32 _z) {x = __set1_ps(_x); 
									 y = __set1_ps(_y); 
									 z = __set1_ps(_z);}
	v3_lane(v3 _v) {x = __set1_ps(_v.x); 
					y = __set1_ps(_v.y); 
					z = __set1_ps(_v.z);}
	v3_lane(f32_lane _x, f32_lane _y, f32_lane _z) {
									 x = _x.v; 
									 y = _y.v; 
									 z = _z.v;}

	v3_lane(const v3_lane& o) {memcpy(this,&o,sizeof(v3_lane));}
	v3_lane(const v3_lane&& o) {memcpy(this,&o,sizeof(v3_lane));}
	inline v3_lane VEC operator=(const v3_lane& o) {memcpy(this,&o,sizeof(v3_lane)); return *this;}
	inline v3_lane VEC operator=(const v3_lane&& o) {memcpy(this,&o,sizeof(v3_lane)); return *this;}
};
static_assert(sizeof(v3_lane) == LANE_WIDTH * 12, "sizeof(v3_lane) != LANE_WIDTH * 12");

struct rand_state {
	u32 x = 123456789;
	u32 y = 362436069;
	u32 z = 521288629;
};
extern rand_state __state;
inline u32 randomu();
inline f32 randomf();
inline v3 random_leunit();
inline v3 random_ledisk();
inline f32_lane randf_lane();
inline v3_lane random_leunit_lane();
inline v3_lane random_ledisk_lane();

struct ray {
	v3 pos, dir;
	f32 t = 0.0f;
	v3 get(f32 d) const;
};

struct ray_lane {
	v3_lane pos, dir;
	v3_lane get(const f32_lane& t) const;
};

inline v3 VEC operator+(const v3 l, const v3 r);
inline v3 VEC operator+(const v3 l, const f32 r);
inline v3 VEC operator+(const f32 l, const v3 r);
inline v3 VEC operator-(const v3 l, const v3 r);
inline v3 VEC operator-(const v3 l, const f32 r);
inline v3 VEC operator-(const f32 l, const v3 r);
inline v3 VEC operator*(const v3 l, const v3 r);
inline v3 VEC operator*(const v3 l, const f32 r);
inline v3 VEC operator*(const f32 l, const v3 r);
inline v3 VEC operator/(const v3 l, const v3 r);
inline v3 VEC operator/(const v3 l, const f32 r);
inline v3 VEC operator/(const f32 l, const v3 r);
inline bool VEC operator==(const v3 l, const v3 r);
inline bool VEC operator!=(const v3 l, const v3 r);
inline bool close(const f32 l, const f32 r, const f32 e);
inline bool VEC close(const v3 l, const v3 r, f32 e);

inline v3 VEC vmin(const v3 l, const v3 r);
inline v3 VEC vmax(const v3 l, const v3 r);
inline v3 VEC eq_mask(const v3 l, const v3 r);
inline v3 VEC neq_mask(const v3 l, const v3 r);
inline v3 VEC operator>(const v3 l, const v3 r);
inline v3 VEC operator<(const v3 l, const v3 r);
inline v3 VEC operator>=(const v3 l, const v3 r);
inline v3 VEC operator<=(const v3 l, const v3 r);
inline bool VEC none(const v3 v);
inline bool VEC all(const v3 v);
inline bool VEC any(const v3 v);

inline v3 VEC pow(const v3 v, const f32 r);
inline f32 dot(const v3 l, const v3 r);

inline v3 VEC reflect(const v3 v, const v3 n);
inline f32 lensq(const v3 v);
inline f32 len(const v3 v);

inline v3 VEC norm(const v3 v);
inline v3 VEC cross(v3 l, v3 r);

inline v3 VEC lerp(v3 min, v3 max, f32 dist);

inline f32 safe(f32 f);
inline v3 safe(const v3 v);

inline v3_lane VEC operator+(const v3_lane& l, const v3_lane& r);
inline v3_lane VEC operator+(const v3_lane& l, const f32 _r);
inline v3_lane VEC operator+(const f32 _l, const v3_lane& r);
inline v3_lane VEC operator+(const v3_lane& l, const v3 r);
inline v3_lane VEC operator+(const v3 l, const v3_lane& r);
inline v3_lane VEC operator-(const v3_lane& l, const v3_lane& r);
inline v3_lane VEC operator-(const v3_lane& l, const f32 _r);
inline v3_lane VEC operator-(const f32 _l, const v3_lane& r);
inline v3_lane VEC operator-(const v3_lane& l, const v3 r);
inline v3_lane VEC operator-(const v3 l, const v3_lane& r);
inline v3_lane VEC operator*(const v3_lane& l, const v3_lane& r);
inline v3_lane VEC operator*(const v3_lane& l, const f32 _r);
inline v3_lane VEC operator*(const v3_lane& l, const v3 r);
inline v3_lane VEC operator*(const v3 l, const v3_lane& r);
inline v3_lane VEC operator*(const f32 _l, const v3_lane& r);
inline v3_lane VEC operator/(const v3_lane& l, const v3_lane& r);
inline v3_lane VEC operator/(const v3_lane& l, const f32 _r);
inline v3_lane VEC operator/(const f32 _l, const v3_lane& r);
inline v3_lane VEC operator/(const v3_lane& l, const v3 r);
inline v3_lane VEC operator/(const v3 l, const v3_lane& r);

inline f32_lane VEC operator==(const v3_lane& l, const v3_lane& r);
inline f32_lane VEC operator!=(const v3_lane& l, const v3_lane& r);

inline bool VEC none(const f32_lane& v);
inline bool VEC all(const f32_lane& v);
inline bool VEC any(const f32_lane& v);

// NOTE(max): mask 1: left 0: right
inline f32_lane select(const f32_lane& l, const f32_lane& r, const f32_lane& m);

inline f32_lane VEC operator&(const f32_lane& l, const f32_lane& r);
inline f32_lane VEC operator|(const f32_lane& l, const f32_lane& r);

inline v3_lane VEC operator&(const v3_lane& l, const f32_lane& r);
inline v3_lane VEC operator|(const v3_lane& l, const f32_lane& r);
inline v3_lane VEC operator&(const f32_lane& l, const v3_lane& r);
inline v3_lane VEC operator|(const f32_lane& l, const v3_lane& r);

inline f32_lane VEC operator+(const f32_lane& l, const f32_lane& r);
inline f32_lane VEC operator+(const f32_lane& l, const f32 _r);
inline f32_lane VEC operator+(const f32 _l, const f32_lane& r);
inline f32_lane VEC operator-(const f32_lane& l, const f32_lane& r);
inline f32_lane VEC operator-(const f32_lane& l, const f32 _r);
inline f32_lane VEC operator-(const f32 _l, const f32_lane& r);
inline f32_lane VEC operator*(const f32_lane& l, const f32_lane& r);
inline f32_lane VEC operator*(const f32_lane& l, const f32 _r);
inline f32_lane VEC operator*(const f32 _l, const f32_lane& r);
inline f32_lane VEC operator/(const f32_lane& l, const f32_lane& r);
inline f32_lane VEC operator/(const f32_lane& l, const f32 _r);
inline f32_lane VEC operator/(const f32 _l, const f32_lane& r);

inline f32_lane VEC operator==(const f32_lane& l, const f32_lane& r);
inline f32_lane VEC operator!=(const f32_lane& l, const f32_lane& r);
inline f32_lane VEC operator>(const f32_lane& l, const f32_lane& r);
inline f32_lane VEC operator<(const f32_lane& l, const f32_lane& r);
inline f32_lane VEC operator>=(const f32_lane& l, const f32_lane& r);
inline f32_lane VEC operator<=(const f32_lane& l, const f32_lane& r);
inline f32_lane VEC operator==(const f32_lane& l, f32 r);
inline f32_lane VEC operator!=(const f32_lane& l, f32 r);
inline f32_lane VEC operator>(const f32_lane& l, f32 r);
inline f32_lane VEC operator<(const f32_lane& l, f32 r);
inline f32_lane VEC operator>=(const f32_lane& l, f32 r);
inline f32_lane VEC operator<=(const f32_lane& l, f32 r);
inline f32_lane VEC operator==(f32 l, const f32_lane& r);
inline f32_lane VEC operator!=(f32 l, const f32_lane& r);
inline f32_lane VEC operator>(f32 l, const f32_lane& r);
inline f32_lane VEC operator<(f32 l, const f32_lane& r);
inline f32_lane VEC operator>=(f32 l, const f32_lane& r);
inline f32_lane VEC operator<=(f32 l, const f32_lane& r);

inline f32_lane VEC sqrt(const f32_lane& l);

inline f32_lane VEC vmin(const f32_lane& l, const f32_lane& r);
inline f32_lane VEC vmax(const f32_lane& l, const f32_lane& r);

std::ostream& operator<<(std::ostream& out, const v3 r);
std::ostream& VEC operator<<(std::ostream& out, const v3_lane& r);
std::ostream& VEC operator<<(std::ostream& out, const f32_lane& r);

inline f32 VEC hsum(const f32_lane& l);
inline f32 VEC hmin(const f32_lane& l);
inline f32 VEC hmax(const f32_lane& l);

inline v3 VEC hmin(const v3_lane& l);
inline v3 VEC hmax(const v3_lane& l);

inline v3_lane VEC operator*(const v3 l, const f32_lane& r);

inline f32_lane VEC dot(const v3_lane& l, const v3_lane& r);
inline v3_lane VEC reflect(const v3_lane& v, const v3_lane& n);
inline f32_lane VEC lensq(const v3_lane& v);
inline f32_lane VEC len(const v3_lane& v);
inline v3_lane VEC norm(const v3_lane& v);
inline v3_lane VEC lerp(const v3_lane& min, const v3_lane& max, const f32_lane& dist);

inline v3_lane VEC cross(const v3_lane& l, const v3_lane& r);

void test_math();

#ifdef _MSC_VER
#pragma warning(pop)
#endif
