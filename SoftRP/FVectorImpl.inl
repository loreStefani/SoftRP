#ifndef SOFTRP_FVECTOR_IMPL_INL_
#define SOFTRP_FVECTOR_IMPL_INL_
#include "FVector.h"
#include <stdexcept>

namespace SoftRP {

#ifdef SOFTRP_FMATH_SIMD
	inline float getXFV(FVector v) {	
		return v.m128_f32[0];
		/*return static_cast<float>(_mm_extract_ps(v, 0));*/
	}

	inline float getYFV(FVector v) {
		/*return static_cast<float>(_mm_extract_ps(v, 1));*/
		return v.m128_f32[1];
	}

	inline float getZFV(FVector v) {
		/*return static_cast<float>(_mm_extract_ps(v, 2));*/
		return v.m128_f32[2];
	}

	inline float getWFV(FVector v) {
		/*return static_cast<float>(_mm_extract_ps(v, 3));*/
		return v.m128_f32[3];
	}

	inline float get(FVector v, unsigned int i) {
		return v.m128_f32[i];		
	}	

	inline FVector setXFV(FVector in, float x) {
		__m128 xVec = _mm_set_ps1(x);
		/*
		0xC0 == 11 00 0000:
		11 : the fourth component of xVec is taken
		00 : "in"'s components are copied except for the first one, 
			 which is replaced by the one taken as explained above
		0000 : all components are inserted in the result
		*/
		return _mm_insert_ps(in, xVec, 0xC0);
	}

	inline FVector setYFV(FVector in, float y) {
		__m128 yVec = _mm_set_ps1(y);
		/*
		0xD0 == 11 01 0000:
		11 : the fourth component of yVec is taken
		01 : "in"'s components are copied except for the second one,
		which is replaced by the one taken as explained above
		0000 : all components are inserted in the result
		*/
		return _mm_insert_ps(in, yVec, 0xD0);
	}

	inline FVector setZFV(FVector in, float z) {
		__m128 zVec = _mm_set_ps1(z);
		/*
		0xE0 == 11 10 0000:
		11 : the fourth component of zVec is taken
		10 : "in"'s components are copied except for the third one,
		which is replaced by the one taken as explained above
		0000 : all components are inserted in the result
		*/
		return _mm_insert_ps(in, zVec, 0xE0);
	}

	inline FVector setWFV(FVector in, float w) {
		__m128 wVec = _mm_set_ps1(w);
		/*
		0xF0 == 11 11 0000:
		11 : the fourth component of wVec is taken
		11 : "in"'s components are copied except for the fourth one,
		which is replaced by the one taken as explained above
		0000 : all components are inserted in the result
		*/
		return _mm_insert_ps(in, wVec, 0xF0);
	}
		
	inline FVector set(FVector v, unsigned int i, float value) {
		FVector res = v;
		res.m128_f32[i] = value;
		return res;
	}

	inline FVector createFV(float x, float y, float z, float w) {
		return _mm_set_ps(w, z, y, x);
	}

	inline FVector createFV(std::initializer_list<float> init) {
		if (init.size() != 4)
			throw std::runtime_error{ "Invalid initializer_list size." };
		unsigned int i = 0;
		__m128 val;
		for (float v : init)
			val.m128_f32[i++] = v;
		return val;
	}
	
	inline FVector createFV(float(&data)[4]) {
		__m128 val;
		for (unsigned int i = 0; i < 4; i++)
			val.m128_f32[i] = data[i];
		return val;
	}
	
	template<typename Iterator>
	inline FVector createFV (Iterator beg, Iterator end) {
		auto range = end - beg;
		if (range != 4)
			throw std::runtime_error{ "Invalid range size." };
		__m128 val;
		unsigned int i = 0;
		while (beg != end) {
			val.m128_f32[i] = *beg;
			i++;
			beg++;
		}
		return val;
	}
		
	inline FVector createFV(const Math::Vector4& v) {
		return createFV(v[0], v[1], v[2], v[3]);
	}

	inline Math::Vector4 createVector4FV(FVector v) {
		return Math::Vector4{ reinterpret_cast<float*>(&v), reinterpret_cast<float*>(&v) + 4 };
	}

