#ifndef SOFTRP_VERTEX_COLOR_PIXEL_SHADER_H_
#define SOFTRP_VERTEX_COLOR_PIXEL_SHADER_H_
#include "PixelShader.h"
#include "Vector.h"
namespace SoftRP{

	/*
	Specialization of PixelShader which outputs the color interpolated from the vertices.
	*/
	class VertexColorPixelShader  : public PixelShader{
	public:
		VertexColorPixelShader() = default;
		virtual ~VertexColorPixelShader() = default;		
		virtual void operator() (const ShaderContext& sc, 
								 const PSExecutionContext& psec, 
								 size_t instance, Math::Vector4* out) const override;
	protected:
		VertexColorPixelShader(const VertexColorPixelShader&) = delete;
		VertexColorPixelShader(VertexColorPixelShader&&) = delete;
		VertexColorPixelShader& operator=(const VertexColorPixelShader&) = delete;
		VertexColorPixelShader& operator=(VertexColorPixelShader&&) = delete;
	};
}
#include "VertexColorPixelShaderImpl.inl"
#endif
