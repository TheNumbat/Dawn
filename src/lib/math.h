
#pragma once

#include "basic.h"

#include <iostream>

#include <string.h>
#include <math.h>
#include <immintrin.h>
#include <xmmintrin.h>

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

#define PI32  (3.14159265358979323846264338327950288f)
#define TAU32 (2.0f * 3.14159265358979323846264338327950288f)
#define RADIANS(v) (v * (PI32 / 180.0f)) 
#define DEGREES(v) (v * (180.0f / PI32)) 

union v2 {
	struct {
		f32 x, y;
	};
	f32 a[2] = {};

	void operator+=(const v2 o) {x += o.x; y += o.y;}
	void operator-=(const v2 o) {x -= o.x; y -= o.y;}
	void operator*=(const v2 o) {x *= o.x; y *= o.y;}
	void operator/=(const v2 o) {x /= o.x; y /= o.y;}
	void operator*=(f32 s) {x *= s; y *= s;}
	void operator/=(f32 s) {x /= s; y /= s;}
	f32& operator[](i32 idx) {return a[idx];}
	f32 operator[](i32 idx) const {return a[idx];}
	v2 operator-() const {return {-x,-y};}

	v2() {}
	v2(f32 _x) {x = _x; y = _x;}
	v2(f32 _x, f32 _y) {x = _x; y = _y;}
	v2(i32 _x, i32 _y) {x = (f32)_x; y = (f32)_y;}
	v2(const v2& o) {x = o.x; y = o.y;}
	v2(const v2&& o) {x = o.x; y = o.y;}
	v2& operator=(const v2& o) {x = o.x; y = o.y; return *this;}
	v2& operator=(const v2&& o) {x = o.x; y = o.y; return *this;}
};
static_assert(sizeof(v2) == 8, "sizeof(v2) != 8");

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
	f32 operator[](i32 idx) const {return a[idx];}
	v3 VEC operator-() const {return _mm_sub_ps(_mm_setzero_ps(),v);}

	v3() {}
	v3(f32 _x) {v = _mm_set1_ps(_x);}
	v3(__m128 _v) {v = _v;}
	v3(v2 xy, f32 _z) {v = _mm_set_ps(0.0f, _z, xy.y, xy.x);}
	v3(f32 _x, v2 yz) {v = _mm_set_ps(0.0f, yz.y, yz.x, _x);}
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

	f32_lane(const f32_lane& o) {v = o.v;}
	f32_lane(const f32_lane&& o) {v = o.v;}
	inline f32_lane VEC operator=(const f32_lane& o) {v = o.v; return *this;}
	inline f32_lane VEC operator=(const f32_lane&& o) {v = o.v; return *this;}
};
static_assert(sizeof(f32_lane) == LANE_WIDTH * 4, "sizeof(f32_lane) != LANE_WIDTH * 4");

union v2_lane {
	struct {
		__lane x, y;
	};
	struct {
		f32 xf[LANE_WIDTH], yf[LANE_WIDTH];
	};
	f32 af[2 * LANE_WIDTH];
	f32_lane v[2];
	__lane a[2] = {};

	void VEC operator+=(const v2_lane& o) {x = __add_ps(x, o.x);
						   		y = __add_ps(y, o.y);}
	void VEC operator-=(const v2_lane& o) {x = __sub_ps(x, o.x);
						   		y = __sub_ps(y, o.y);}
	void VEC operator*=(const v2_lane& o) {x = __mul_ps(x, o.x);
						   		y = __mul_ps(y, o.y);}
	void VEC operator/=(const v2_lane& o) {x = __div_ps(x, o.x);
						   		y = __div_ps(y, o.y);}
	void VEC operator*=(f32_lane s) {
							x = __mul_ps(x, s.v);
						    y = __mul_ps(y, s.v);}
	void VEC operator/=(f32_lane s) {
							x = __div_ps(x, s.v);
						    y = __div_ps(y, s.v);}
	void operator*=(f32 s) {__lane _s = __set1_ps(s);
							x = __mul_ps(x, _s);
						    y = __mul_ps(y, _s);}
	void operator/=(f32 s) {__lane _s = __set1_ps(s);
							x = __div_ps(x, _s);
						    y = __div_ps(y, _s);}

	void VEC operator|=(f32_lane o) {x = __or_ps(x,o.v);
								 y = __or_ps(y,o.v);}
	void VEC operator&=(f32_lane o) {x = __and_ps(x,o.v);
								 y = __and_ps(y,o.v);}
	void VEC operator|=(const v2_lane& o) {x = __or_ps(x,o.x);
								y = __or_ps(y,o.y);}
	void VEC operator&=(const v2_lane& o) {x = __and_ps(x,o.x);
								y = __and_ps(y,o.y);}

	v2_lane VEC operator-() const {__lane _z = __setzero_ps();
					return {__sub_ps(_z,x),
							__sub_ps(_z,y)};}

	v2 operator[](i32 idx) const {return {xf[idx],yf[idx]};}
	void set(i32 idx, v2 o) {xf[idx] = o.x; yf[idx] = o.y;}

	v2_lane() {}
	v2_lane(f32 _x) {x = y = __set1_ps(_x);}
	v2_lane(const f32_lane& _x) {x = y = _x.v;}
	v2_lane(f32 _x, f32 _y) {x = __set1_ps(_x); 
									 y = __set1_ps(_y);}
	v2_lane(v2 _v) {x = __set1_ps(_v.x); 
					y = __set1_ps(_v.y);}
	v2_lane(f32_lane _x, f32_lane _y) {
									 x = _x.v; 
									 y = _y.v;}

	v2_lane(const v2_lane& o) {memcpy(this,&o,sizeof(v2_lane));}
	v2_lane(const v2_lane&& o) {memcpy(this,&o,sizeof(v2_lane));}
	inline v2_lane VEC operator=(const v2_lane& o) {memcpy(this,&o,sizeof(v2_lane)); return *this;}
	inline v2_lane VEC operator=(const v2_lane&& o) {memcpy(this,&o,sizeof(v2_lane)); return *this;}
};
static_assert(sizeof(v2_lane) == LANE_WIDTH * 8, "sizeof(v2_lane) != LANE_WIDTH * 8");

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

// NOTE(max): yes this is basically completely redundant to v3...could just
// use a strong typedef with applicable functions (e.g. dot product, m4 mul),
// but that is more confusing than just duplicating the type.
union v4 {
	struct {
		f32 x, y, z, w;
	};
	__m128 v;
	f32 a[4] = {};

	void operator+=(v4 o) {v = _mm_add_ps(v, o.v);}
	void operator-=(v4 o) {v = _mm_sub_ps(v, o.v);}
	void operator*=(v4 o) {v = _mm_mul_ps(v, o.v);}
	void operator/=(v4 o) {v = _mm_div_ps(v, o.v);}
	void operator*=(f32 s) {v = _mm_mul_ps(v, _mm_set1_ps(s));}
	void operator/=(f32 s) {v = _mm_div_ps(v, _mm_set1_ps(s));}
	f32& operator[](i32 idx) {return a[idx];}
	f32 operator[](i32 idx) const {return a[idx];}

