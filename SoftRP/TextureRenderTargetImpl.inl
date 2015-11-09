#ifndef SOFTRP_TEXTURE_RENDER_TARGET_IMPL_INL_
#define SOFTRP_TEXTURE_RENDER_TARGET_IMPL_INL_
#include "TextureRenderTarget.h"
namespace SoftRP {

	inline TextureRenderTarget::TextureRenderTarget(unsigned int width, unsigned int height)
		: m_texture{ width, height } {}

	inline TextureRenderTarget::TextureRenderTarget(const TextureRenderTarget& rt) : m_texture{ rt.m_texture } {}

	inline TextureRenderTarget& TextureRenderTarget::operator=(const TextureRenderTarget& rt) {
		m_texture = rt.m_texture;
	}

	inline TextureRenderTarget::TextureRenderTarget(TextureRenderTarget&& rt) : m_texture{ std::move(rt.m_texture) } {}

	inline TextureRenderTarget& TextureRenderTarget::operator=(TextureRenderTarget&& rt) {
		m_texture = std::move(rt.m_texture);
	}

	inline unsigned int TextureRenderTarget::width() const{
		return m_texture.width();
	}

	inline unsigned int TextureRenderTarget::height() const{
		return m_texture.height();
	}

	inline void TextureRenderTarget::resize(unsigned int width, unsigned int height) {
		m_texture.resize(width, height);
	}

	inline void TextureRenderTarget::set(unsigned int i, unsigned int j, Math::Vector4 value) {
		m_texture.set(i, j, value);
	}

	inline Math::Vector4 TextureRenderTarget::get(unsigned int i, unsigned int j) {
		return m_texture.get(i, j);
	}

	inline void TextureRenderTarget::clear(Math::Vector4 value) {
		m_texture.clear(value);
	}

#ifdef SOFTRP_MULTI_THREAD
	inline void TextureRenderTarget::clear(Math::Vector4 value, ThreadPool& threadPool) {
		m_texture.clear(value, threadPool);
	}
#endif

}
#endif
