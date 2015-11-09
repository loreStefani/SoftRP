#ifndef SOFTRP_TEXT_COORD_PIXEL_SHADER_H_
#define SOFTRP_TEXT_COORD_PIXEL_SHADER_H_
#include "PixelShader.h"
namespace SoftRP {

	/*
	Specialization of PixelShader which samples a texture and outputs the obtained sample.
	*/
	class TextCoordPixelShader : public PixelShader{
	public:
		TextCoordPixelShader() = default;
		virtual ~TextCoordPixelShader() = default;
		
		virtual void operator() (const ShaderContext& sc, 
								 const PSExecutionContext& psec, 
								 size_t instance, Math::Vector4* out) const override;
	protected:
		TextCoordPixelShader(const TextCoordPixelShader&) = delete;
		TextCoordPixelShader(TextCoordPixelShader&&) = delete;
		TextCoordPixelShader& operator=(const TextCoordPixelShader&) = delete;
		TextCoordPixelShader& operator=(TextCoordPixelShader&&) = delete;	
	};
}
#include "TextCoordPixelShaderImpl.inl"
#endif
