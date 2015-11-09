#ifndef SOFTRP_TEXT_COORD_VERTEX_SHADER_H_
#define SOFTRP_TEXT_COORD_VERTEX_SHADER_H_
#include "VertexShader.h"
namespace SoftRP {

	/*
	Specialization of VertexShader which transforms vertices to Clip space and outputs the texture coordinates 
	associated with them.
	*/
	class TextCoordVertexShader : public VertexShader{
	public:
		TextCoordVertexShader() = default;
		virtual ~TextCoordVertexShader() = default;
				
#ifdef SOFTRP_MULTI_THREAD		
		virtual ThreadPool::Fence operator()(const ShaderContext& sc,
											 const Vertex* input, Vertex* output, size_t vertexCount, size_t instance,
											 ThreadPool& threadPool) const override;
#else
		virtual void operator()(const ShaderContext& sc,
								const Vertex* input, Vertex* output, size_t vertexCount, size_t instance) const override;
#endif

	protected:
		TextCoordVertexShader(const TextCoordVertexShader&) = delete;
		TextCoordVertexShader(TextCoordVertexShader&&) = delete;
		TextCoordVertexShader& operator=(const TextCoordVertexShader&) = delete;
		TextCoordVertexShader& operator=(TextCoordVertexShader&&) = delete;					
	};
}
#include "TextCoordVertexShaderImpl.inl"
#endif
