#ifndef SOFTRP_FVECTOR_H_
#define SOFTRP_FVECTOR_H_
#include "SoftRPDefs.h"
#include "Vector.h"
#include "SIMDInclude.h"
#include <initializer_list>

namespace SoftRP {

#ifdef SOFTRP_FMATH_SIMD
	using FVector = __m128;
#else
	using FVector = Math::Vector4;
#endif
		
	float getXFV(FVector v);
	float getYFV(FVector v);
	float getZFV(FVector v);
	float getWFV(FVector v);
	float get(FVector v, unsigned int i);	
	FVector setXFV(FVector in, float x);
	FVector setYFV(FVector in, float y);
	FVector setZFV(FVector in, float z);
	FVector setWFV(FVector in, float w);
	FVector set(FVector v, unsigned int i, float value);
	FVector createFV(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f);
	FVector createFV(std::initializer_list<float> init);
	FVector createFV(float(&data)[4]);	
	FVector createFV(const Math::Vector4& v);
	FVector creteFromAlignedFV(const float* addr);
	FVector createFromUnalignedFV(const float* addr);
	Math::Vector4 createVector4FV(FVector v);

	template<typename Iterator>
	FVector createFV(Iterator beg, Iterator end);

	FVector addFV(FVector v1, FVector v2);
	FVector subFV(FVector v1, FVector v2);
	FVector mulFV(FVector v1, FVector v2);
	FVector scaleFV(FVector v, float scale);
	FVector divFV(FVector v1, FVector v2);
	FVector dotReplicateFV(FVector v1, FVector v2);	
	float dotFV(FVector v1, FVector v2);
	FVector minFV(FVector v1, FVector v2);
	FVector maxFV(FVector v1, FVector v2);
	FVector lerpFV(FVector v1, FVector v2, float t);
	FVector lengthReplicateFV(FVector v);
	float lengthFV(FVector v);
	FVector normalizeFV(FVector v);
	//assuming v1.w == v2.w == 1.0f
	FVector cross3FV(FVector v1, FVector v2);	
}

#include "FVectorImpl.inl"

#endif