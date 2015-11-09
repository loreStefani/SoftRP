#ifndef SOFTRP_RASTERIZER_IMPL_INL_
#define SOFTRP_RASTERIZER_IMPL_INL_
#include "Rasterizer.h"
namespace SoftRP {

	inline void Rasterizer::setRenderTarget(RenderTarget* renderTarget) { m_renderTarget = renderTarget; }
	inline void Rasterizer::setViewPort(const ViewPort* viewPort) { m_viewPort = viewPort; }
	inline void Rasterizer::setDepthBuffer(DepthBuffer* depthBuffer) { m_depthBuffer = depthBuffer; }
	inline void Rasterizer::setPixelShader(const PixelShader* pixelShader) { m_pixelShader = pixelShader; }
	inline void Rasterizer::setShaderContext(const ShaderContext* shaderContext) { m_shaderContext = shaderContext; }

	inline RenderTarget* Rasterizer::renderTarget() const { return m_renderTarget; }
	inline const ViewPort* Rasterizer::viewPort() const { return m_viewPort; }
	inline DepthBuffer* Rasterizer::depthBuffer() const { return m_depthBuffer; }
	inline const PixelShader* Rasterizer::pixelShader() const { return m_pixelShader; }
	inline const ShaderContext* Rasterizer::shaderContext() const { return m_shaderContext; }

	inline bool Rasterizer::depthTest(unsigned int i, unsigned int j, float compare) {
		float currDepth = m_depthBuffer->get(i, j);
		if (currDepth > compare) {
			m_depthBuffer->set(i, j, compare);
			return true;
		}
		return false;
	}

	inline void Rasterizer::writePixel(unsigned int i, unsigned int j, Math::Vector4 data) {
		m_renderTarget->set(i, j, data);
	}
}
#endif
