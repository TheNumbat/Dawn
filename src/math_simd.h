
#pragma once

#define LANE_WIDTH 8

#include "basic.h"

#include <math.h>
#include <immintrin.h>
#include <random>

#ifdef _MSC_VER
#pragma warning(disable : 4201)
#endif

union f32_lane {
	__m256 v;
	i32 i[LANE_WIDTH];
	f32 f[LANE_WIDTH] = {};

	void operator+=(f32_lane x) {v = _mm256_add_ps(v, x.v);}
	void operator-=(f32_lane x) {v = _mm256_sub_ps(v, x.v);}
	void operator*=(f32_lane x) {v = _mm256_mul_ps(v, x.v);}
	void operator/=(f32_lane x) {v = _mm256_div_ps(v, x.v);}

	void operator*=(f32 _s) {__m256 s = _mm256_set1_ps(_s);
							 v = _mm256_mul_ps(v, s);}
	void operator/=(f32 _s) {__m256 s = _mm256_set1_ps(_s);
							 v = _mm256_div_ps(v, s);}

	void operator|=(f32_lane x) {v = _mm256_or_ps(v,x.v);}
	void operator&=(f32_lane x) {v = _mm256_and_ps(v,x.v);}
	f32_lane operator~() {return {_mm256_xor_ps(v,_mm256_castsi256_ps(_mm256_set1_epi32(0xffffffff)))};}

	f32_lane operator-() {__m256 _z = _mm256_setzero_ps();
						  return {_mm256_sub_ps(_z,v)};}

	void set_all_int(i32 _i) {v = _mm256_castsi256_ps(_mm256_set1_epi32(_i));}

	f32_lane() {}
	f32_lane(f32 _v) {v = _mm256_set1_ps(_v);}
	f32_lane(i32 _v) {v = _mm256_castsi256_ps(_mm256_set1_epi32(_v));}
	f32_lane(__m256 _v) {v = _v;}

	f32_lane(const f32_lane& o) {memcpy(this,&o,sizeof(f32_lane));}
	f32_lane(const f32_lane&& o) {memcpy(this,&o,sizeof(f32_lane));}
	f32_lane operator=(const f32_lane& o) {memcpy(this,&o,sizeof(f32_lane)); return *this;}
	f32_lane operator=(const f32_lane&& o) {memcpy(this,&o,sizeof(f32_lane)); return *this;}
};
static_assert(sizeof(f32_lane) == 4 * 8, "sizeof(f32_lane) != 32");

union v3_lane {
	struct {
		__m256 x, y, z;
	};
	struct {
		f32 xf[LANE_WIDTH], yf[LANE_WIDTH], zf[LANE_WIDTH];
	};
	f32 af[3 * LANE_WIDTH];
	f32_lane v[3];
	__m256 a[3] = {};

	void operator+=(v3_lane o) {x = _mm256_add_ps(x, o.x);
						   		y = _mm256_add_ps(y, o.y);
						   		z = _mm256_add_ps(z, o.z);}
	void operator-=(v3_lane o) {x = _mm256_sub_ps(x, o.x);
						   		y = _mm256_sub_ps(y, o.y);
						   		z = _mm256_sub_ps(z, o.z);}
	void operator*=(v3_lane o) {x = _mm256_mul_ps(x, o.x);
						   		y = _mm256_mul_ps(y, o.y);
						   		z = _mm256_mul_ps(z, o.z);}
	void operator/=(v3_lane o) {x = _mm256_div_ps(x, o.x);
						   		y = _mm256_div_ps(y, o.y);
						   		z = _mm256_div_ps(z, o.z);}
	void operator*=(f32_lane s) {
							x = _mm256_mul_ps(x, s.v);
						    y = _mm256_mul_ps(y, s.v);
						    z = _mm256_mul_ps(z, s.v);}
	void operator/=(f32_lane s) {
							x = _mm256_div_ps(x, s.v);
						    y = _mm256_div_ps(y, s.v);
						    z = _mm256_div_ps(z, s.v);}
	void operator*=(f32 s) {__m256 _s = _mm256_set1_ps(s);
							x = _mm256_mul_ps(x, _s);
						    y = _mm256_mul_ps(y, _s);
						    z = _mm256_mul_ps(z, _s);}
	void operator/=(f32 s) {__m256 _s = _mm256_set1_ps(s);
							x = _mm256_div_ps(x, _s);
						    y = _mm256_div_ps(y, _s);
						    z = _mm256_div_ps(z, _s);}

