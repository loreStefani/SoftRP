#pragma once
#include <DemoAppBase.h>

namespace SoftRPDemo
{
	using namespace SoftRP;
	using namespace Math;

	class ConstBuffColorPixelShader : public PixelShader
	{
	public:
		ConstBuffColorPixelShader() = default;
		~ConstBuffColorPixelShader() = default;
		virtual void operator() (const ShaderContext& sc,
								 const PSExecutionContext& psec,
								 size_t instance, Math::Vector4* out) const override
		{
			const Math::Vector4* color = sc.constantBuffers()[1]->getField(1, instance).asVector4();
			for (unsigned int i = 0; i < 4; i++)
			{
				if ((psec.mask & (1 << i)) == 0)
					continue;
				out[i] = *color;
			}
		}
	};
}