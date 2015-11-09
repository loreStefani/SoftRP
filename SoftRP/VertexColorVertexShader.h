#ifndef SOFTRP_VERTEX_COLOR_VERTEX_SHADER_H_
#define SOFTRP_VERTEX_COLOR_VERTEX_SHADER_H_
#include "VertexShader.h"
namespace SoftRP {

	/*
	Specialization of VertexShader which transforms vertices to Clip space and outputs the color
	associated with them.
	*/
	class VertexColorVertexShader : public VertexShader {
	public:

		VertexColorVertexShader() = default;
		virtual ~VertexColorVertexShader() = default;

#ifdef SOFTRP_MULTI_THREAD		
		virtual ThreadPool::Fence operator()(const ShaderContext& sc,
											 const Vertex* input, Vertex* output, size_t vertexCount, size_t instance,
											 ThreadPool& threadPool) const override;
#else
		virtual void operator()(const ShaderContext& sc,
								const Vertex* input, Vertex* output, size_t vertexCount, size_t instance) const override;
#endif
	protected:
		VertexColorVertexShader(const VertexColorVertexShader&) = delete;
		VertexColorVertexShader(VertexColorVertexShader&&) = delete;
		VertexColorVertexShader& operator=(const VertexColorVertexShader&) = delete;
		VertexColorVertexShader& operator=(VertexColorVertexShader&&) = delete;
	};
}
#include "VertexColorVertexShaderImpl.inl"
#endif