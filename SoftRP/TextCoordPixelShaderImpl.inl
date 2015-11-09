#ifndef SOFTRP_TEXT_COORD_PIXEL_SHADER_IMPL_INL_
#define SOFTRP_TEXT_COORD_PIXEL_SHADER_IMPL_INL_
#include "TextCoordPixelShader.h"
namespace SoftRP {
	inline void TextCoordPixelShader::operator() (const ShaderContext& sc,
												  const PSExecutionContext& psec,
												  size_t instance, Math::Vector4* out) const {
		Math::Vector4 textCoordDerivatives[4];
		computeDDXDDY(psec, 1, textCoordDerivatives);
		const TextureUnit& textureUnit = *sc.textureUnits()[0];
		for (unsigned int i = 0; i < 4; i++) {
			if ((psec.mask & (1 << i)) == 0)
				continue;
			const Math::Vector2& textCoords = *Math::vectorFromPtr<2>(psec.interpolated[i].getField(1));
			const Math::Vector4* dtcdx = textCoordDerivatives + getDDXIndex(i);
			const Math::Vector4* dtcdy = textCoordDerivatives + getDDYIndex(i);
			out[i] = textureUnit.sample(textCoords, *Math::vectorFromPtr<2, 4>(dtcdx), *Math::vectorFromPtr<2, 4>(dtcdy));
		}
	}

}
#endif
