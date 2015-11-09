#ifndef SOFTRP_VERTEX_COLOR_VERTEX_SHADER_IMPL_INL_
#define SOFTRP_VERTEX_COLOR_VERTEX_SHADER_IMPL_INL_
#include "VertexColorVertexShader.h"
#include "FVector.h"
#include "FMatrix.h"
#include "Matrix.h"
namespace SoftRP {

#ifdef SOFTRP_MULTI_THREAD		
	inline ThreadPool::Fence VertexColorVertexShader::operator()(const ShaderContext& sc,
																 const Vertex* input, Vertex* output, size_t vertexCount, size_t instance,
																 ThreadPool& threadPool) const
#else
	inline void VertexColorVertexShader::operator()(const ShaderContext& sc,
													const Vertex* input, Vertex* output, size_t vertexCount, size_t instance) const
#endif

	{
		const Math::Matrix4* projViewWorld = sc.constantBuffers()[0]->getField(0, instance).asMatrix4();
		const FMatrix fprojViewWorld = createFM(*projViewWorld);
		for (size_t i = 0; i < vertexCount; i++, input++, output++) {
			FVector fv = createFV(input->position());
			output->position() = createVector4FV(mulFM(fprojViewWorld, fv));
			float* vertColor = output->getField(1);
			const float* inputVertColor = input->getField(1);
			for (unsigned int i = 0; i < 3; i++)
				vertColor[i] = inputVertColor[i];
		}

#ifdef SOFTRP_MULTI_THREAD
		return threadPool.currFence();
#endif
	}


}
#endif