	v4() {}
	v4(f32 _v) {v = _mm_set_ps(_v, _v, _v, _v);}
	v4(v3 _v, f32 _w) {v = _mm_set_ps(_w, _v.z, _v.y, _v.x);}
	v4(f32 _x, f32 _y, f32 _z, f32 _w) {v = _mm_set_ps(_w, _z, _y, _x);}
	v4(__m128 p) {v = p;}

	v4(const v4& o) {v = o.v;}
	v4(const v4&& o) {v = o.v;}
	inline v4 VEC operator=(const v4& o) {v = o.v; return *this;}
	inline v4 VEC operator=(const v4&& o) {v = o.v; return *this;}
};
static_assert(sizeof(v4) == 16, "sizeof(v4) != 16");

union m4;
inline m4 VEC operator*(m4 l, m4 r);

union m4 {
	f32 a[16] = {1, 0, 0, 0,
				 0, 1, 0, 0,
				 0, 0, 1, 0,
				 0, 0, 0, 1}; 
	v4 cols[4];
	__m128 v[4];

	void operator+=(m4 o) {v[0] = _mm_add_ps(v[0], o.v[0]);
						   v[1] = _mm_add_ps(v[1], o.v[1]);
						   v[2] = _mm_add_ps(v[2], o.v[2]);
						   v[3] = _mm_add_ps(v[3], o.v[3]);}
	void operator-=(m4 o) {v[0] = _mm_sub_ps(v[0], o.v[0]);
						   v[1] = _mm_sub_ps(v[1], o.v[1]);
						   v[2] = _mm_sub_ps(v[2], o.v[2]);
						   v[3] = _mm_sub_ps(v[3], o.v[3]);}
	void operator*=(m4 o) {*this = *this * o;}
	void operator*=(f32 s) {__m128 mul = _mm_set1_ps(s); for(i32 i = 0; i < 4; i++) v[i] = _mm_mul_ps(v[i], mul);}
	void operator/=(f32 s) {__m128 div = _mm_set1_ps(s); for(i32 i = 0; i < 4; i++) v[i] = _mm_div_ps(v[i], div);}

	v4& operator[](i32 idx) {return cols[idx];}
	v4 operator[](i32 idx) const {return cols[idx];}

	m4() {}
	m4(v4 c0, v4 c1, v4 c2, v4 c3) {cols[0] = c0; cols[1] = c1; cols[2] = c2; cols[3] = c3;}
	m4(const m4& o) {memcpy(this,&o,sizeof(m4));}
	m4(const m4&& o) {memcpy(this,&o,sizeof(m4));}
	inline m4 VEC operator=(const m4& o) {memcpy(this,&o,sizeof(m4)); return *this;}
	inline m4 VEC operator=(const m4&& o) {memcpy(this,&o,sizeof(m4)); return *this;}

	static m4 zero;
	static m4 I;
};
static_assert(sizeof(m4) == 64, "sizeof(m4) != 64");

struct rand_state {
	u32 x = 123456789;
	u32 y = 362436069;
	u32 z = 521288629;
};
extern rand_state __state;

struct ray {
	v3 pos, dir;
	f32 t = 0.0f;
	v3 get(f32 d) const;
};

struct ray_lane {
	v3_lane pos, dir;
	v3_lane get(const f32_lane& t) const;
};

std::ostream& operator<<(std::ostream& out, const v3 r);
std::ostream& VEC operator<<(std::ostream& out, const v3_lane& r);
std::ostream& VEC operator<<(std::ostream& out, const f32_lane& r);
void test_math();

inline v2 operator+(const v2 l, const v2 r) {
	return {l.x + r.x, l.y + r.y};
}
inline v2 operator+(const v2 l, const f32 r) {
	return {l.x + r, l.y + r};
}
inline v2 operator+(const f32 l, const v2 r) {
	return {l + r.x, l + r.y};
}
inline v2 operator-(const v2 l, const v2 r) {
	return {l.x - r.x, l.y - r.y};
}
inline v2 operator-(const v2 l, const f32 r) {
	return {l.x - r, l.y - r};
}
inline v2 operator-(const f32 l, const v2 r) {
	return {l - r.x, l - r.y};
}
inline v2 operator*(const v2 l, const v2 r) {
	return {l.x * r.x, l.y * r.y};
}
inline v2 operator*(const v2 l, const f32 r) {
	return {l.x * r, l.y * r};
}
inline v2 operator*(const f32 l, const v2 r) {
	return {l * r.x, l * r.y};
}
inline v2 operator/(const v2 l, const v2 r) {
	return {l.x / r.x, l.y / r.y};
}
inline v2 operator/(const v2 l, const f32 r) {
	return {l.x / r, l.y / r};
}
inline v2 operator/(const f32 l, const v2 r) {
	return {l / r.x, l / r.y};
}
inline bool operator==(const v2 l, const v2 r) {
	return l.x == r.x && l.y == r.y;
}
inline bool operator!=(const v2 l, const v2 r) {
	return l.x != r.x || l.y != r.y;
}

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

inline v3 VEC vmin(const v3 l, const v3 r) {
	return {_mm_min_ps(l.v,r.v)};
}
inline v3 VEC vmax(const v3 l, const v3 r) {
	return {_mm_max_ps(l.v,r.v)};
}
inline v3 VEC eq_mask(const v3 l, const v3 r) {
	return {_mm_cmp_ps(l.v,r.v,_CMP_EQ_OQ)};
}
inline v3 VEC neq_mask(const v3 l, const v3 r) {
	return {_mm_cmp_ps(l.v,r.v,_CMP_NEQ_OQ)};
}
inline v3 VEC operator>(const v3 l, const v3 r) {
	return {_mm_cmp_ps(l.v,r.v,_CMP_GT_OS)};
}
inline v3 VEC operator<(const v3 l, const v3 r) {
	return {_mm_cmp_ps(l.v,r.v,_CMP_LT_OS)};
}
inline v3 VEC operator>=(const v3 l, const v3 r) {
	return {_mm_cmp_ps(l.v,r.v,_CMP_GE_OS)};
}
inline v3 VEC operator<=(const v3 l, const v3 r) {
	return {_mm_cmp_ps(l.v,r.v,_CMP_LE_OS)};
}
inline bool VEC none(const v3 v) {
	return (_mm_movemask_ps(v.v) & 0b111) == 0b0;
}
inline bool VEC all(const v3 v) {
	return (_mm_movemask_ps(v.v) & 0b111) == 0b111;
}
inline bool VEC any(const v3 v) {
	return (_mm_movemask_ps(v.v) & 0b111) != 0b0;
}

