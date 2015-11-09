#ifndef SOFTRP_LINEAR_SAMPLER_IMPL_INL_
#define SOFTRP_LINEAR_SAMPLER_IMPL_INL_
#include "LinearSampler.h"
namespace SoftRP {

	inline Math::Vector4 LinearSampler::sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords, unsigned int mipLevel) {
		const unsigned int width = texture.mipLevelWidth(mipLevel);
		const unsigned int height = texture.mipLevelHeight(mipLevel);
				
		const float u = textCoords[0] * width -0.5f;
		const float v = textCoords[1] * height -0.5f;
				
		const float floorU = std::floorf(u);
		const float floorV = std::floorf(v);

		const float fracU = u - floorU;
		const float fracV = v - floorV;

		const unsigned int maxX = width - 1;
		const unsigned int maxY = height - 1;
		
		const unsigned int x0 = std::min(std::max(static_cast<unsigned int>(floorU), static_cast<unsigned int>(0)), maxX);
		const unsigned int y0 = std::min(std::max(static_cast<unsigned int>(floorV), static_cast<unsigned int>(0)), maxY);
		const unsigned int x1 = std::min(x0 + 1, maxX);
		const unsigned int y1 = std::min(y0 + 1, maxY);

		const auto& texel1 = texture.get(y0, x0, mipLevel);
		const auto& texel2 = texture.get(y0, x1, mipLevel);
		const auto& texel3 = texture.get(y1, x0, mipLevel);
		const auto& texel4 = texture.get(y1, x1, mipLevel);
				
		auto texelX0 = texel1;
		texelX0.lerp(fracV, texel3);

		auto texelX1 = texel2;
		texelX1.lerp(fracV, texel4);

		texelX0.lerp(fracU, texelX1);

		return texelX0;
	}

	inline Math::Vector4 LinearSampler::sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords)const {
		return sample(texture, textCoords, static_cast<unsigned int>(0));
	}
}
#endif