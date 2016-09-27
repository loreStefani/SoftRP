#pragma once
#include <DemoAppBase.h>

namespace SoftRPDemo
{
	using namespace SoftRP;
	using namespace Math;

	class LightVertexShader : public VertexShader
	{
	public:

		LightVertexShader() = default;
		virtual ~LightVertexShader() = default;

#ifdef SOFTRP_MULTI_THREAD
		virtual ThreadPool::Fence operator()(const ShaderContext& sc, const Vertex* input, Vertex* output,
											 size_t vertexCount, size_t instance, ThreadPool& threadPool) const override
		{
#else
		virtual void operator()(const ShaderContext& sc, const Vertex* input, Vertex* output,
								size_t vertexCount, size_t instance) const override
		{
#endif

			const Math::Matrix4* projViewWorld = sc.constantBuffers()[0]->getField(0, static_cast<unsigned int>(instance)).asMatrix4();
			const FMatrix fprojView = createFM(*projViewWorld);

			for (size_t i = 0; i < vertexCount; i++, input++, output++)
			{

				//assuming world transform == identity

				FVector worldPos = createFV(input->position());
				output->position() = createVector4FV(mulFM(fprojView, worldPos));
				*Math::vectorFromPtr<4>(output->getField(1)) = createVector4FV(worldPos);

				float* normal = output->getField(2);
				const float* inputNormal = input->getField(1);
				for (unsigned int i = 0; i < 3; i++)
					normal[i] = inputNormal[i];

				float* tangent = output->getField(3);
				const float* inputTangent = input->getField(2);
				for (unsigned int i = 0; i < 3; i++)
					tangent[i] = inputTangent[i];

				float* textCoords = output->getField(4);
				const float* inputTextCoords = input->getField(3);
				for (unsigned int i = 0; i < 2; i++)
					textCoords[i] = inputTextCoords[i];
			}

#ifdef SOFTRP_MULTI_THREAD
			return threadPool.currFence();
#endif
		}
	};
}