#ifndef SOFTRP_TEXTURE_UNIT_IMPL_INL_
#define SOFTRP_TEXTURE_UNIT_IMPL_INL_
#include "TextureUnit.h"
#include "LinearSampler.h"
#include "MipMapSampler.h"
#include <stdexcept>
namespace SoftRP {

	inline void TextureUnit::setTexture(Texture2D<Math::Vector4>* texture) {
		m_texture = texture;
	}

	inline void TextureUnit::setMagnificationSampler(Sampler* magnificationSampler) {
		m_magnificationSampler = magnificationSampler;
		updateSwitchOverPoint();
	}

	inline void TextureUnit::setMinificationSampler(Sampler* minificationSampler) {
		m_minificationSampler = minificationSampler;
		updateSwitchOverPoint();
	}

	inline void TextureUnit::updateSwitchOverPoint() {
		/*

		From OpenGL specs :

		Finally, there is the choice of c, the minification vs. magnification
		switch-over point. If the magnification filter is given by LINEAR and the minification filter
		is given by NEAREST_MIPMAP_NEAREST or NEAREST_MIPMAP_LINEAR, then c = 0.5.
		This is done to ensure that a minified texture
		does not appear ``sharper'' than a magnified texture. Otherwise c = 0.
		*/
		if (m_magnificationSampler && m_minificationSampler) {
			LinearSampler* magLinear = dynamic_cast<LinearSampler*>(m_magnificationSampler);
			if (magLinear) {
				//NEAREST_MIPMAP_NEAREST
				MipMapPointSampler* minMipMapNearest = dynamic_cast<MipMapPointSampler*>(m_minificationSampler);
				if (minMipMapNearest)
					m_minMagSwitchOverPoint = 0.5f;
				else {
					//NEAREST_MIPMAP_LINEAR
					AdjMipMapPointSampler* minAdjMipMapNearest = dynamic_cast<AdjMipMapPointSampler*>(m_minificationSampler);
					if (minAdjMipMapNearest)
						m_minMagSwitchOverPoint = 0.5f;
				}
			}
		}
		m_minMagSwitchOverPoint = 0.0f;
	}

	inline void TextureUnit::setAddressModeU() {
		throw std::runtime_error{ "Not implemented yet" };
	}

	inline void TextureUnit::setAddressModeV() {
		throw std::runtime_error{ "Not implemented yet" };
	}

	inline Math::Vector4 TextureUnit::sample(const Math::Vector2& textCoords) const {
		return m_magnificationSampler->sample(*m_texture, textCoords);
	}

	inline Math::Vector4 TextureUnit::sample(const Math::Vector2& textCoords, const Math::Vector2& dtcdx, const Math::Vector2& dtcdy) const {
		Sampler* sampler{ nullptr };
		float LOD = computeLOD(dtcdx, dtcdy);
		bool magnified = isMagnified(LOD);
		if (magnified)
			return m_magnificationSampler->sample(*m_texture, textCoords);
		else
			return m_minificationSampler->sample(*m_texture, textCoords, LOD);
	}

	inline float TextureUnit::computeLOD(const Math::Vector2& dtcdx, const Math::Vector2& dtcdy) const {
		const Math::Vector2 textureSize{ static_cast<float>(m_texture->width()), static_cast<float>(m_texture->height()) };
		const float squaredLen1 = (textureSize*dtcdx).squaredLength();
		const float squaredLen2 = (textureSize*dtcdy).squaredLength();
		return std::log2f(std::sqrtf(std::max(squaredLen1, squaredLen2)));
	}


	inline bool TextureUnit::isMagnified(float LOD)const {
		return LOD <= m_minMagSwitchOverPoint + std::numeric_limits<float>::epsilon();
	}

}
#endif
