#ifndef SOFTRP_LINEAR_SAMPLER_H_
#define SOFTRP_LINEAR_SAMPLER_H_
#include "Sampler.h"
namespace SoftRP {

	/*
	Sampler specialization which implements the OpenGL's LINEAR.
	see : https://www.opengl.org/registry/doc/glspec45.core.pdf s.8.14.2
	*/

	class LinearSampler : public Sampler {
	public:
		LinearSampler() = default;
		virtual ~LinearSampler() = default;
		static Math::Vector4 sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords, unsigned int mipLevel);
		virtual Math::Vector4 sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords) const override;
	protected:
		LinearSampler(const LinearSampler&) = delete;
		LinearSampler(LinearSampler&&) = delete;
		LinearSampler& operator=(const LinearSampler&) = delete;
		LinearSampler& operator=(LinearSampler&&) = delete;
	};
}
#include "LinearSamplerImpl.inl"
#endif
