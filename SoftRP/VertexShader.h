#ifndef SOFTRP_VERTEX_SHADER_H_
#define SOFTRP_VERTEX_SHADER_H_
#include "SoftRPDefs.h"
#include "ShaderContext.h"
#include "Vertex.h"
#include "ThreadPool.h"
namespace SoftRP {
	
	/*
	Abstract data type which represents a vertex shader.
	*/
	class VertexShader{
	public:
		
		VertexShader() = default;
		virtual ~VertexShader() = default;
		
#ifdef SOFTRP_MULTI_THREAD
		virtual ThreadPool::Fence operator()(const ShaderContext& sc,
											 const Vertex* input, Vertex* output, size_t vertexCount, size_t instance,
											 ThreadPool& threadPool) const = 0;
#else
		virtual void operator()(const ShaderContext& sc, 
								const Vertex* input, Vertex* output, size_t vertexCount, size_t instance = 0)const = 0;
#endif
				
	protected:
		VertexShader(const VertexShader&) = delete;
		VertexShader(VertexShader&&) = delete;
		VertexShader& operator=(const VertexShader&) = delete;
		VertexShader& operator=(VertexShader&&) = delete;
	};	
}
#endif