	void operator|=(f32_lane o) {x = _mm256_or_ps(x,o.v);
								 y = _mm256_or_ps(y,o.v);
								 z = _mm256_or_ps(z,o.v);}
	void operator&=(f32_lane o) {x = _mm256_and_ps(x,o.v);
								 y = _mm256_and_ps(y,o.v);
								 z = _mm256_and_ps(z,o.v);}
	void operator|=(v3_lane o) {x = _mm256_or_ps(x,o.x);
								y = _mm256_or_ps(y,o.y);
								z = _mm256_or_ps(z,o.z);}
	void operator&=(v3_lane o) {x = _mm256_and_ps(x,o.x);
								y = _mm256_and_ps(y,o.y);
								z = _mm256_and_ps(z,o.z);}

	v3_lane operator-() {__m256 _z = _mm256_setzero_ps();
					return {_mm256_sub_ps(_z,x),
							_mm256_sub_ps(_z,y),
							_mm256_sub_ps(_z,z)};}

	v3_lane() {}
	v3_lane(f32 _x) {x = y = z = _mm256_set1_ps(_x);}
	v3_lane(f32_lane _x) {x = y = z = _x.v;}
	v3_lane(f32 _x, f32 _y, f32 _z) {x = _mm256_set1_ps(_x); 
									 y = _mm256_set1_ps(_y); 
									 z = _mm256_set1_ps(_z);}
	v3_lane(f32_lane _x, f32_lane _y, f32_lane _z) {
									 x = _x.v; 
									 y = _y.v; 
									 z = _z.v;}

	v3_lane(const v3_lane& o) {memcpy(this,&o,sizeof(v3_lane));}
	v3_lane(const v3_lane&& o) {memcpy(this,&o,sizeof(v3_lane));}
	v3_lane operator=(const v3_lane& o) {memcpy(this,&o,sizeof(v3_lane)); return *this;}
	v3_lane operator=(const v3_lane&& o) {memcpy(this,&o,sizeof(v3_lane)); return *this;}
};
static_assert(sizeof(v3_lane) == 12 * 8, "sizeof(v3_lane) != 96");

