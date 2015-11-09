#ifndef SOFTRP_TEXT_COORD_VERTEX_SHADER_IMPL_INL_
#define SOFTRP_TEXT_COORD_VERTEX_SHADER_IMPL_INL_
#include "TextCoordVertexShader.h"
#include "Matrix.h"
#include "FVector.h"
#include "FMatrix.h"
namespace SoftRP {
#ifdef SOFTRP_MULTI_THREAD		
	inline ThreadPool::Fence TextCoordVertexShader::operator()(const ShaderContext& sc,
															   const Vertex* input, Vertex* output, size_t vertexCount, size_t instance,
															   ThreadPool& threadPool) const
#else
	inline void TextCoordVertexShader::operator()(const ShaderContext& sc,
												  const Vertex* input, Vertex* output, size_t vertexCount, size_t instance) const
#endif

	{
		const Math::Matrix4* projViewWorld = sc.constantBuffers()[0]->getField(0, instance).asMatrix4();
		const FMatrix fprojViewWorld = createFM(*projViewWorld);
		for (size_t i = 0; i < vertexCount; i++, input++, output++) {
			FVector fv = createFV(input->position());
			output->position() = createVector4FV(mulFM(fprojViewWorld, fv));
			float* textCoords = output->getField(1);
			const float* inputTextCoords = input->getField(1);
			for (unsigned int i = 0; i < 2; i++)
				textCoords[i] = inputTextCoords[i];
		}

#ifdef SOFTRP_MULTI_THREAD
		return threadPool.currFence();
#endif
	}

}
#endif