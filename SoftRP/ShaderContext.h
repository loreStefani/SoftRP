#ifndef SOFTRP_SHADER_CONTEXT_H_
#define SOFTRP_SHADER_CONTEXT_H_
namespace SoftRP {

	class ConstantBuffer;
	class TextureUnit;

	/*
	Concrete data type which represents the read-only context that a 
	shader can access during its execution.
	*/

	class ShaderContext {
	public:

		ShaderContext() = default;
		virtual ~ShaderContext() = default;

		//copy
		ShaderContext(const ShaderContext&) = default;		
		ShaderContext& operator=(const ShaderContext&) = default;

		//move
		ShaderContext(ShaderContext&&) = default;
		ShaderContext& operator=(ShaderContext&&) = default;

		/* getters */
		ConstantBuffer* const * constantBuffers() const;
		TextureUnit* const * textureUnits() const;

		/* setters */
		void setConstantBuffers(ConstantBuffer* const * constantBuffers);
		void setTextureUnits(TextureUnit* const * textureUnits);		
		
	private:
		ConstantBuffer* const * m_constantBuffers;
		TextureUnit* const * m_textureUnits;
	};
}
#include "ShaderContextImpl.inl"
#endif
