#ifndef SOFTRP_FMATRIX_IMPL_INL_
#define SOFTRP_FMATRIX_IMPL_INL_
#include "FMatrix.h"
namespace SoftRP {

#ifdef SOFTRP_FMATH_SIMD
	inline FMatrix createFM(float r0c0, float r0c1, float r0c2, float r0c3,
				   float r1c0, float r1c1, float r1c2, float r1c3,
				   float r2c0, float r2c1, float r2c2, float r2c3,
				   float r3c0, float r3c1, float r3c2, float r3c3) {
		FMatrix m;
		m.doubleRows[0] = _mm256_set_ps(r1c3, r1c2, r1c1, r1c0,
										r0c3, r0c2, r0c1, r0c0);
		m.doubleRows[1] = _mm256_set_ps(r3c3, r3c2, r3c1, r3c0,
										r2c3, r2c2, r2c1, r2c0);
		return m;
	}

	inline FMatrix createFM(std::initializer_list<float> init) {
		if (init.size() != 16)
			throw std::runtime_error{ "Invalid initializer_list size" };
		FMatrix m;

		auto firstSecondRowsIt = init.begin();
		auto thirdFourthRowsIt = firstSecondRowsIt + 8;
		for (unsigned int i = 0; i < 8; i++) {
			m.doubleRows[0].m256_f32[i] = *firstSecondRowsIt;
			m.doubleRows[1].m256_f32[i] = *thirdFourthRowsIt;
			firstSecondRowsIt += 1;
			thirdFourthRowsIt += 1;
		}

		return m;
	}

	inline FMatrix createFM(const Math::Matrix4& m) {
		FMatrix res;		
		res.doubleRows[0] = _mm256_loadu_ps(m.data());
		res.doubleRows[1] = _mm256_loadu_ps(m.data() + 8);
		return res;
	}

	inline float get(FMatrix m, unsigned int i) {
		return i < 8 ? m.doubleRows[0].m256_f32[i] : m.doubleRows[1].m256_f32[i - 8];
	}

	inline float get(FMatrix m, unsigned int i, unsigned int j) {
		const bool rowEven = i % 2 == 0;
		unsigned int column = rowEven ? j : j + 4;
		if (i < 2)
			return m.doubleRows[0].m256_f32[column];
		else
			return m.doubleRows[1].m256_f32[column];		
	}

	inline FMatrix set(FMatrix m, unsigned int i, float value) {
		FMatrix m1 = m;
		if (i < 8)
			m1.doubleRows[0].m256_f32[i] = value;
		else
			m1.doubleRows[0].m256_f32[i - 8] = value;
		return m1;			
	}

	inline FMatrix set(FMatrix m, unsigned int i, unsigned int j, float value) {
		FMatrix m1 = m;
		const bool rowEven = i % 2 == 0;
		unsigned int column = rowEven ? j : j + 4;
		if(i < 2)
			m1.doubleRows[0].m256_f32[column] = value;
		else
			m1.doubleRows[1].m256_f32[column] = value;
		return m1;
	}

	inline FMatrix addFM(FMatrix m1, FMatrix m2) {
		FMatrix m;
		for (unsigned int i = 0; i < 2; i++)
			m.doubleRows[i] = _mm256_add_ps(m1.doubleRows[i], m2.doubleRows[i]);
		return m;
	}

	inline FMatrix subFM(FMatrix m1, FMatrix m2) {
		FMatrix m;
		for (unsigned int i = 0; i < 2; i++)
			m.doubleRows[i] = _mm256_sub_ps(m1.doubleRows[i], m2.doubleRows[i]);
		return m;
	}

