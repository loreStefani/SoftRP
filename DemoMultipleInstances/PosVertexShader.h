#pragma once
#include <DemoAppBase.h>

namespace SoftRPDemo
{
	using namespace SoftRP;
	using namespace Math;

	class PosVertexShader : public VertexShader
	{
	public:
		PosVertexShader() = default;
		virtual ~PosVertexShader() = default;
#ifdef SOFTRP_MULTI_THREAD
		virtual	ThreadPool::Fence operator()(const ShaderContext& sc,
											 const Vertex* input, Vertex* output, size_t vertexCount, size_t instance,
											 ThreadPool& threadPool) const override
		{
#else
		virtual	void operator()(const ShaderContext& sc,
								const Vertex* input, Vertex* output, size_t vertexCount,
								size_t instance) const override
		{
#endif
			const Math::Matrix4* projView = sc.constantBuffers()[0]->getField(0).asMatrix4();
			const Math::Matrix4* world = sc.constantBuffers()[1]->getField(0, instance).asMatrix4();
			const FMatrix fprojViewWorld = mulFM(createFM(*projView), createFM(*world));
			for (size_t i = 0; i < vertexCount; i++, input++, output++)
				output->position() = createVector4FV(mulFM(fprojViewWorld, createFV(input->position())));

#ifdef SOFTRP_MULTI_THREAD
			return threadPool.currFence();
#endif
		}
	};
}