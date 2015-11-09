#ifndef SOFTRP_PIXEL_SHADER_H_
#define SOFTRP_PIXEL_SHADER_H_
#include "ShaderContext.h"
#include "Vector.h"
#include "Vertex.h"
namespace SoftRP {
	
	/*
	Concrete data type which represents the context of a pixel shader's invocation.
	The context is composed by the interpolated vertices at the pixels' centers of a 
	2x2 pixel block. The mask indicates which pixels will actually be written to the 
	RenderTarget
	*/
	struct PSExecutionContext {
		int mask;
		Vertex interpolated[4];
	};

	/*
	Abstract data type which represents a pixel shader.
	*/

	class PixelShader{
	public:
		PixelShader() = default;
		virtual ~PixelShader() = default; 				
		virtual void operator() (const ShaderContext& sc, 
								 const PSExecutionContext& psec, 
								 size_t instance, Math::Vector4* out) const = 0;

		static void computeDDXDDY(const PSExecutionContext& psec, size_t fieldIndex, Math::Vector4* out);
		static unsigned int getDDXIndex(unsigned int pixelIndex);
		static unsigned int getDDYIndex(unsigned int pixelIndex);

	protected:
		PixelShader(const PixelShader&) = delete;
		PixelShader(PixelShader&&) = delete;
		PixelShader& operator=(const PixelShader&) = delete;
		PixelShader& operator=(PixelShader&&) = delete;
	};
}
#include "PixelShaderImpl.inl"
#endif