v3_lane operator+(v3_lane l, v3_lane r) {
	return 
	{_mm256_add_ps(l.x, r.x),
	 _mm256_add_ps(l.y, r.y),
	 _mm256_add_ps(l.z, r.z)};
}
v3_lane operator+(v3_lane l, f32 _r) {
	__m256 r = _mm256_set1_ps(_r);
	return 
	{_mm256_add_ps(l.x, r),
	 _mm256_add_ps(l.y, r),
	 _mm256_add_ps(l.z, r)};
}
v3_lane operator+(f32 _l, v3_lane r) {
	__m256 l = _mm256_set1_ps(_l);
	return 
	{_mm256_add_ps(l, r.x),
	 _mm256_add_ps(l, r.y),
	 _mm256_add_ps(l, r.z)};
}
v3_lane operator+(v3_lane l, v3 r) {
	__m256 x = _mm256_set1_ps(r.x);
	__m256 y = _mm256_set1_ps(r.y);
	__m256 z = _mm256_set1_ps(r.z);
	return 
	{_mm256_add_ps(l.x, x),
	 _mm256_add_ps(l.y, y),
	 _mm256_add_ps(l.z, z)};
}
v3_lane operator+(v3 l, v3_lane r) {
	__m256 x = _mm256_set1_ps(l.x);
	__m256 y = _mm256_set1_ps(l.y);
	__m256 z = _mm256_set1_ps(l.z);
	return 
	{_mm256_add_ps(x, r.x),
	 _mm256_add_ps(y, r.y),
	 _mm256_add_ps(z, r.z)};
}
v3_lane operator-(v3_lane l, v3_lane r) {
	return 
	{_mm256_sub_ps(l.x, r.x),
	 _mm256_sub_ps(l.y, r.y),
	 _mm256_sub_ps(l.z, r.z)};
}
v3_lane operator-(v3_lane l, f32 _r) {
	__m256 r = _mm256_set1_ps(_r);
	return 
	{_mm256_sub_ps(l.x, r),
	 _mm256_sub_ps(l.y, r),
	 _mm256_sub_ps(l.z, r)};
}
v3_lane operator-(f32 _l, v3_lane r) {
	__m256 l = _mm256_set1_ps(_l);
	return 
	{_mm256_sub_ps(l, r.x),
	 _mm256_sub_ps(l, r.y),
	 _mm256_sub_ps(l, r.z)};
}
v3_lane operator-(v3_lane l, v3 r) {
	__m256 x = _mm256_set1_ps(r.x);
	__m256 y = _mm256_set1_ps(r.y);
	__m256 z = _mm256_set1_ps(r.z);
	return 
	{_mm256_sub_ps(l.x, x),
	 _mm256_sub_ps(l.y, y),
	 _mm256_sub_ps(l.z, z)};
}
v3_lane operator-(v3 l, v3_lane r) {
	__m256 x = _mm256_set1_ps(l.x);
	__m256 y = _mm256_set1_ps(l.y);
	__m256 z = _mm256_set1_ps(l.z);
	return 
	{_mm256_sub_ps(x, r.x),
	 _mm256_sub_ps(y, r.y),
	 _mm256_sub_ps(z, r.z)};
}
v3_lane operator*(v3_lane l, v3_lane r) {
	return 
	{_mm256_mul_ps(l.x, r.x),
	 _mm256_mul_ps(l.y, r.y),
	 _mm256_mul_ps(l.z, r.z)};
}
v3_lane operator*(v3_lane l, f32 _r) {
	__m256 r = _mm256_set1_ps(_r);
	return 
	{_mm256_mul_ps(l.x, r),
	 _mm256_mul_ps(l.y, r),
	 _mm256_mul_ps(l.z, r)};
}
v3_lane operator*(v3_lane l, v3 r) {
	__m256 x = _mm256_set1_ps(r.x);
	__m256 y = _mm256_set1_ps(r.y);
	__m256 z = _mm256_set1_ps(r.z);
	return 
	{_mm256_mul_ps(l.x, x),
	 _mm256_mul_ps(l.y, y),
	 _mm256_mul_ps(l.z, z)};
}
v3_lane operator*(v3 l, v3_lane r) {
	__m256 x = _mm256_set1_ps(l.x);
	__m256 y = _mm256_set1_ps(l.y);
	__m256 z = _mm256_set1_ps(l.z);
	return 
	{_mm256_mul_ps(x, r.x),
	 _mm256_mul_ps(y, r.y),
	 _mm256_mul_ps(z, r.z)};
}
v3_lane operator*(f32 _l, v3_lane r) {
	__m256 l = _mm256_set1_ps(_l);
	return 
	{_mm256_mul_ps(l, r.x),
	 _mm256_mul_ps(l, r.y),
	 _mm256_mul_ps(l, r.z)};
}
v3_lane operator/(v3_lane l, v3_lane r) {
	return 
	{_mm256_div_ps(l.x, r.x),
	 _mm256_div_ps(l.y, r.y),
	 _mm256_div_ps(l.z, r.z)};
}
v3_lane operator/(v3_lane l, f32 _r) {
	__m256 r = _mm256_set1_ps(_r);
	return 
	{_mm256_div_ps(l.x, r),
	 _mm256_div_ps(l.y, r),
	 _mm256_div_ps(l.z, r)};
}
v3_lane operator/(f32 _l, v3_lane r) {
	__m256 l = _mm256_set1_ps(_l);
	return 
	{_mm256_div_ps(l, r.x),
	 _mm256_div_ps(l, r.y),
	 _mm256_div_ps(l, r.z)};
}
v3_lane operator/(v3_lane l, v3 r) {
	__m256 x = _mm256_set1_ps(r.x);
	__m256 y = _mm256_set1_ps(r.y);
	__m256 z = _mm256_set1_ps(r.z);
	return 
	{_mm256_div_ps(l.x, x),
	 _mm256_div_ps(l.y, y),
	 _mm256_div_ps(l.z, z)};
}
v3_lane operator/(v3 l, v3_lane r) {
	__m256 x = _mm256_set1_ps(l.x);
	__m256 y = _mm256_set1_ps(l.y);
	__m256 z = _mm256_set1_ps(l.z);
	return 
	{_mm256_div_ps(x, r.x),
	 _mm256_div_ps(y, r.y),
	 _mm256_div_ps(z, r.z)};
}

