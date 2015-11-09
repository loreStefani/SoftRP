#ifndef SOFTRP_SAMPLER_H_
#define SOFTRP_SAMPLER_H_
#include "Texture2D.h"
#include "Vector.h"
namespace SoftRP {

	/*
	Abstract data type which represents a texture sampling technique.
	*/

	class Sampler {
	public:
		Sampler() = default;
		virtual ~Sampler() = default;
		//sample a texture with the given texture coordinates
		virtual Math::Vector4 sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords) const= 0;
		/*
		sample a texture with the given texture coordinates and the texture level of detail. 
		the default implementation ignores the latter.
		*/
		virtual Math::Vector4 sample(const Texture2D<Math::Vector4>& texture, const Math::Vector2& textCoords, float LOD) const;
	protected:
		Sampler(const Sampler&) = delete;
		Sampler(Sampler&&) = delete;
		Sampler& operator=(const Sampler&) = delete;
		Sampler& operator=(Sampler&&) = delete;
	};
}
#include "SamplerImpl.inl"
#endif
