#ifndef SOFTRP_VERTEX_COLOR_PIXEL_SHADER_IMPL_INL_
#define SOFTRP_VERTEX_COLOR_PIXEL_SHADER_IMPL_INL_
#include "VertexColorPixelShader.h"
namespace SoftRP {

	inline void VertexColorPixelShader::operator() (const ShaderContext& sc,
													const PSExecutionContext& psec,
													size_t instance, Math::Vector4* out) const {
		for (unsigned int i = 0; i < 4; i++) {
			const float* vertexData = psec.interpolated[i].getField(1);
			out[i] = Math::Vector4{ vertexData[0], vertexData[1], vertexData[2], 1.0f };
		}
	}

}
#endif