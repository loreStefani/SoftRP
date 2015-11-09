#ifndef SOFTRP_POINT_SAMPLER_IMPL_INL_
#define SOFTRP_POINT_SAMPLER_IMPL_INL_
#include "PointSampler.h"
namespace SoftRP {
	inline Math::Vector4 PointSampler::sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords, unsigned int mipLevel) {
		const unsigned int width = texture.mipLevelWidth(mipLevel);
		const unsigned int height = texture.mipLevelHeight(mipLevel);
		float u = std::floorf(textCoords[0] * width);
		float v = std::floorf(textCoords[1] * height);
		unsigned int i = std::min(static_cast<unsigned int>(std::max(u, 0.0f)), width - 1);
		unsigned int j = std::min(static_cast<unsigned int>(std::max(v, 0.0f)), height - 1);
		return texture.get(j, i, mipLevel);
	}

	inline Math::Vector4 PointSampler::sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords)const {
		return sample(texture, textCoords, static_cast<unsigned int>(0));
	}
}
#endif