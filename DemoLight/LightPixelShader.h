#pragma once
#include <DemoAppBase.h>

namespace SoftRPDemo
{
	using namespace SoftRP;
	using namespace Math;

	class LightPixelShader : public PixelShader
	{
	public:

		LightPixelShader() = default;
		virtual ~LightPixelShader() = default;

		virtual void operator() (const ShaderContext& sc, const PSExecutionContext& psec, size_t instance, Math::Vector4* out) const override
		{
			const ConstantBuffer& constantBuffer = *sc.constantBuffers()[0];
			const Math::Vector4& lightColor = *constantBuffer.getField(1).asVector4();
			const Math::Vector3& eyePos = *constantBuffer.getField(2).asVector3();
			const Math::Vector3& lightPos = *constantBuffer.getField(3).asVector3();
			const float specularExp = 650.0f;
			const float normFactor = (specularExp + 2.0f) / 2.0f;

			Math::Vector4 textCoordDerivatives[4];
			computeDDXDDY(psec, 1, textCoordDerivatives);

			const TextureUnit& textureUnit = *sc.textureUnits()[0];
			for (unsigned int i = 0; i < 4; i++)
			{
				if ((psec.mask & (1 << i)) == 0)
					continue;

				//add ambient term
				out[i] = Vector4{ 0.05f, 0.05f, 0.05f, 0.0f };

				const Vector3& position = *Math::vectorFromPtr<3>(psec.interpolated[i].getField(1));
				Math::Vector3 toLight = (lightPos - position).normalize();
				Math::Vector3 normal = *Math::vectorFromPtr<3>(psec.interpolated[i].getField(2));
				normal.normalize();

				const float cosTheta_i = normal.dot(toLight);
				if (cosTheta_i <= 0.0f)
					//the light source doesn't contribute					
					continue;

				//compute brdf

				//get diffuse color
				const Math::Vector2& textCoords = *Math::vectorFromPtr<2>(psec.interpolated[i].getField(3));
				const Math::Vector4* dtcdx = textCoordDerivatives + getDDXIndex(i);
				const Math::Vector4* dtcdy = textCoordDerivatives + getDDYIndex(i);
				Math::Vector4 brdf = textureUnit.sample(textCoords, *Math::vectorFromPtr<2, 4>(dtcdx), *Math::vectorFromPtr<2, 4>(dtcdy));

				//get specular color
				Math::Vector4 specColor = Math::Vector4{ 1.0f, 1.0f, 1.0f, 1.0f } -brdf;
				specColor.w() = 1.0f;

				//specular contribution
				Math::Vector3 toCamera = (eyePos - position).normalize();
				Math::Vector3 halfVector = (toLight + toCamera).normalize();
				float cosTheta_h = normal.dot(halfVector);
				if (cosTheta_h > 0.0f)
					brdf += specColor * normFactor * std::powf(cosTheta_h, specularExp);

				//compute total contribution												
				out[i] += brdf * lightColor * cosTheta_i;
			}
		}
	};
}