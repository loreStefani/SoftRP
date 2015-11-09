#ifndef SOFTRP_POSITION_VERTEX_SHADER_IMPL_INL_
#define SOFTRP_POSITION_VERTEX_SHADER_IMPL_INL_
#include "PositionVertexShader.h"
#include "Matrix.h"
#include "FMatrix.h"
#include "FVector.h"
namespace SoftRP {
#ifdef SOFTRP_MULTI_THREAD		 
	inline ThreadPool::Fence PositionVertexShader::operator()(const ShaderContext& sc,
															  const Vertex* input, Vertex* output, size_t vertexCount, size_t instance,
															  ThreadPool& threadPool) const
#else
	inline void PositionVertexShader::operator()(const ShaderContext& sc,
												 const Vertex* input, Vertex* output, size_t vertexCount, size_t instance) const
#endif
	{
		const Math::Matrix4* projViewWorld = sc.constantBuffers()[0]->getField(0, instance).asMatrix4();
		const FMatrix fprojViewWorld = createFM(*projViewWorld);
		for (size_t i = 0; i < vertexCount; i++, input++, output++)
			output->position() = createVector4FV(mulFM(fprojViewWorld, createFV(input->position())));

#ifdef SOFTRP_MULTI_THREAD
		return threadPool.currFence();
#endif
	}

}
#endif