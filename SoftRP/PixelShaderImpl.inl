#ifndef SOFTRP_PIXEL_SHADER_IMPL_INL_
#define SOFTRP_PIXEL_SHADER_IMPL_INL_
#include "PixelShader.h"
#include "SIMDInclude.h"
namespace SoftRP {

	inline void PixelShader::computeDDXDDY(const PSExecutionContext& psec, size_t fieldIndex, Math::Vector4* out) {
#ifdef SOFTRP_USE_SIMD

		const __m128 field0 = _mm_load_ps(psec.interpolated[0].getField(fieldIndex));
		const __m128 field1 = _mm_load_ps(psec.interpolated[1].getField(fieldIndex));
		const __m128 field2 = _mm_load_ps(psec.interpolated[2].getField(fieldIndex));
		const __m128 field3 = _mm_load_ps(psec.interpolated[3].getField(fieldIndex));
		const __m256 field02 = _mm256_set_m128(field0, field2);
		const __m256 field13 = _mm256_set_m128(field1, field3);

		// df/dx with backward differencing
		const __m256 dfdx = _mm256_sub_ps(field02, field13);
		_mm256_storeu2_m128(out[0].data(), out[1].data(), dfdx);

		const __m256 field01 = _mm256_set_m128(field0, field1);
		const __m256 field23 = _mm256_set_m128(field2, field3);

		// df/dy with backward differencing
		const __m256 dfdy = _mm256_sub_ps(field01, field23);
		_mm256_storeu2_m128(out[2].data(), out[3].data(), dfdy);

#else
		const Math::Vector4& field0 = *Math::vectorFromPtr<4>(psec.interpolated[0].getField(fieldIndex));
		const Math::Vector4& field1 = *Math::vectorFromPtr<4>(psec.interpolated[1].getField(fieldIndex));
		const Math::Vector4& field2 = *Math::vectorFromPtr<4>(psec.interpolated[2].getField(fieldIndex));
		const Math::Vector4& field3 = *Math::vectorFromPtr<4>(psec.interpolated[3].getField(fieldIndex));

		// df/dx with backward differencing
		out[0] = field0 - field1;
		out[1] = field2 - field3;
		// df/dy with backward differencing
		out[2] = field0 - field2;
		out[3] = field1 - field3;
#endif
	}

	inline unsigned int PixelShader::getDDXIndex(unsigned int pixelIndex) {
		switch (pixelIndex) {
		case 0:
		case 1:
			return 0;
		case 2:
		case 3:
			return 1;
		}
		throw std::runtime_error{ "Invalid pixel index" };
	}

	inline unsigned int PixelShader::getDDYIndex(unsigned int pixelIndex) {
		switch (pixelIndex) {
		case 0:
		case 2:
			return 2;
		case 1:
		case 3:
			return 3;
		}
		throw std::runtime_error{ "Invalid pixel index" };
	}

}
#endif
