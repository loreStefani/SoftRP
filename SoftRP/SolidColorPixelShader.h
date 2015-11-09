#ifndef SOFTRP_SOLID_COLOR_PIXEL_SHADER_H_
#define SOFTRP_SOLID_COLOR_PIXEL_SHADER_H_
#include "PixelShader.h"
namespace SoftRP {

	/*
	Specialization of PixelShader which outputs a solid color.
	*/

	template<unsigned int r, unsigned int g, unsigned int b, unsigned int a = 255>
	class SolidColorPixelShader : public PixelShader {
	public:
		SolidColorPixelShader() = default;
		virtual ~SolidColorPixelShader() = default;		
		virtual void operator() (const ShaderContext& sc, 
								 const PSExecutionContext& psec, 
								 size_t instance, Math::Vector4* out) const override;
	protected:
		SolidColorPixelShader(const SolidColorPixelShader&) = delete;
		SolidColorPixelShader(SolidColorPixelShader&&) = delete;
		SolidColorPixelShader& operator=(const SolidColorPixelShader&) = delete;
		SolidColorPixelShader& operator=(SolidColorPixelShader&&) = delete;
	};
}
#include "SolidColorPixelShaderImpl.inl"
#endif