f32_lane operator==(v3_lane l, v3_lane r) {
	__m256 cmpx = _mm256_cmp_ps(l.x,r.x,_CMP_EQ_OQ);
	__m256 cmpy = _mm256_cmp_ps(l.y,r.y,_CMP_EQ_OQ);
	__m256 cmpz = _mm256_cmp_ps(l.z,r.z,_CMP_EQ_OQ);
	return _mm256_and_ps(_mm256_and_ps(cmpx,cmpy),cmpz);
}
f32_lane operator!=(v3_lane l, v3_lane r) {
	__m256 cmpx = _mm256_cmp_ps(l.x,r.x,_CMP_NEQ_OQ);
	__m256 cmpy = _mm256_cmp_ps(l.y,r.y,_CMP_NEQ_OQ);
	__m256 cmpz = _mm256_cmp_ps(l.z,r.z,_CMP_NEQ_OQ);
	return _mm256_or_ps(_mm256_or_ps(cmpx,cmpy),cmpz);
}

v3_lane pow(v3_lane v, f32 _r) {
	__m256 r = _mm256_set1_ps(_r);
	return {_mm256_pow_ps(v.x,r),
			_mm256_pow_ps(v.y,r),
			_mm256_pow_ps(v.z,r)};
}

f32_lane operator&(f32_lane l, f32_lane r) {
	return {_mm256_and_ps(l.v, r.v)};
}
f32_lane operator|(f32_lane l, f32_lane r) {
	return {_mm256_or_ps(l.v, r.v)};
}

v3_lane operator&(v3_lane l, f32_lane r) {
	return {_mm256_and_ps(l.x, r.v),
			_mm256_and_ps(l.y, r.v),
			_mm256_and_ps(l.z, r.v)};
}
v3_lane operator|(v3_lane l, f32_lane r) {
	return {_mm256_or_ps(l.x, r.v),
			_mm256_or_ps(l.y, r.v),
			_mm256_or_ps(l.z, r.v)};
}
v3_lane operator&(f32_lane l, v3_lane r) {
	return {_mm256_and_ps(l.v, r.x),
			_mm256_and_ps(l.v, r.y),
			_mm256_and_ps(l.v, r.z)};
}
v3_lane operator|(f32_lane l, v3_lane r) {
	return {_mm256_or_ps(l.v, r.x),
			_mm256_or_ps(l.v, r.y),
			_mm256_or_ps(l.v, r.z)};
}

