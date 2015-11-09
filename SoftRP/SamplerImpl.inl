#ifndef SOFTRP_SAMPLER_IMPL_INL_
#define SOFTRP_SAMPLER_IMPL_INL_
#include "Sampler.h"
namespace SoftRP {
	inline Math::Vector4 Sampler::sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords, float LOD) const {
		return sample(texture, textCoords);
	}
}
#endif
