
#pragma once

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
#include <immintrin.h>
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
#include <xmmintrin.h>
#else
#error "LANE_WIDTH not 4 or 8"
#endif

#include "basic.h"

#include <math.h>
#include <random>

#ifdef _MSC_VER
#pragma warning(disable : 4201)
#endif

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

// NOTE(max): mask 1: left 0: right
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

inline f32_lane VEC min(const f32_lane& l, const f32_lane& r) {
	return {__min_ps(l.v,r.v)};
}
inline f32_lane VEC max(const f32_lane& l, const f32_lane& r) {
	return {__max_ps(l.v,r.v)};
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

struct ray_lane {
	v3_lane pos, dir;
	v3_lane get(const f32_lane& t) const {return pos + t * dir;}
};

inline f32_lane randf_lane() {
	f32_lane ret;
	for(i32 i = 0; i < LANE_WIDTH; i++) {
		f32 r = randf_cpp();
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

void test_simd() {

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