//TODO(max): SIMD - _mm_pow_ps needs Intel SVML or another library for log/exp 
inline v3 VEC pow(const v3 v, const f32 r) {
	return {powf(v.x,r),powf(v.y,r),powf(v.z,r)};
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
inline v3 VEC cross(const v3 l, const v3 r) {
	__m128 ret = _mm_sub_ps(
		_mm_mul_ps(r.v, _mm_shuffle_ps(l.v, l.v, _MM_SHUFFLE(3, 0, 2, 1))), 
		_mm_mul_ps(l.v, _mm_shuffle_ps(r.v, r.v, _MM_SHUFFLE(3, 0, 2, 1))));
	return {_mm_shuffle_ps(ret, ret, _MM_SHUFFLE(3, 0, 2, 1 ))};
}

inline v3 VEC lerp(const v3 min, const v3 max, f32 dist) {
	return (max - min) * dist + min;
}

inline v3 VEC floor(const v3 v) {
	return {_mm_floor_ps(v.v)};
}
inline v3 VEC afract(const v3 v) {
	return v - floor(v);
}
inline v3 VEC abs(const v3 v) {
	return {fabsf(v.x),fabsf(v.y),fabsf(v.z)};
}

inline f32 minf(f32 l, f32 r) {
	return l < r ? l : r;
}
inline f32 maxf(f32 l, f32 r) {
	return l > r ? l : r;
}
inline f32 lerp(f32 min, f32 max, f32 dist) {
	return (max - min) * dist + min;
}
inline f32 clamp(f32 f, f32 min, f32 max) {
	return maxf(minf(f, max), min);
}
inline v3 clamp(v3 f, v3 min, v3 max) {
	return vmax(vmin(f, max), min);
}
inline f32 safe(f32 f) {
	return isnan(f) ? 0.0f : f;
}
inline v3 safe(const v3 v) {
	return (isnan(v.x) || isnan(v.y) || isnan(v.z)) ? v3{} : v;
}

inline f32 VEC trilerp(f32 vals[2][2][2], v3 uvw) {
	f32 accum = 0.0f;
	for(i32 i = 0; i < 2; i++) {
		for(i32 j = 0; j < 2; j++) {
			for(i32 k = 0; k < 2; k++) {
				accum += (i*uvw.x + (1-i)*(1-uvw.x))*
						 (j*uvw.y + (1-j)*(1-uvw.y))*
						 (k*uvw.z + (1-k)*(1-uvw.z))*
						  vals[i][j][k];
			}
		}
	}
	return accum;
}
inline f32 VEC trilerp(v3 vals[2][2][2], v3 uvw) {
	f32 accum = 0.0f;
	v3 sm = uvw * uvw * (3.0f - 2.0f * uvw); // Smoothed
	for(i32 i = 0; i < 2; i++) {
		for(i32 j = 0; j < 2; j++) {
			for(i32 k = 0; k < 2; k++) {
				v3 weight = uvw - v3(i,j,k);
				accum += (i*sm.x + (1-i)*(1-sm.x))*
						 (j*sm.y + (1-j)*(1-sm.y))*
						 (k*sm.z + (1-k)*(1-sm.z))*
						 dot(vals[i][j][k], weight);
			}
		}
	}
	return accum;
}

inline v3_lane VEC operator+(const v3_lane& l, const v3_lane& r) {
	return 
	{__add_ps(l.x, r.x),
	 __add_ps(l.y, r.y),
	 __add_ps(l.z, r.z)};
}
inline v3_lane VEC operator+(const v3_lane& l, const f32 _r) {
	__lane r = __set1_ps(_r);
	return 
	{__add_ps(l.x, r),
	 __add_ps(l.y, r),
	 __add_ps(l.z, r)};
}
inline v3_lane VEC operator+(const f32 _l, const v3_lane& r) {
	__lane l = __set1_ps(_l);
	return 
	{__add_ps(l, r.x),
	 __add_ps(l, r.y),
	 __add_ps(l, r.z)};
}
inline v3_lane VEC operator+(const v3_lane& l, const v3 r) {
	__lane x = __set1_ps(r.x);
	__lane y = __set1_ps(r.y);
	__lane z = __set1_ps(r.z);
	return 
	{__add_ps(l.x, x),
	 __add_ps(l.y, y),
	 __add_ps(l.z, z)};
}
inline v3_lane VEC operator+(const v3 l, const v3_lane& r) {
	__lane x = __set1_ps(l.x);
	__lane y = __set1_ps(l.y);
	__lane z = __set1_ps(l.z);
	return 
	{__add_ps(x, r.x),
	 __add_ps(y, r.y),
	 __add_ps(z, r.z)};
}
inline v3_lane VEC operator-(const v3_lane& l, const v3_lane& r) {
	return 
	{__sub_ps(l.x, r.x),
	 __sub_ps(l.y, r.y),
	 __sub_ps(l.z, r.z)};
}
inline v3_lane VEC operator-(const v3_lane& l, const f32 _r) {
	__lane r = __set1_ps(_r);
	return 
	{__sub_ps(l.x, r),
	 __sub_ps(l.y, r),
	 __sub_ps(l.z, r)};
}
inline v3_lane VEC operator-(const f32 _l, const v3_lane& r) {
	__lane l = __set1_ps(_l);
	return 
	{__sub_ps(l, r.x),
	 __sub_ps(l, r.y),
	 __sub_ps(l, r.z)};
}
inline v3_lane VEC operator-(const v3_lane& l, const v3 r) {
	__lane x = __set1_ps(r.x);
	__lane y = __set1_ps(r.y);
	__lane z = __set1_ps(r.z);
	return 
	{__sub_ps(l.x, x),
	 __sub_ps(l.y, y),
	 __sub_ps(l.z, z)};
}
inline v3_lane VEC operator-(const v3 l, const v3_lane& r) {
	__lane x = __set1_ps(l.x);
	__lane y = __set1_ps(l.y);
	__lane z = __set1_ps(l.z);
	return 
	{__sub_ps(x, r.x),
	 __sub_ps(y, r.y),
	 __sub_ps(z, r.z)};
}
inline v3_lane VEC operator*(const v3_lane& l, const v3_lane& r) {
	return 
	{__mul_ps(l.x, r.x),
	 __mul_ps(l.y, r.y),
	 __mul_ps(l.z, r.z)};
}
inline v3_lane VEC operator*(const v3_lane& l, const f32 _r) {
	__lane r = __set1_ps(_r);
	return 
	{__mul_ps(l.x, r),
	 __mul_ps(l.y, r),
	 __mul_ps(l.z, r)};
}
inline v3_lane VEC operator*(const v3_lane& l, const v3 r) {
	__lane x = __set1_ps(r.x);
	__lane y = __set1_ps(r.y);
	__lane z = __set1_ps(r.z);
	return 
	{__mul_ps(l.x, x),
	 __mul_ps(l.y, y),
	 __mul_ps(l.z, z)};
}
inline v3_lane VEC operator*(const v3 l, const v3_lane& r) {
	__lane x = __set1_ps(l.x);
	__lane y = __set1_ps(l.y);
	__lane z = __set1_ps(l.z);
	return 
	{__mul_ps(x, r.x),
	 __mul_ps(y, r.y),
	 __mul_ps(z, r.z)};
}
inline v3_lane VEC operator*(const f32 _l, const v3_lane& r) {
	__lane l = __set1_ps(_l);
	return 
	{__mul_ps(l, r.x),
	 __mul_ps(l, r.y),
	 __mul_ps(l, r.z)};
}
inline v3_lane VEC operator/(const v3_lane& l, const v3_lane& r) {
	return 
	{__div_ps(l.x, r.x),
	 __div_ps(l.y, r.y),
	 __div_ps(l.z, r.z)};
}
inline v3_lane VEC operator/(const v3_lane& l, const f32 _r) {
	__lane r = __set1_ps(_r);
	return 
	{__div_ps(l.x, r),
	 __div_ps(l.y, r),
	 __div_ps(l.z, r)};
}
inline v3_lane VEC operator/(const f32 _l, const v3_lane& r) {
	__lane l = __set1_ps(_l);
	return 
	{__div_ps(l, r.x),
	 __div_ps(l, r.y),
	 __div_ps(l, r.z)};
}
inline v3_lane VEC operator/(const v3_lane& l, const v3 r) {
	__lane x = __set1_ps(r.x);
	__lane y = __set1_ps(r.y);
	__lane z = __set1_ps(r.z);
	return 
	{__div_ps(l.x, x),
	 __div_ps(l.y, y),
	 __div_ps(l.z, z)};
}
inline v3_lane VEC operator/(const v3 l, const v3_lane& r) {
	__lane x = __set1_ps(l.x);
	__lane y = __set1_ps(l.y);
	__lane z = __set1_ps(l.z);
	return 
	{__div_ps(x, r.x),
	 __div_ps(y, r.y),
	 __div_ps(z, r.z)};
}

inline f32_lane VEC operator==(const v3_lane& l, const v3_lane& r) {
	__lane cmpx = __cmp_ps(l.x,r.x,_CMP_EQ_OQ);
	__lane cmpy = __cmp_ps(l.y,r.y,_CMP_EQ_OQ);
	__lane cmpz = __cmp_ps(l.z,r.z,_CMP_EQ_OQ);
	return __and_ps(__and_ps(cmpx,cmpy),cmpz);
}
inline f32_lane VEC operator!=(const v3_lane& l, const v3_lane& r) {
	__lane cmpx = __cmp_ps(l.x,r.x,_CMP_NEQ_OQ);
	__lane cmpy = __cmp_ps(l.y,r.y,_CMP_NEQ_OQ);
	__lane cmpz = __cmp_ps(l.z,r.z,_CMP_NEQ_OQ);
	return __or_ps(__or_ps(cmpx,cmpy),cmpz);
}

inline bool VEC none(const f32_lane& v) {
	return __movemask_ps(v.v) == 0x0;
}
inline bool VEC all(const f32_lane& v) {
	return __movemask_ps(v.v) == 0xf;
}
inline bool VEC any(const f32_lane& v) {
	return __movemask_ps(v.v) != 0x0;
}


inline v2_lane VEC operator+(const v2_lane& l, const v2_lane& r) {
	return 
	{__add_ps(l.x, r.x),
	 __add_ps(l.y, r.y)};
}
inline v2_lane VEC operator+(const v2_lane& l, const f32 _r) {
	__lane r = __set1_ps(_r);
	return 
	{__add_ps(l.x, r),
	 __add_ps(l.y, r)};
}
inline v2_lane VEC operator+(const f32 _l, const v2_lane& r) {
	__lane l = __set1_ps(_l);
	return 
	{__add_ps(l, r.x),
	 __add_ps(l, r.y)};
}
inline v2_lane VEC operator+(const v2_lane& l, const v2 r) {
	__lane x = __set1_ps(r.x);
	__lane y = __set1_ps(r.y);
	return 
	{__add_ps(l.x, x),
	 __add_ps(l.y, y)};
}
inline v2_lane VEC operator+(const v2 l, const v2_lane& r) {
	__lane x = __set1_ps(l.x);
	__lane y = __set1_ps(l.y);
	return 
	{__add_ps(x, r.x),
	 __add_ps(y, r.y)};
}
inline v2_lane VEC operator-(const v2_lane& l, const v2_lane& r) {
	return 
	{__sub_ps(l.x, r.x),
	 __sub_ps(l.y, r.y)};
}
inline v2_lane VEC operator-(const v2_lane& l, const f32 _r) {
	__lane r = __set1_ps(_r);
	return 
	{__sub_ps(l.x, r),
	 __sub_ps(l.y, r)};
}
inline v2_lane VEC operator-(const f32 _l, const v2_lane& r) {
	__lane l = __set1_ps(_l);
	return 
	{__sub_ps(l, r.x),
	 __sub_ps(l, r.y)};
}
inline v2_lane VEC operator-(const v2_lane& l, const v2 r) {
	__lane x = __set1_ps(r.x);
	__lane y = __set1_ps(r.y);
	return 
	{__sub_ps(l.x, x),
	 __sub_ps(l.y, y)};
}
inline v2_lane VEC operator-(const v2 l, const v2_lane& r) {
	__lane x = __set1_ps(l.x);
	__lane y = __set1_ps(l.y);
	return 
	{__sub_ps(x, r.x),
	 __sub_ps(y, r.y)};
}
inline v2_lane VEC operator*(const v2_lane& l, const v2_lane& r) {
	return 
	{__mul_ps(l.x, r.x),
	 __mul_ps(l.y, r.y)};
}
inline v2_lane VEC operator*(const v2_lane& l, const f32 _r) {
	__lane r = __set1_ps(_r);
	return 
	{__mul_ps(l.x, r),
	 __mul_ps(l.y, r)};
}
inline v2_lane VEC operator*(const f32 _l, const v2_lane& r) {
	__lane l = __set1_ps(_l);
	return 
	{__mul_ps(l, r.x),
	 __mul_ps(l, r.y)};
}
inline v2_lane VEC operator*(const v2_lane& l, const v2 r) {
	__lane x = __set1_ps(r.x);
	__lane y = __set1_ps(r.y);
	return 
	{__mul_ps(l.x, x),
	 __mul_ps(l.y, y)};
}
inline v2_lane VEC operator*(const v2 l, const v2_lane& r) {
	__lane x = __set1_ps(l.x);
	__lane y = __set1_ps(l.y);
	return 
	{__mul_ps(x, r.x),
	 __mul_ps(y, r.y)};
}
inline v2_lane VEC operator/(const v2_lane& l, const v2_lane& r) {
	return 
	{__div_ps(l.x, r.x),
	 __div_ps(l.y, r.y)};
}
inline v2_lane VEC operator/(const v2_lane& l, const f32 _r) {
	__lane r = __set1_ps(_r);
	return 
	{__div_ps(l.x, r),
	 __div_ps(l.y, r)};
}
inline v2_lane VEC operator/(const f32 _l, const v2_lane& r) {
	__lane l = __set1_ps(_l);
	return 
	{__div_ps(l, r.x),
	 __div_ps(l, r.y)};
}
inline v2_lane VEC operator/(const v2_lane& l, const v2 r) {
	__lane x = __set1_ps(r.x);
	__lane y = __set1_ps(r.y);
	return 
	{__div_ps(l.x, x),
	 __div_ps(l.y, y)};
}
inline v2_lane VEC operator/(const v2 l, const v2_lane& r) {
	__lane x = __set1_ps(l.x);
	__lane y = __set1_ps(l.y);
	return 
	{__div_ps(x, r.x),
	 __div_ps(y, r.y)};
}

inline f32_lane select(const f32_lane& l, const f32_lane& r, const f32_lane& m) {
	return {__blendv_ps(r.v,l.v,m.v)};
}

inline f32_lane VEC operator&(const f32_lane& l, const f32_lane& r) {
	return {__and_ps(l.v, r.v)};
}
inline f32_lane VEC operator|(const f32_lane& l, const f32_lane& r) {
	return {__or_ps(l.v, r.v)};
}

inline v3_lane VEC operator&(const v3_lane& l, const f32_lane& r) {
	return {__and_ps(l.x, r.v),
			__and_ps(l.y, r.v),
			__and_ps(l.z, r.v)};
}
inline v3_lane VEC operator|(const v3_lane& l, const f32_lane& r) {
	return {__or_ps(l.x, r.v),
			__or_ps(l.y, r.v),
			__or_ps(l.z, r.v)};
}
inline v3_lane VEC operator&(const f32_lane& l, const v3_lane& r) {
	return {__and_ps(l.v, r.x),
			__and_ps(l.v, r.y),
			__and_ps(l.v, r.z)};
}
inline v3_lane VEC operator|(const f32_lane& l, const v3_lane& r) {
	return {__or_ps(l.v, r.x),
			__or_ps(l.v, r.y),
			__or_ps(l.v, r.z)};
}

inline f32_lane VEC operator+(const f32_lane& l, const f32_lane& r) {
	return {__add_ps(l.v, r.v)};
}
inline f32_lane VEC operator+(const f32_lane& l, const f32 _r) {
	__lane r = __set1_ps(_r);
	return {__add_ps(l.v, r)};
}
inline f32_lane VEC operator+(const f32 _l, const f32_lane& r) {
	__lane l = __set1_ps(_l);
	return {__add_ps(l, r.v)};
}
inline f32_lane VEC operator-(const f32_lane& l, const f32_lane& r) {
	return {__sub_ps(l.v, r.v)};
}
inline f32_lane VEC operator-(const f32_lane& l, const f32 _r) {
	__lane r = __set1_ps(_r);
	return {__sub_ps(l.v, r)};
}
inline f32_lane VEC operator-(const f32 _l, const f32_lane& r) {
	__lane l = __set1_ps(_l);
	return {__sub_ps(l, r.v)};
}
inline f32_lane VEC operator*(const f32_lane& l, const f32_lane& r) {
	return {__mul_ps(l.v, r.v)};
}
inline f32_lane VEC operator*(const f32_lane& l, const f32 _r) {
	__lane r = __set1_ps(_r);
	return {__mul_ps(l.v, r)};
}
inline f32_lane VEC operator*(const f32 _l, const f32_lane& r) {
	__lane l = __set1_ps(_l);
	return {__mul_ps(l, r.v)};
}
inline f32_lane VEC operator/(const f32_lane& l, const f32_lane& r) {
	return {__div_ps(l.v, r.v)};
}
inline f32_lane VEC operator/(const f32_lane& l, const f32 _r) {
	__lane r = __set1_ps(_r);
	return {__div_ps(l.v, r)};
}
inline f32_lane VEC operator/(const f32 _l, const f32_lane& r) {
	__lane l = __set1_ps(_l);
	return {__div_ps(l, r.v)};
}

inline f32_lane VEC operator==(const f32_lane& l, const f32_lane& r) {
	return __cmp_ps(l.v,r.v,_CMP_EQ_OQ);
}
inline f32_lane VEC operator!=(const f32_lane& l, const f32_lane& r) {
	return __cmp_ps(l.v,r.v,_CMP_NEQ_OQ);
}
inline f32_lane VEC operator>(const f32_lane& l, const f32_lane& r) {
	return __cmp_ps(l.v,r.v,_CMP_GT_OS);
}
inline f32_lane VEC operator<(const f32_lane& l, const f32_lane& r) {
	return __cmp_ps(l.v,r.v,_CMP_LT_OS);
}
inline f32_lane VEC operator>=(const f32_lane& l, const f32_lane& r) {
	return __cmp_ps(l.v,r.v,_CMP_GE_OS);
}
inline f32_lane VEC operator<=(const f32_lane& l, const f32_lane& r) {
	return __cmp_ps(l.v,r.v,_CMP_LE_OS);
}
inline f32_lane VEC operator==(const f32_lane& l, f32 r) {
	return __cmp_ps(l.v,__set1_ps(r),_CMP_EQ_OQ);
}
inline f32_lane VEC operator!=(const f32_lane& l, f32 r) {
	return __cmp_ps(l.v,__set1_ps(r),_CMP_NEQ_OQ);
}
inline f32_lane VEC operator>(const f32_lane& l, f32 r) {
	return __cmp_ps(l.v,__set1_ps(r),_CMP_GT_OS);
}
inline f32_lane VEC operator<(const f32_lane& l, f32 r) {
	return __cmp_ps(l.v,__set1_ps(r),_CMP_LT_OS);
}
inline f32_lane VEC operator>=(const f32_lane& l, f32 r) {
	return __cmp_ps(l.v,__set1_ps(r),_CMP_GE_OS);
}
inline f32_lane VEC operator<=(const f32_lane& l, f32 r) {
	return __cmp_ps(l.v,__set1_ps(r),_CMP_LE_OS);
}
inline f32_lane VEC operator==(f32 l, const f32_lane& r) {
	return __cmp_ps(__set1_ps(l),r.v,_CMP_EQ_OQ);
}
inline f32_lane VEC operator!=(f32 l, const f32_lane& r) {
	return __cmp_ps(__set1_ps(l),r.v,_CMP_NEQ_OQ);
}
inline f32_lane VEC operator>(f32 l, const f32_lane& r) {
	return __cmp_ps(__set1_ps(l),r.v,_CMP_GT_OS);
}
inline f32_lane VEC operator<(f32 l, const f32_lane& r) {
	return __cmp_ps(__set1_ps(l),r.v,_CMP_LT_OS);
}
inline f32_lane VEC operator>=(f32 l, const f32_lane& r) {
	return __cmp_ps(__set1_ps(l),r.v,_CMP_GE_OS);
}
inline f32_lane VEC operator<=(f32 l, const f32_lane& r) {
	return __cmp_ps(__set1_ps(l),r.v,_CMP_LE_OS);
}

inline f32_lane VEC sqrt(const f32_lane& l) {
	return {__sqrt_ps(l.v)};
}

inline f32_lane VEC vmin(const f32_lane& l, const f32_lane& r) {
	return {__min_ps(l.v,r.v)};
}
inline f32_lane VEC vmax(const f32_lane& l, const f32_lane& r) {
	return {__max_ps(l.v,r.v)};
}

inline f32 VEC hsum(const f32_lane& l) {
	__lane v = l.v;

	v = __hadd_ps(v,v);
	v = __hadd_ps(v,v);

	f32_lane f{v};
#if LANE_WIDTH==8
	return f.f[0] + f.f[4];
#else
	return f.f[0];
#endif
}

inline f32 VEC hmin(const f32_lane& l) {
	__lane v = l.v;

#if LANE_WIDTH==8
	v = __min_ps(v, _mm256_permute2f128_ps(v, v, 1));
#endif
    v = __min_ps(v, __shuffle_ps(v, v, _MM_SHUFFLE(0,0,2,3)));
    v = __min_ps(v, __shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,1)));

	return f32_lane{v}.f[0];
}

