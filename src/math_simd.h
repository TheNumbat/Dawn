
#pragma once

#define LANE_WIDTH 8

#include "basic.h"
#include <math.h>
#include <random>

#ifdef _MSC_VER
#pragma warning(disable : 4201)
#endif

union f32_lane {
	__m256 v;
	f32 f[LANE_WIDTH];

	void operator+=(f32_lane x) {v = _mm256_add_ps(v, x.v);}
	void operator-=(f32_lane x) {v = _mm256_sub_ps(v, x.v);}
	void operator*=(f32_lane x) {v = _mm256_mul_ps(v, x.v);}
	void operator/=(f32_lane x) {v = _mm256_div_ps(v, x.v);}

	void operator*=(f32 _s) {__m256 s = _mm256_set1_ps(_s);
							 v = _mm256_mul_ps(v, s);}
	void operator/=(f32 _s) {__m256 s = _mm256_set1_ps(_s);
							 v = _mm256_div_ps(v, s);}

	f32_lane operator-() {__m256 _z = _mm256_setzero_ps();
						  return {_mm256_sub_ps(_z,v)};}

	f32_lane() {}
	f32_lane(f32 _v) {v = _mm256_set1_ps(_v);}
	f32_lane(__m256 _v) {v = _v;}

	f32_lane(const f32_lane& o) {memcpy(this,&o,sizeof(f32_lane));}
	f32_lane(const f32_lane&& o) {memcpy(this,&o,sizeof(f32_lane));}
	f32_lane operator=(const f32_lane& o) {memcpy(this,&o,sizeof(f32_lane)); return *this;}
	f32_lane operator=(const f32_lane&& o) {memcpy(this,&o,sizeof(f32_lane)); return *this;}
};

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

	void operator+=(v3_lane v) {x = _mm256_add_ps(x, v.x);
						   		y = _mm256_add_ps(y, v.y);
						   		z = _mm256_add_ps(z, v.z);}
	void operator-=(v3_lane v) {x = _mm256_sub_ps(x, v.x);
						   		y = _mm256_sub_ps(y, v.y);
						   		z = _mm256_sub_ps(z, v.z);}
	void operator*=(v3_lane v) {x = _mm256_mul_ps(x, v.x);
						   		y = _mm256_mul_ps(y, v.y);
						   		z = _mm256_mul_ps(z, v.z);}
	void operator/=(v3_lane v) {x = _mm256_div_ps(x, v.x);
						   		y = _mm256_div_ps(y, v.y);
						   		z = _mm256_div_ps(z, v.z);}
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

i32 operator==(v3_lane l, v3_lane r) {
	i32 cmpx = _mm256_movemask_ps(_mm256_cmp_ps(l.x,r.x,_CMP_EQ_OQ));
	i32 cmpy = _mm256_movemask_ps(_mm256_cmp_ps(l.y,r.y,_CMP_EQ_OQ));
	i32 cmpz = _mm256_movemask_ps(_mm256_cmp_ps(l.z,r.z,_CMP_EQ_OQ));
	return cmpx & cmpy & cmpz;
}
i32 operator!=(v3_lane l, v3_lane r) {
	i32 cmpx = _mm256_movemask_ps(_mm256_cmp_ps(l.x,r.x,_CMP_NEQ_OQ));
	i32 cmpy = _mm256_movemask_ps(_mm256_cmp_ps(l.y,r.y,_CMP_NEQ_OQ));
	i32 cmpz = _mm256_movemask_ps(_mm256_cmp_ps(l.z,r.z,_CMP_NEQ_OQ));
	return cmpx | cmpy | cmpz;
}

v3_lane pow(v3_lane v, f32 _r) {
	__m256 r = _mm256_set1_ps(_r);
	return {_mm256_pow_ps(v.x,r),
			_mm256_pow_ps(v.y,r),
			_mm256_pow_ps(v.z,r)};
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

bool operator==(f32_lane l, f32_lane r) {
	i32 cmp = _mm256_movemask_ps(_mm256_cmp_ps(l.v,r.v,_CMP_EQ_OQ));
	return cmp == 0xff;
}
bool operator!=(f32_lane l, f32_lane r) {
	i32 cmp = _mm256_movemask_ps(_mm256_cmp_ps(l.v,r.v,_CMP_NEQ_OQ));
	return cmp != 0;
}

f32_lane pow(f32_lane l, f32 _r) {
	__m256 r = _mm256_set1_ps(_r);
	return {_mm256_pow_ps(l.v,r)};
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