	inline FVector createFromUnalignedFV(const float* addr) {
		return _mm_loadu_ps(addr);
	}

	inline FVector creteFromAlignedFV(const float* addr) {
		return _mm_load_ps(addr);
	}

	inline FVector addFV(FVector v1, FVector v2) {
		return _mm_add_ps(v1, v2);
	}

	inline FVector subFV(FVector v1, FVector v2) {
		return _mm_sub_ps(v1, v2);
	}

	inline FVector mulFV(FVector v1, FVector v2) {
		return _mm_mul_ps(v1, v2);
	}

	inline FVector scaleFV(FVector v, float scale) {
		__m128 scaleVec = _mm_set_ps1(scale);
		return _mm_mul_ps(v, scaleVec);
	}

	inline FVector divFV(FVector v1, FVector v2) {
		return _mm_div_ps(v1, v2);
	}

	inline FVector dotReplicateFV(FVector v1, FVector v2) {
		return _mm_dp_ps(v1, v2, 0xFF);
	}
		
	inline float dotFV(FVector v1, FVector v2) {
		return _mm_dp_ps(v1, v2, 0xF1).m128_f32[0];
	}
	
	inline FVector minFV(FVector v1, FVector v2) {
		__m128 comp = _mm_cmplt_ps(v1, v2);
		//TODO : improve
		__m128 res;
		for (unsigned int i = 0; i < 4; i++)
			res.m128_f32[i] = comp.m128_i32[i] ? v1.m128_f32[i] : v2.m128_f32[i];
		return res;		
	}

	inline FVector maxFV(FVector v1, FVector v2) {
		__m128 comp = _mm_cmpgt_ps(v1, v2);
		//TODO : improve
		__m128 res;
		for (unsigned int i = 0; i < 4; i++)
			res.m128_f32[i] = comp.m128_i32[i] ? v1.m128_f32[i] : v2.m128_f32[i];
		return res;
	}
	
	inline FVector lerpFV(FVector v1, FVector v2, float t) {
		const __m128 tVec = _mm_set_ps1(t);
		const __m128 oneMinusTVec = _mm_set_ps1(1.0f - t);
		const __m128 temp = _mm_mul_ps(v1, oneMinusTVec);
		const __m128 temp1 = _mm_mul_ps(v2, tVec);
		return _mm_add_ps(temp, temp1);
	}

	inline FVector lengthReplicateFV(FVector v) {
		const __m128 squaredLength = dotReplicateFV(v, v);
		return _mm_sqrt_ps(squaredLength);
	}

	inline float lengthFV(FVector v) {
		const float squaredLength = dotFV(v, v);
		return std::sqrtf(squaredLength);
	}

	inline FVector normalizeFV(FVector v) {
		const __m128 squaredLengthRep = dotReplicateFV(v, v);
		//TODO : division by zero ?
		const __m128 invSqrt = _mm_rsqrt_ps(squaredLengthRep);
		return _mm_mul_ps(v, invSqrt);
	}

	//assuming v1.w == v2.w == 1.0f
	inline FVector cross3FV(FVector v1, FVector v2) {
		//temp = (w, z, y, x) = (1, v2y, v2x, v2z)
		__m128 temp = _mm_set_ps(1.0f, v2.m128_f32[1], v2.m128_f32[0], v2.m128_f32[2]);
		//temp = (x, y, z, w) = (v1x*v2z, v1y*v2x, v1z*v2y, 1)
		temp = _mm_mul_ps(v1, temp);
		//temp1 = (w, z, y, x) = (2, v2x, v2z, v2y)
		__m128 temp1 = _mm_set_ps(2.0f, v2.m128_f32[0], v2.m128_f32[2], v2.m128_f32[1]);
		//temp1 = (x, y, z, w) = (v1x*v2y, v1y*v2z, v1z*v2x, 2)
		temp1 = _mm_mul_ps(v1, temp1);

		/*
			shuffle temp's component by : 0xD2 == 11 01 00 10
			where each 2-bit group is an index which selects a component in one of the two arguments. The four least 
			significative ones select from the first argument, whereas the four most significative ones select from
			the second argument.
			temp = (x, y, z, w) = (v1z*v2y, v1x*v2z, v1y*v2x, 1)
		*/
		temp = _mm_shuffle_ps(temp, temp, 0xD2);
		/*
			0xC9 == 11 00 10 01
			temp1 = (x, y, z, w) = (v1y*v2z, v1z*v2x, v1x*v2y, 2)
		*/
		temp1 = _mm_shuffle_ps(temp1, temp1, 0xC9);

		//return temp1 - temp = (x, y, z, w) = (v1y*v2z - v1z*v2y, v1z*v2x - v1x*v2z, v1x*v2y - v1y*v2x, 1)
		return _mm_sub_ps(temp1, temp);
	}	

#else

