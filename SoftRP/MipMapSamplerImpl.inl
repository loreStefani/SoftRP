#ifndef SOFTRP_MIPMAP_SAMPLER_IMPL_INL_
#define SOFTRP_MIPMAP_SAMPLER_IMPL_INL_
#include "MipMapSampler.h"
namespace SoftRP {

	/*  MipMapSampler implementation  */

	template<typename InMipMapSampler>
	inline Math::Vector4 MipMapSampler<InMipMapSampler>::sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords) const {
		return InMipMapSampler::sample(texture, textCoords, 0);
	}

	template<typename InMipMapSampler>
	inline Math::Vector4 MipMapSampler<InMipMapSampler>::sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords, float LOD) const {
		const unsigned int mipMapLevel = std::min(static_cast<unsigned int>(std::ceilf(LOD + 0.5f)) - 1, texture.maxMipLevel());
		return InMipMapSampler::sample(texture, textCoords, mipMapLevel);
	}

	/*  AdjMipMapSampler implementation  */

	template<typename InMipMapSampler>
	inline Math::Vector4 AdjMipMapSampler<InMipMapSampler>::sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords)const {
		return InMipMapSampler::sample(texture, textCoords, static_cast<unsigned int>(0));
	}

	template<typename InMipMapSampler>
	inline Math::Vector4 AdjMipMapSampler<InMipMapSampler>::sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords, float LOD) const {
		const unsigned int maxMipLevel = texture.maxMipLevel();
		const unsigned int mipMapLevel1 = std::min(static_cast<unsigned int>(std::floor(LOD)), maxMipLevel);
		const unsigned int mipMapLevel2 = std::min(mipMapLevel1 + 1, maxMipLevel);
		
		if (mipMapLevel1 == mipMapLevel2)
			return InMipMapSampler::sample(texture, textCoords, mipMapLevel1);

		Math::Vector4 v0 = InMipMapSampler::sample(texture, textCoords, mipMapLevel1);
		Math::Vector4 v1 = InMipMapSampler::sample(texture, textCoords, mipMapLevel2);

		return v0.lerp(LOD - std::floorf(LOD), v1);
	}
}
#endif