f32_lane operator+(f32_lane l, f32_lane r) {
	return {_mm256_add_ps(l.v, r.v)};
}
f32_lane operator+(f32_lane l, f32 _r) {
	__m256 r = _mm256_set1_ps(_r);
	return {_mm256_add_ps(l.v, r)};
}
f32_lane operator+(f32 _l, f32_lane r) {
	__m256 l = _mm256_set1_ps(_l);
	return {_mm256_add_ps(l, r.v)};
}
f32_lane operator-(f32_lane l, f32_lane r) {
	return {_mm256_sub_ps(l.v, r.v)};
}
f32_lane operator-(f32_lane l, f32 _r) {
	__m256 r = _mm256_set1_ps(_r);
	return {_mm256_sub_ps(l.v, r)};
}
f32_lane operator-(f32 _l, f32_lane r) {
	__m256 l = _mm256_set1_ps(_l);
	return {_mm256_sub_ps(l, r.v)};
}
f32_lane operator*(f32_lane l, f32_lane r) {
	return {_mm256_mul_ps(l.v, r.v)};
}
f32_lane operator*(f32_lane l, f32 _r) {
	__m256 r = _mm256_set1_ps(_r);
	return {_mm256_mul_ps(l.v, r)};
}
f32_lane operator*(f32 _l, f32_lane r) {
	__m256 l = _mm256_set1_ps(_l);
	return {_mm256_mul_ps(l, r.v)};
}
f32_lane operator/(f32_lane l, f32_lane r) {
	return {_mm256_div_ps(l.v, r.v)};
}
f32_lane operator/(f32_lane l, f32 _r) {
	__m256 r = _mm256_set1_ps(_r);
	return {_mm256_div_ps(l.v, r)};
}
f32_lane operator/(f32 _l, f32_lane r) {
	__m256 l = _mm256_set1_ps(_l);
	return {_mm256_div_ps(l, r.v)};
}

f32_lane operator==(f32_lane l, f32_lane r) {
	return _mm256_cmp_ps(l.v,r.v,_CMP_EQ_OQ);
}
f32_lane operator!=(f32_lane l, f32_lane r) {
	return _mm256_cmp_ps(l.v,r.v,_CMP_NEQ_OQ);
}
f32_lane operator>(f32_lane l, f32_lane r) {
	return _mm256_cmp_ps(l.v,r.v,_CMP_GT_OS);
}
f32_lane operator<(f32_lane l, f32_lane r) {
	return _mm256_cmp_ps(l.v,r.v,_CMP_LT_OS);
}
f32_lane operator>=(f32_lane l, f32_lane r) {
	return _mm256_cmp_ps(l.v,r.v,_CMP_GE_OS);
}
f32_lane operator<=(f32_lane l, f32_lane r) {
	return _mm256_cmp_ps(l.v,r.v,_CMP_LE_OS);
}
f32_lane operator==(f32_lane l, f32 r) {
	return _mm256_cmp_ps(l.v,_mm256_set1_ps(r),_CMP_EQ_OQ);
}
f32_lane operator!=(f32_lane l, f32 r) {
	return _mm256_cmp_ps(l.v,_mm256_set1_ps(r),_CMP_NEQ_OQ);
}
f32_lane operator>(f32_lane l, f32 r) {
	return _mm256_cmp_ps(l.v,_mm256_set1_ps(r),_CMP_GT_OS);
}
f32_lane operator<(f32_lane l, f32 r) {
	return _mm256_cmp_ps(l.v,_mm256_set1_ps(r),_CMP_LT_OS);
}
f32_lane operator>=(f32_lane l, f32 r) {
	return _mm256_cmp_ps(l.v,_mm256_set1_ps(r),_CMP_GE_OS);
}
f32_lane operator<=(f32_lane l, f32 r) {
	return _mm256_cmp_ps(l.v,_mm256_set1_ps(r),_CMP_LE_OS);
}
f32_lane operator==(f32 l, f32_lane r) {
	return _mm256_cmp_ps(_mm256_set1_ps(l),r.v,_CMP_EQ_OQ);
}
f32_lane operator!=(f32 l, f32_lane r) {
	return _mm256_cmp_ps(_mm256_set1_ps(l),r.v,_CMP_NEQ_OQ);
}
f32_lane operator>(f32 l, f32_lane r) {
	return _mm256_cmp_ps(_mm256_set1_ps(l),r.v,_CMP_GT_OS);
}
f32_lane operator<(f32 l, f32_lane r) {
	return _mm256_cmp_ps(_mm256_set1_ps(l),r.v,_CMP_LT_OS);
}
f32_lane operator>=(f32 l, f32_lane r) {
	return _mm256_cmp_ps(_mm256_set1_ps(l),r.v,_CMP_GE_OS);
}
f32_lane operator<=(f32 l, f32_lane r) {
	return _mm256_cmp_ps(_mm256_set1_ps(l),r.v,_CMP_LE_OS);
}

