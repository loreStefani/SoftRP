#ifndef SOFTRP_TEXTURE_RENDER_TARGET_H_
#define SOFTRP_TEXTURE_RENDER_TARGET_H_
#include "RenderTarget.h"
#include "Texture2D.h"
#include "Vector.h"
#include <utility>
namespace SoftRP {

	/*
	Specialization of RenderTarget which uses a Texture2D to store rasterized data.
	*/

	class TextureRenderTarget :	public RenderTarget {
	public:
		
		//ctors
		TextureRenderTarget() = default;
		TextureRenderTarget(unsigned int width, unsigned int height);
		
		//dtor
		virtual ~TextureRenderTarget() = default;

		//copy
		TextureRenderTarget(const TextureRenderTarget&);
		TextureRenderTarget& operator=(const TextureRenderTarget&);
		//move
		TextureRenderTarget(TextureRenderTarget&&);		
		TextureRenderTarget& operator=(TextureRenderTarget&&);

		virtual unsigned int width() const override final;
		virtual unsigned int height() const override final;
		virtual void resize(unsigned int width, unsigned int height) override final;
						
		virtual Math::Vector4 get(unsigned int i, unsigned int j) override final;

		virtual void set(unsigned int i, unsigned int j, Math::Vector4 value) override final;
		
		virtual void clear(Math::Vector4 value) override final;

#ifdef SOFTRP_MULTI_THREAD
		virtual void clear(Math::Vector4 value, ThreadPool& threadPool) override final;
#endif
	private:
		Texture2D<Math::Vector4> m_texture{};
	};			
}
#include "TextureRenderTargetImpl.inl"
#endif