inline f32 VEC hmax(const f32_lane& l) {
	__lane v = l.v;

#if LANE_WIDTH==8
	v = _mm256_max_ps(v, _mm256_permute2f128_ps(v, v, 1));
#endif
    v = __max_ps(v, __shuffle_ps(v, v, _MM_SHUFFLE(0,0,2,3)));
    v = __max_ps(v, __shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,1)));

	return f32_lane{v}.f[0];
}

inline f32 VEC hmin(const v3 l) {
	__m128 v = l.v;
    v = _mm_min_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,2,3)));
    v = _mm_min_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,1)));
	return v3{v}.x;
}

inline f32 VEC hmax(const v3 l) {
	__m128 v = l.v;
    v = _mm_max_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,2,3)));
    v = _mm_max_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,1)));
	return v3{v}.x;
}

inline v3 VEC hmin(const v3_lane& l) {
	return {hmin(l.x),hmin(l.y),hmin(l.z)};
}
inline v3 VEC hmax(const v3_lane& l) {
	return {hmax(l.x),hmax(l.y),hmax(l.z)};
}

inline v3_lane VEC operator*(const v3 l, const f32_lane& r) {
	return v3_lane{l.x * r, l.y * r, l.z * r};
}

inline f32_lane VEC dot(const v3_lane& l, const v3_lane& r) {
	__lane x = __mul_ps(l.x,r.x);
	__lane y = __mul_ps(l.y,r.y);
	__lane z = __mul_ps(l.z,r.z);
	return __add_ps(__add_ps(x,y),z);
}

