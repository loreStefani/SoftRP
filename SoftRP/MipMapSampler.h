#ifndef SOFTRP_MIPMAP_SAMPLER_H_
#define SOFTRP_MIPMAP_SAMPLER_H_
#include "Sampler.h"
#include "MathCommon.h"
#include "PointSampler.h"
#include "LinearSampler.h"
namespace SoftRP {
		
	/*
	Sampler specialization which samples one mipmap (selected with the LOD parameter) with the Sampler InMipMapSampler
	*/
	template<typename InMipMapSampler>
	class MipMapSampler : public Sampler {
	public:
		MipMapSampler() = default;
		virtual ~MipMapSampler() = default;
		virtual Math::Vector4 sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords) const override;
		virtual Math::Vector4 sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords, float LOD) const override;
	protected:
		MipMapSampler(const MipMapSampler&) = delete;
		MipMapSampler(MipMapSampler&&) = delete;
		MipMapSampler& operator=(const MipMapSampler&) = delete;
		MipMapSampler& operator=(MipMapSampler&&) = delete;
	};

	/*
	MipMapSampler specialization which implements the OpenGL's NEAREST_MIPMAP_NEAREST.
	see : https://www.opengl.org/registry/doc/glspec45.core.pdf s.8.14.3
	*/
	using MipMapPointSampler = MipMapSampler<PointSampler>;

	/*
	MipMapSampler specialization which implements the OpenGL's LINEAR_MIPMAP_NEAREST.
	see : https://www.opengl.org/registry/doc/glspec45.core.pdf s.8.14.3
	*/
	using MipMapLinearSampler = MipMapSampler<LinearSampler>;


	/*
	Sampler specialization which samples two mipmaps (selected with the LOD parameter) 
	independently with the Sampler InMipMapSampler, then linearly interpolates between the two samples.
	*/
	template<typename InMipMapSampler>
	class AdjMipMapSampler : public Sampler {
	public:
		AdjMipMapSampler() = default;
		virtual ~AdjMipMapSampler() = default;

		virtual Math::Vector4 sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords)const override;
		virtual Math::Vector4 sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords, float LOD) const override;

	protected:
		AdjMipMapSampler(const AdjMipMapSampler&) = delete;
		AdjMipMapSampler(AdjMipMapSampler&&) = delete;
		AdjMipMapSampler& operator=(const AdjMipMapSampler&) = delete;
		AdjMipMapSampler& operator=(AdjMipMapSampler&&) = delete;
	};

	/*
	AdjMipMapSampler specialization which implements the OpenGL's LINEAR_MIPMAP_NEAREST.
	see : https://www.opengl.org/registry/doc/glspec45.core.pdf s.8.14.3
	*/
	using AdjMipMapPointSampler = AdjMipMapSampler<PointSampler>;

	/*
	AdjMipMapSampler specialization which implements the OpenGL's LINEAR_MIPMAP_LINEAR.
	see : https://www.opengl.org/registry/doc/glspec45.core.pdf s.8.14.3
	*/
	using AdjMipMapLinearSampler = AdjMipMapSampler<LinearSampler>;	
}
#include "MipMapSamplerImpl.inl"
#endif
