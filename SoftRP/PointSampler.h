#ifndef SOFTRP_POINT_SAMPLER_H_
#define SOFTRP_POINT_SAMPLER_H_
#include "Sampler.h"
namespace SoftRP {

	/*
	Sampler specialization which implements the OpenGL's NEAREST.
	see : https://www.opengl.org/registry/doc/glspec45.core.pdf s.8.14.2
	*/

	class PointSampler : public Sampler {
	public:

		PointSampler() = default;
		virtual ~PointSampler() = default;
		
		static Math::Vector4 sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords, unsigned int mipLevel);
		virtual Math::Vector4 sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords) const override;

	protected:
		PointSampler(const PointSampler&) = delete;
		PointSampler(PointSampler&&) = delete;
		PointSampler& operator=(const PointSampler&) = delete;
		PointSampler& operator=(PointSampler&&) = delete;
	};
}
#include "PointSamplerImpl.inl"
#endif