	inline FMatrix mulFM(FMatrix m1, FMatrix m2){
		__m128 firstColumn2 = _mm256_extractf128_ps(m2.doubleRows[0], 0x00);
		__m128 secondColumn2 = _mm256_extractf128_ps(m2.doubleRows[0], 0x01);

		__m128 thirdColumn2 = _mm256_extractf128_ps(m2.doubleRows[1], 0x00);
		__m128 fourthColumn2 = _mm256_extractf128_ps(m2.doubleRows[1], 0x01);

		_MM_TRANSPOSE4_PS(firstColumn2, secondColumn2, thirdColumn2, fourthColumn2);

		FVector c0 = mulFM(m1, firstColumn2);
		FVector c1 = mulFM(m1, secondColumn2);
		FVector c2 = mulFM(m1, thirdColumn2);
		FVector c3 = mulFM(m1, fourthColumn2);
		
		_MM_TRANSPOSE4_PS(c0, c1, c2, c3);

		FMatrix res;
		res.doubleRows[0] = _mm256_set_m128(c1, c0);
		res.doubleRows[1] = _mm256_set_m128(c3, c2);
		return res;
	}

	inline FVector mulFM(FMatrix m, FVector v) {
		const __m256 doubleV = _mm256_set_m128(v, v);
		const __m256 firstSecondRowDot = _mm256_dp_ps(m.doubleRows[0], doubleV, 0xF1);
		const __m256 thirdFourthRowDot = _mm256_dp_ps(m.doubleRows[1], doubleV, 0xF1);

		const __m128 firstRowDot = _mm256_extractf128_ps(firstSecondRowDot, 0x00);
		const __m128 secondRowDot = _mm256_extractf128_ps(firstSecondRowDot, 0x01);

		const __m128 thirdRowDot = _mm256_extractf128_ps(thirdFourthRowDot, 0x00);
		const __m128 fourthRowDot = _mm256_extractf128_ps(thirdFourthRowDot, 0x01);

		__m128 firstSecondRowDot_128 = _mm_shuffle_ps(firstRowDot, secondRowDot, 0x00);
		__m128 thirdFourthRowDot_128 = _mm_shuffle_ps(thirdRowDot, fourthRowDot, 0x00);

		firstSecondRowDot_128 = _mm_permutevar_ps(firstSecondRowDot_128, _mm_set_epi32(0x02, 0x00, 0x02, 0x00));
		thirdFourthRowDot_128 = _mm_permutevar_ps(thirdFourthRowDot_128, _mm_set_epi32(0x02, 0x00, 0x02, 0x00));
		
		return _mm_shuffle_ps(firstSecondRowDot_128, thirdFourthRowDot_128, 0xE4); // 11 10 01 00
	}
#else
	inline FMatrix createFM(float r0c0, float r0c1, float r0c2, float r0c3,
							float r1c0, float r1c1, float r1c2, float r1c3,
							float r2c0, float r2c1, float r2c2, float r2c3,
							float r3c0, float r3c1, float r3c2, float r3c3) {
		return Math::Matrix4{
			r0c0, r0c1, r0c2, r0c3,
			r1c0, r1c1, r1c2, r1c3,
			r2c0, r2c1, r2c2, r2c3,
			r3c0, r3c1, r3c2, r3c3
		};
	}

	inline FMatrix createFM(std::initializer_list<float> init) {
		return Math::Matrix4{ init };
	}

	inline FMatrix createFM(const Math::Matrix4& m) {
		return m;
	}

	inline float get(FMatrix m, unsigned int i) {
		return m[i];
	}

	inline float get(FMatrix m, unsigned int i, unsigned int j) {
		return m.get(i, j);
	}

	inline FMatrix set(FMatrix m, unsigned int i, float value) {
		FMatrix m1 = m;
		m1[i] = value;
		return m1;
	}

	inline FMatrix set(FMatrix m, unsigned int i, unsigned int j, float value) {
		FMatrix m1 = m;
		m1.get(i, j) = value;
		return m1;
	}

	inline FMatrix addFM(FMatrix m1, FMatrix m2) {
		return m1 + m2;
	}

	inline FMatrix subFM(FMatrix m1, FMatrix m2) {
		return m1 - m2;
	}

	inline FMatrix mulFM(FMatrix m1, FMatrix m2) {
		return m1*m2;
	}

	inline FVector mulFM(FMatrix m, FVector v) {
		return m*v;
	}
#endif

}
#endif
