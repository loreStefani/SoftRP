#ifndef SOFTRP_POSITION_VERTEX_SHADER_H_
#define SOFTRP_POSITION_VERTEX_SHADER_H_
#include "SoftRPDefs.h"
#include "VertexShader.h"
namespace SoftRP {

	/*
	Specialization of VertexShader which transforms vertices to Clip space.
	*/
	class PositionVertexShader : public VertexShader{
	public:
		PositionVertexShader() = default;
		virtual ~PositionVertexShader() = default;
				
#ifdef SOFTRP_MULTI_THREAD		 
		virtual	ThreadPool::Fence operator()(const ShaderContext& sc,
										const Vertex* input, Vertex* output, size_t vertexCount, size_t instance, 
										 ThreadPool& threadPool) const override;
#else
		virtual void operator()(const ShaderContext& sc,
							const Vertex* input, Vertex* output, size_t vertexCount, size_t instance) const override;
#endif
	protected:
		PositionVertexShader(const PositionVertexShader&) = delete;
		PositionVertexShader(PositionVertexShader&&) = delete;
		PositionVertexShader& operator=(const PositionVertexShader&) = delete;
		PositionVertexShader& operator=(PositionVertexShader&&) = delete;	
	};
}
#include "PositionVertexShaderImpl.inl"
#endif
