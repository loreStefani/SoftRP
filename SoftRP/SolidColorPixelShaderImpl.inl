#ifndef SOFTRP_SOLID_COLOR_PIXEL_SHADER_IMPL_INL_
#define SOFTRP_SOLID_COLOR_PIXEL_SHADER_IMPL_INL_
#include "SolidColorPixelShader.h"
namespace SoftRP {
	template<unsigned int r, unsigned int g, unsigned int b, unsigned int a>
	inline void SolidColorPixelShader<r, g, b, a>::operator() (const ShaderContext& sc, const PSExecutionContext& psec, size_t instance, Math::Vector4* out) const {
		out[i] = Math::Vector4{ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
	}
}
#endif