	inline float getXFV(FVector v) {
		return v[0];
	}

	inline float getYFV(FVector v) {
		return v[1];
	}

	inline float getZFV(FVector v) {
		return v[2];
	}

	inline float getWFV(FVector v) {
		return v[3];
	}

	inline float get(FVector v, unsigned int i) {
		return v[i];
	}

	inline FVector setXFV(FVector in, float x) {
		in[0] = x;
	}

	inline FVector setYFV(FVector in, float y) {
		in[1] = y;
	}

	inline FVector setZFV(FVector in, float z) {
		in[2] = z;
	}

	inline FVector setWFV(FVector in, float w) {
		in[3] = w;
	}
		
	inline FVector set(FVector v, unsigned int i, float value) {
		FVector res = v;
		res[i] = value;
		return res;
	}

	inline FVector createFV(float x, float y, float z, float w) {
		return Math::Vector4{ x, y, z, w };
	}

	inline FVector createFV(std::initializer_list<float> init) {
		return Math::Vector4{ init };
	}

	inline FVector createFV(float(&data)[4]) {
		return Math::Vector4{ data };
	}

	template<typename Iterator>
	inline FVector createFV(Iterator beg, Iterator end) {
		return Math::Vector4{ beg, end };
	}

	inline FVector createFV(const Math::Vector4& v) {
		return v;
	}

	inline Math::Vector4 createVector4FV(FVector v) {
		return v;
	}

	inline FVector createFromUnalignedFV(const float* addr) {
		return Math::Vector4{ addr, addr + 4 };
	}

	inline FVector creteFromAlignedFV(const float* addr) {
		return createFromUnalignedFV(addr);
	}

	inline FVector addFV(FVector v1, FVector v2) {
		return v1 + v2;
	}

	inline FVector subFV(FVector v1, FVector v2) {
		return v1 - v2;
	}

	inline FVector mulFV(FVector v1, FVector v2) {
		return v1*v2;
	}

	inline FVector scaleFV(FVector v, float scale) {
		return v * scale;
	}

	inline FVector divFV(FVector v1, FVector v2) {
		return v1 / v2;
	}

	inline FVector dotReplicateFV(FVector v1, FVector v2) {
		float dotRes = v1.dot(v2);
		return Math::Vector4{ dotRes, dotRes, dotRes, dotRes };
	}
		
	inline float dotFV(FVector v1, FVector v2) {
		return v1.dot(v2);
	}

	inline FVector minFV(FVector v1, FVector v2) {
		return Math::min(v1, v2);
	}

	inline FVector maxFV(FVector v1, FVector v2) {
		return Math::max(v1, v2);
	}

	inline FVector lerpFV(FVector v1, FVector v2, float t) {
		return Math::lerp(t, v1, v2);
	}

	inline FVector lengthReplicateFV(FVector v) {
		float lengthRes = v.length();
		return Math::Vector4{ lengthRes, lengthRes, lengthRes, lengthRes };
	}

	inline float lengthFV(FVector v) {
		return v.length();
	}

	inline FVector normalizeFV(FVector v) {
		Math::Vector4 res = v;
		res.normalize();
		return res;
	}
		
	inline FVector cross3FV(FVector v1, FVector v2) {
		Math::Vector3 _v1{ v1 };
		Math::Vector3 _v2{ v2 };
		return Math::Vector4{ Math::cross(_v1, _v2), 1.0f };
	}
#endif

}

#endif