f32_lane pow(f32_lane l, f32 _r) {
	__m256 r = _mm256_set1_ps(_r);
	return {_mm256_pow_ps(l.v,r)};
}

f32_lane sqrt(f32_lane l) {
	return {_mm256_sqrt_ps(l.v)};
}
f32 sum(f32_lane l) {
	__m256 a = _mm256_hadd_ps(l.v,l.v);
	__m256 b = _mm256_hadd_ps(a,a);
	__m256 c = _mm256_hadd_ps(b,b);
	return f32_lane{c}.f[0];
}

std::ostream& operator<<(std::ostream& out, v3_lane r) {
	out << "{";
	for(i32 i = 0; i < LANE_WIDTH; i++) {
		out << "{" << r.xf[i] << "," << r.yf[i] << "," << r.zf[i] << "}";
		if(i != LANE_WIDTH - 1) out << ",";
	}
	out << "}";
	return out;
}
std::ostream& operator<<(std::ostream& out, f32_lane r) {
	out << "{";
	for(i32 i = 0; i < LANE_WIDTH; i++) {
		out << r.f[i];
		if(i != LANE_WIDTH - 1) out << ",";
	}
	out << "}";
	return out;
}

v3_lane operator*(v3 l, f32_lane r) {
	return v3_lane{l.x * r, l.y * r, l.z * r};
}

f32_lane dot(v3_lane l, v3_lane r) {
	__m256 x = _mm256_mul_ps(l.x,r.x);
	__m256 y = _mm256_mul_ps(l.y,r.y);
	__m256 z = _mm256_mul_ps(l.z,r.z);
	return _mm256_add_ps(_mm256_add_ps(x,y),z);
}

v3_lane reflect(v3_lane v, v3_lane n) {
	return v - 2.0f * dot(n, v) * n;
}
f32_lane lensq(v3_lane v) {
	return dot(v, v);
}
f32_lane len(v3_lane v) {
	return _mm256_sqrt_ps(lensq(v).v);
}
v3_lane norm(v3_lane v) {
	return v / len(v);
}
v3_lane lerp(v3_lane min, v3_lane max, f32_lane dist) {
	return (max - min) * dist + min;
}

v3_lane cross(v3_lane l, v3_lane r) {
	return {_mm256_mul_ps(l.y,r.z) -
			_mm256_mul_ps(l.z,r.y), 
			_mm256_mul_ps(l.z,r.x) - 
			_mm256_mul_ps(l.x,r.z), 
			_mm256_mul_ps(l.x,r.y) - 
			_mm256_mul_ps(l.y,r.x)};
}

struct ray_lane {
	v3_lane pos, dir;
	v3_lane get(f32_lane t) {return pos + t * dir;}
};

f32_lane randf_lane() {
	f32_lane ret;
	for(i32 i = 0; i < LANE_WIDTH; i++) {
		f32 r = randf_cpp();
		ret.f[i] = r;
	}
	return ret;
}
v3_lane random_leunit_lane() {
	v3_lane ret;
	for(i32 i = 0; i < LANE_WIDTH; i++) {
		v3 r = random_leunit();
		ret.xf[i] = r.x;
		ret.yf[i] = r.y;
		ret.zf[i] = r.z;
	}
	return ret;
}
v3_lane random_ledisk_lane() {
	v3_lane ret;
	for(i32 i = 0; i < LANE_WIDTH; i++) {
		v3 r = random_ledisk();
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
		std::cout << "pow: " << pow(_1,0.0f) << std::endl;
		std::cout << "pow: " << pow(_1,0.5f) << std::endl;
		std::cout << "pow: " << pow(_1,1.0f) << std::endl;
		std::cout << "pow: " << pow(_1,3.0f) << std::endl;
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
