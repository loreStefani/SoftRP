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
			//const float specularExp = 650.0f;
			const float specularExp = 50.0f;
			const float normFactor = (specularExp + 8.0f) / 8.0f;

			Math::Vector4 textCoordDerivatives[4];
			computeDDXDDY(psec, 1, textCoordDerivatives);

			const TextureUnit& textureUnit = *sc.textureUnits()[0];
			const TextureUnit& normalMapTextureUnit = *sc.textureUnits()[1];

			for (unsigned int i = 0; i < 4; i++)
			{
				if ((psec.mask & (1 << i)) == 0)
					continue;

				//add ambient term
				out[i] = Vector4{ 0.1f, 0.1f, 0.1f, 0.0f };

				//get texture sampling params
				const Math::Vector2& textCoords = *Math::vectorFromPtr<2>(psec.interpolated[i].getField(4));
				const Math::Vector2 dtcdx = *Math::vectorFromPtr<2, 4>(textCoordDerivatives + getDDXIndex(i));
				const Math::Vector2 dtcdy = *Math::vectorFromPtr<2, 4>(textCoordDerivatives + getDDYIndex(i));
				
				const Vector3& position = *Math::vectorFromPtr<3>(psec.interpolated[i].getField(1));
				Math::Vector3 toLight = (lightPos - position).normalize();
				Math::Vector3 normal = *Math::vectorFromPtr<3>(psec.interpolated[i].getField(2));
				normal.normalize();

				Math::Vector3 tangent = *Math::vectorFromPtr<3>(psec.interpolated[i].getField(3));
				tangent -= normal * dot(tangent, normal);
				tangent.normalize();

				Math::Vector4 normalMapSample = normalMapTextureUnit.sample(textCoords, dtcdx, dtcdy);
				normal = normalMap(normalMapSample, normal, tangent);

				const float cosTheta_i = normal.dot(toLight);
				if (cosTheta_i <= 0.0f)
					//the light source doesn't contribute					
					continue;

				//compute brdf

				//get diffuse color				
				Math::Vector4 brdf = textureUnit.sample(textCoords, dtcdx, dtcdy);
								
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


	private:
		Math::Vector3 normalMap(const Math::Vector4& normalMapSample,
								const Math::Vector3& normal, const Math::Vector3& tangent)const
		{
			
			Math::Vector3 normalMapSample3 = *Math::vectorFromPtr<3, 4>(&normalMapSample);

			Math::Vector3 bitangent = Math::cross(tangent, normal);

			Matrix3 tbn
			{
				tangent.x(), bitangent.x(), normal.x(),
				tangent.y(), bitangent.y(), normal.y(),
				tangent.z(), bitangent.z(), normal.z()
			};

			return (tbn * normalMapSample3).normalize();
		}
	};
}