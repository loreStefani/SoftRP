#ifndef SOFTRP_SHADER_CONTEXT_IMPL_INL_
#define SOFTRP_SHADER_CONTEXT_IMPL_INL_
#include "ShaderContext.h"
namespace SoftRP {

	inline void ShaderContext::setConstantBuffers(ConstantBuffer* const * constantBuffers) {
		m_constantBuffers = constantBuffers;
	}

	inline void ShaderContext::setTextureUnits(TextureUnit* const * textureUnits) {
		m_textureUnits = textureUnits;
	}

	inline ConstantBuffer* const * ShaderContext::constantBuffers() const {
		return m_constantBuffers;
	}

	inline TextureUnit* const * ShaderContext::textureUnits() const {
		return m_textureUnits;
	}
}
#endif
