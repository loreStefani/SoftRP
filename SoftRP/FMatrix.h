#ifndef SOFTRP_FMATRIX_H_
#define SOFTRP_FMATRIX_H_
#include "SoftRPDefs.h"
#include "Matrix.h"
#include "SIMDInclude.h"
#include <initializer_list>
#include "FVector.h"

namespace SoftRP {

#ifdef SOFTRP_FMATH_SIMD
	struct FMatrix {
		__m256 doubleRows[2];
	};
#else
	using FMatrix = Math::Matrix4;
#endif

	FMatrix createFM(float r0c0 = 1.0f, float r0c1 = 0.0f, float r0c2 = 0.0f, float r0c3 = 0.0f,
				   float r1c0 = 0.0f, float r1c1 = 1.0f, float r1c2 = 0.0f, float r1c3 = 0.0f,
				   float r2c0 = 0.0f, float r2c1 = 0.0f, float r2c2 = 1.0f, float r2c3 = 0.0f,
				   float r3c0 = 0.0f, float r3c1 = 0.0f, float r3c2 = 0.0f, float r3c3 = 1.0f);

	FMatrix createFM(std::initializer_list<float> init);
	FMatrix createFM(const Math::Matrix4& m);
	float get(FMatrix m, unsigned int i);
	float get(FMatrix m, unsigned int i, unsigned int j);
	FMatrix set(FMatrix m, unsigned int i, float value);
	FMatrix set(FMatrix m, unsigned int i, unsigned int j, float value);
	FMatrix addFM(FMatrix m1, FMatrix m2);
	FMatrix subFM(FMatrix m1, FMatrix m2);
	FMatrix mulFM(FMatrix m1, FMatrix m2);
	FVector mulFM(FMatrix m, FVector v);

}

#include "FMatrixImpl.inl"

#endif