inline v3_lane VEC reflect(const v3_lane& v, const v3_lane& n) {
	return v - f32_lane{2.0f} * dot(n, v) * n;
}
inline f32_lane VEC lensq(const v3_lane& v) {
	return dot(v, v);
}
inline f32_lane VEC len(const v3_lane& v) {
	return __sqrt_ps(lensq(v).v);
}
inline v3_lane VEC norm(const v3_lane& v) {
	return v / len(v);
}
inline v3_lane VEC lerp(const v3_lane& min, const v3_lane& max, const f32_lane& dist) {
	return (max - min) * dist + min;
}

inline v3_lane VEC cross(const v3_lane& l, const v3_lane& r) {
	return {f32_lane{__mul_ps(l.y,r.z)} -
			f32_lane{__mul_ps(l.z,r.y)}, 
			f32_lane{__mul_ps(l.z,r.x)} - 
			f32_lane{__mul_ps(l.x,r.z)}, 
			f32_lane{__mul_ps(l.x,r.y)} - 
			f32_lane{__mul_ps(l.y,r.x)}};
}

inline v3 ray::get(f32 d) const {
	return pos + d * dir;
}

inline v3_lane ray_lane::get(const f32_lane& t) const {
	return pos + t * dir;
}

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
inline v3 VEC randomvec() {
	return {2.0f * randomf() - 1.0f, 2.0f * randomf() - 1.0f, 2.0f * randomf() - 1.0f};
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

inline f32_lane randomf_lane() {
	f32_lane ret;
	for(i32 i = 0; i < LANE_WIDTH; i++) {
		f32 r = randomf();
		ret.f[i] = r;
	}
	return ret;
}
inline v3_lane random_leunit_lane() {
	v3_lane ret;
	for(i32 i = 0; i < LANE_WIDTH; i++) {
		const v3 r = random_leunit();
		ret.xf[i] = r.x;
		ret.yf[i] = r.y;
		ret.zf[i] = r.z;
	}
	return ret;
}
inline v3_lane random_ledisk_lane() {
	v3_lane ret;
	for(i32 i = 0; i < LANE_WIDTH; i++) {
		const v3 r = random_ledisk();
		ret.xf[i] = r.x;
		ret.yf[i] = r.y;
		ret.zf[i] = r.z;
	}
	return ret;	
}

struct perlin {

	v3 vecs[256] = {};
	i32 x_perm[256] = {}, y_perm[256] = {}, z_perm[256] = {};

	// NOTE(max): needs to be initialized after the rand state
	void init() {
		for(i32 i = 0; i < 256; i++) {
			vecs[i] = norm(randomvec());
			x_perm[i] = y_perm[i] = z_perm[i] = i;
		}
		for(i32 i = 255; i > 0; i--) {
			i32 locx = i32(randomf() * (i + 1));
			i32 locy = i32(randomf() * (i + 1));
			i32 locz = i32(randomf() * (i + 1));
			
			i32 tmp = x_perm[i];
			x_perm[i] = x_perm[locx];
			x_perm[locx] = tmp;

			tmp = y_perm[i];
			y_perm[i] = y_perm[locy];
			y_perm[locx] = tmp;

			tmp = z_perm[i];
			z_perm[i] = z_perm[locz];
			z_perm[locz] = tmp;
		}
	}

	f32 trilerp(v3 pos) const {

		v3 uvw = afract(pos);
		i32 i = (i32)floor(pos.x);
		i32 j = (i32)floor(pos.y);
		i32 k = (i32)floor(pos.z);
		v3 vals[2][2][2];
		for(i32 di = 0; di < 2; di++) {
			for(i32 dj = 0; dj < 2; dj++) {
				for(i32 dk = 0; dk < 2; dk++) {
					vals[di][dj][dk] = vecs[x_perm[(i+di) & 0xff] ^ 
											y_perm[(j+dj) & 0xff] ^ 
											z_perm[(k+dk) & 0xff]];
				}
			}
		}
		return ::trilerp(vals, uvw);
	}

	f32 turb(v3 pos, i32 depth) const {

		f32 accum = 0.0f, weight = 1.0f;
		for(i32 i = 0; i < depth; i++) {
			accum += weight * trilerp(pos);
			weight *= 0.5f;
			pos *= 2.0f;
		}
		return accum;
	}
};
extern perlin g_perlin;

inline v4 VEC operator+(const v4 l, const v4 r) {
	return {_mm_add_ps(l.v, r.v)};
}
inline v4 VEC operator-(const v4 l, const v4 r) {
	return {_mm_sub_ps(l.v, r.v)};
}
inline v4 VEC operator*(const v4 l, const v4 r) {
	return {_mm_mul_ps(l.v, r.v)};
}
inline v4 VEC operator*(const v4 l, f32 r) {
	return {_mm_mul_ps(l.v, _mm_set1_ps(r))};
}
inline v4 VEC operator*(f32 l, const v4 r) {
	return {_mm_mul_ps(_mm_set1_ps(l), r.v)};
}
inline v4 VEC operator/(const v4 l, const v4 r) {
	return {_mm_div_ps(l.v, r.v)};
}
inline v4 VEC operator/(const v4 l, f32 r) {
	return {_mm_div_ps(l.v, _mm_set1_ps(r))};
}
inline v4 VEC operator/(f32 l, const v4 r) {
	return {_mm_div_ps(_mm_set1_ps(l), r.v)};
}
inline f32 VEC dot(const v4 l, const v4 r) {
	return v4{_mm_dp_ps(l.v, r.v, 0xf1)}.x;
}
inline f32 VEC lensq(const v4 v) {
	return dot(v, v);
}
inline f32 VEC len(const v4 v) {
	return sqrtf(lensq(v));
}
inline v4 VEC norm(const v4 v) {
	return v / len(v);
}
inline bool VEC operator==(const v4 l, const v4 r) {
	return _mm_movemask_ps(_mm_cmpeq_ps(l.v, r.v)) == 0xf;
}
inline bool VEC operator!=(const v4 l, const v4 r) {
	return _mm_movemask_ps(_mm_cmpeq_ps(l.v, r.v)) != 0xf;
}

inline bool VEC operator==(const m4 l, const m4 r) {
	return l.cols[0] == r.cols[0] &&
		   l.cols[1] == r.cols[1] &&
		   l.cols[2] == r.cols[2] &&
		   l.cols[3] == r.cols[3];
}
inline bool VEC operator!=(const m4 l, const m4 r) {
	return l.cols[0] != r.cols[0] ||
		   l.cols[1] != r.cols[1] ||
		   l.cols[2] != r.cols[2] ||
		   l.cols[3] != r.cols[3];
}

inline m4 VEC transpose(m4 m) {
	m4 ret;
	for(i32 i = 0; i < 4; i++)
		for(u8 j = 0; j < 4; j++)
			ret[i][j] = m[j][i];
	return ret;
}
inline m4 VEC operator+(m4 l, m4 r) {
	m4 ret;
	ret.v[0] = _mm_add_ps(l.v[0], r.v[0]);
	ret.v[1] = _mm_add_ps(l.v[1], r.v[1]);
	ret.v[2] = _mm_add_ps(l.v[2], r.v[2]);
	ret.v[3] = _mm_add_ps(l.v[3], r.v[3]);
	return ret;
}
inline m4 VEC operator-(m4 l, m4 r) {
	m4 ret;
	ret.v[0] = _mm_sub_ps(l.v[0], r.v[0]);
	ret.v[1] = _mm_sub_ps(l.v[1], r.v[1]);
	ret.v[2] = _mm_sub_ps(l.v[2], r.v[2]);
	ret.v[3] = _mm_sub_ps(l.v[3], r.v[3]);
	return ret;
}
inline m4 VEC operator*(m4 l, m4 r) {
	m4 ret;
    for(i32 i = 0; i < 4; i++) {
        ret.v[i] = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(r[i][0]), l.v[0]),
            _mm_mul_ps(_mm_set1_ps(r[i][1]), l.v[1])), 
       	_mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(r[i][2]), l.v[2]),
            _mm_mul_ps(_mm_set1_ps(r[i][3]), l.v[3])));
    }
    return ret;
}
inline v4 operator*(m4 l, v4 r) {
    return (r.x * l.cols[0] + r.y * l.cols[1]) +
    	   (r.z * l.cols[2] + r.w * l.cols[3]);
}
inline m4 VEC operator*(m4 l, f32 r) {
	m4 ret;
	__m128 mul = _mm_set1_ps(r);
	ret.v[0] = _mm_mul_ps(l.v[0], mul);
	ret.v[1] = _mm_mul_ps(l.v[1], mul);
	ret.v[2] = _mm_mul_ps(l.v[2], mul);
	ret.v[3] = _mm_mul_ps(l.v[3], mul);
	return ret;
}
inline m4 VEC operator/(m4 l, f32 r) {
	m4 ret;
	__m128 mul = _mm_set1_ps(r);
	ret.v[0] = _mm_div_ps(l.v[0], mul);
	ret.v[1] = _mm_div_ps(l.v[1], mul);
	ret.v[2] = _mm_div_ps(l.v[2], mul);
	ret.v[3] = _mm_div_ps(l.v[3], mul);
	return ret;
}
inline m4 VEC ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
    m4 ret;
    ret[0][0] = 2.0f / (r - l);
    ret[1][1] = 2.0f / (t - b);
    ret[2][2] = 2.0f / (n - f);
    ret[3][0] = (-l - r) / (r - l);
    ret[3][1] = (-b - t)  / (t - b);
    ret[3][2] = - n / (f - n);
    return ret;
}
inline m4 VEC project(f32 fov, f32 ar, f32 n)
{
    float f = 1.0f / tanf(RADIANS(fov) / 2.0f);
    m4 ret;
    ret[0][0] = f / ar;
    ret[1][1] = f;
    ret[2][2] = 0.0f;
    ret[3][3] = 0.0f;
    ret[3][2] = n;
    ret[2][3] = -1.0f;
    return ret;
}
inline m4 VEC translate(v3 v) {
	m4 ret;
	ret[3][0] = v[0];
	ret[3][1] = v[1];
	ret[3][2] = v[2];
    return ret;
}
inline m4 VEC rotate(f32 a, v3 axis) {
	m4 ret;
	f32 c = cosf(RADIANS(a));
	f32 s = sinf(RADIANS(a));
	
	axis = norm(axis);
	v3 temp = axis * (1.0f - c);

	ret[0][0] = c + temp[0] * axis[0];
	ret[0][1] = temp[0] * axis[1] + s * axis[2];
	ret[0][2] = temp[0] * axis[2] - s * axis[1];
	ret[1][0] = temp[1] * axis[0] - s * axis[2];
	ret[1][1] = c + temp[1] * axis[1];
	ret[1][2] = temp[1] * axis[2] + s * axis[0];
	ret[2][0] = temp[2] * axis[0] + s * axis[1];
	ret[2][1] = temp[2] * axis[1] - s * axis[0];
	ret[2][2] = c + temp[2] * axis[2];

	return ret;
}
inline m4 VEC scale(v3 s) {
	m4 ret;
    ret[0][0] = s.x;
    ret[1][1] = s.y;
    ret[2][2] = s.z;
    return ret;
}
inline m4 VEC look_at(v3 pos, v3 at, v3 up) {
    m4 ret = m4::zero;

    v3 F = norm(at - pos);
    v3 S = norm(cross(F, up));
    v3 U = cross(S, F);

    ret[0][0] =  S.x;
    ret[0][1] =  U.x;
    ret[0][2] = -F.x;
    ret[1][0] =  S.y;
    ret[1][1] =  U.y;
    ret[1][2] = -F.y;
    ret[2][0] =  S.z;
    ret[2][1] =  U.z;
    ret[2][2] = -F.z;
    ret[3][0] = -dot(S, pos);
    ret[3][1] = -dot(U, pos);
    ret[3][2] =  dot(F, pos);
    ret[3][3] = 1.0f;

    return ret;
}

