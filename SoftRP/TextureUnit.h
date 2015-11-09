#ifndef SOFTRP_TEXTURE_UNIT_H_
#define SOFTRP_TEXTURE_UNIT_H_
#include "Texture2D.h"
#include "Sampler.h"
#include "Vector.h"
namespace SoftRP {

	/*
	Concrete data type which abstracts the operation of sampling a texture, from the texture and 
	the way it is sampled.
	*/

	class TextureUnit {
	public:
		//ctor
		TextureUnit() = default;
		//dtor
		~TextureUnit() = default;
		//copy
		TextureUnit(const TextureUnit&) = default;		
		TextureUnit& operator=(const TextureUnit&) = default;
		//move
		TextureUnit(TextureUnit&&) = default;
		TextureUnit& operator=(TextureUnit&&) = default;

		void setTexture(Texture2D<Math::Vector4>* texture);
		void setMagnificationSampler(Sampler* magnificationSampler);
		void setMinificationSampler(Sampler* minificationSampler);
		void setAddressModeU();
		void setAddressModeV();

		//sample the current Texture2D with the given texture coordinates
		Math::Vector4 sample(const Math::Vector2& textCoords) const;
		/*
		sample the current Texture2D with the given texture coordinates and their derivatives across a rasterized primitive.
		The derivatives are used to compute the level of detail parameter of the texture. This parameter determines 
		if the mapping of the texture on the (rasterized) primitive magnify or minify the texture image. Depending on this, 
		the correct Sampler (magnification sampler or minification sampler) is used.
		The current implementation refers to the OpenGL specs.
		*/
		Math::Vector4 sample(const Math::Vector2& textCoords, const Math::Vector2& dtcdx, const Math::Vector2& dtcdy) const;

	private:

		float computeLOD(const Math::Vector2& dtcdx, const Math::Vector2& dtcdy) const;		
		bool isMagnified(float LOD)const;
		void updateSwitchOverPoint();

		float m_minMagSwitchOverPoint{0.0f};
		Sampler* m_magnificationSampler;
		Sampler* m_minificationSampler;		
		Texture2D<Math::Vector4>* m_texture;
	};
}
#include "TextureUnitImpl.inl"
#endif