// SSE matrix inverse from https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html

#define MakeShuffleMask(x,y,z,w)           (x | (y<<2) | (z<<4) | (w<<6))
#define VecSwizzleMask(vec, mask)          _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(vec), mask))
#define VecSwizzle(vec, x, y, z, w)        VecSwizzleMask(vec, MakeShuffleMask(x,y,z,w))
#define VecSwizzle1(vec, x)                VecSwizzleMask(vec, MakeShuffleMask(x,x,x,x))
#define VecSwizzle_0022(vec)               _mm_moveldup_ps(vec)
#define VecSwizzle_1133(vec)               _mm_movehdup_ps(vec)
#define VecShuffle(vec1, vec2, x,y,z,w)    _mm_shuffle_ps(vec1, vec2, MakeShuffleMask(x,y,z,w))
#define VecShuffle_0101(vec1, vec2)        _mm_movelh_ps(vec1, vec2)
#define VecShuffle_2323(vec1, vec2)        _mm_movehl_ps(vec2, vec1)
inline __m128 VEC Mat2Mul(__m128 vec1, __m128 vec2)
{
	return
		_mm_add_ps(_mm_mul_ps(                     vec1, VecSwizzle(vec2, 0,3,0,3)),
		           _mm_mul_ps(VecSwizzle(vec1, 1,0,3,2), VecSwizzle(vec2, 2,1,2,1)));
}
inline __m128 VEC Mat2AdjMul(__m128 vec1, __m128 vec2)
{
	return
		_mm_sub_ps(_mm_mul_ps(VecSwizzle(vec1, 3,3,0,0), vec2),
		           _mm_mul_ps(VecSwizzle(vec1, 1,1,2,2), VecSwizzle(vec2, 2,3,0,1)));

}
inline __m128 VEC Mat2MulAdj(__m128 vec1, __m128 vec2)
{
	return
		_mm_sub_ps(_mm_mul_ps(                     vec1, VecSwizzle(vec2, 3,0,3,0)),
		           _mm_mul_ps(VecSwizzle(vec1, 1,0,3,2), VecSwizzle(vec2, 2,1,2,1)));
}
inline m4 VEC inverse(m4 inM)
{
	__m128 A = VecShuffle_0101(inM.v[0], inM.v[1]);
	__m128 B = VecShuffle_2323(inM.v[0], inM.v[1]);
	__m128 C = VecShuffle_0101(inM.v[2], inM.v[3]);
	__m128 D = VecShuffle_2323(inM.v[2], inM.v[3]);

	__m128 detSub = _mm_sub_ps(
		_mm_mul_ps(VecShuffle(inM.v[0], inM.v[2], 0,2,0,2), VecShuffle(inM.v[1], inM.v[3], 1,3,1,3)),
		_mm_mul_ps(VecShuffle(inM.v[0], inM.v[2], 1,3,1,3), VecShuffle(inM.v[1], inM.v[3], 0,2,0,2))
	);
	__m128 detA = VecSwizzle1(detSub, 0);
	__m128 detB = VecSwizzle1(detSub, 1);
	__m128 detC = VecSwizzle1(detSub, 2);
	__m128 detD = VecSwizzle1(detSub, 3);
	__m128 D_C = Mat2AdjMul(D, C);
	__m128 A_B = Mat2AdjMul(A, B);
	__m128 X_ = _mm_sub_ps(_mm_mul_ps(detD, A), Mat2Mul(B, D_C));
	__m128 W_ = _mm_sub_ps(_mm_mul_ps(detA, D), Mat2Mul(C, A_B));

	__m128 detM = _mm_mul_ps(detA, detD);
	__m128 Y_ = _mm_sub_ps(_mm_mul_ps(detB, C), Mat2MulAdj(D, A_B));
	__m128 Z_ = _mm_sub_ps(_mm_mul_ps(detC, B), Mat2MulAdj(A, D_C));
	detM = _mm_add_ps(detM, _mm_mul_ps(detB, detC));

	__m128 tr = _mm_mul_ps(A_B, VecSwizzle(D_C, 0,2,1,3));
	tr = _mm_hadd_ps(tr, tr);
	tr = _mm_hadd_ps(tr, tr);
	detM = _mm_sub_ps(detM, tr);

	const __m128 adjSignMask = _mm_setr_ps(1.f, -1.f, -1.f, 1.f);
	__m128 rDetM = _mm_div_ps(adjSignMask, detM);

	X_ = _mm_mul_ps(X_, rDetM);
	Y_ = _mm_mul_ps(Y_, rDetM);
	Z_ = _mm_mul_ps(Z_, rDetM);
	W_ = _mm_mul_ps(W_, rDetM);

	m4 r;
	r.v[0] = VecShuffle(X_, Y_, 3,1,3,1);
	r.v[1] = VecShuffle(X_, Y_, 2,0,2,0);
	r.v[2] = VecShuffle(Z_, W_, 3,1,3,1);
	r.v[3] = VecShuffle(Z_, W_, 2,0,2,0);
	return r;
}

#ifdef MATH_IMPLEMENTATION

perlin g_perlin;
rand_state __state;

m4 m4::zero = {{0.0f, 0.0f, 0.0f, 0.0f},
			   {0.0f, 0.0f, 0.0f, 0.0f},
			   {0.0f, 0.0f, 0.0f, 0.0f},
			   {0.0f, 0.0f, 0.0f, 0.0f}};
m4 m4::I = {{1.0f, 0.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 0.0f, 1.0f}};

std::ostream& operator<<(std::ostream& out, v4 r) {
	out << "{" << r.x << "," << r.y << "," << r.z << "," << r.w << "}";
	return out;
}

std::ostream& operator<<(std::ostream& out, m4 r) {
	out << "{";
	for(i32 i = 0; i < 4; i++) {
		out << r.cols[i] << "," << std::endl;
	}
	out << "}";
	return out;
}

std::ostream& operator<<(std::ostream& out, const v3 r) {
	out << "{" << r.x << "," << r.y << "," << r.z << "}";
	return out;
}

std::ostream& VEC operator<<(std::ostream& out, const v3_lane& r) {
	out << "{";
	for(i32 i = 0; i < LANE_WIDTH; i++) {
		out << "{" << r.xf[i] << "," << r.yf[i] << "," << r.zf[i] << "}";
		if(i != LANE_WIDTH - 1) out << ",";
	}
	out << "}";
	return out;
}
std::ostream& VEC operator<<(std::ostream& out, const f32_lane& r) {
	out << "{";
	for(i32 i = 0; i < LANE_WIDTH; i++) {
		out << r.f[i];
		if(i != LANE_WIDTH - 1) out << ",";
	}
	out << "}";
	return out;
}

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
	{
		f32_lane _0(1.0f);
		f32_lane _1(3.0f);

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

		f32_lane _2;
		_2.f[0] = -2.0f;
		_2.f[1] = -1.0f;
		_2.f[2] = 0.0f;
		_2.f[3] = 1.0f;
#if LANE_WIDTH==8		
		_2.f[4] = 1.0f;
		_2.f[5] = 2.0f;
		_2.f[6] = 3.0f;
		_2.f[7] = 4.0f;
#endif

		std::cout << _2 << std::endl;
		std::cout << "hmin: " << hmin(_2) << std::endl;
		std::cout << "hsum: " << hsum(_2) << std::endl;
	}

	{
		v3_lane _0(0.0f,1.0f,2.0f);
		v3_lane _1(3.0f,4.0f,5.0f);

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
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif
