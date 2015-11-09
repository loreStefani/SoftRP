#include "DemoAppBase.h"

using namespace SoftRP;
using namespace Math;
using namespace WindowsDemo;

namespace SoftRPDemo {
	class DemoApp : public DemoAppBase {

	public:

		explicit DemoApp(HINSTANCE hInstance, unsigned int width = defaultWindowSize(), unsigned int height = defaultWindowSize()) :
			DemoAppBase(hInstance, width, height),
			m_texture{ loadTexture("Resources/lambertian.jpg") } {

			Mesh m{ MeshFactory::createFromObj<true>(L"Resources/LeePerrySmith.obj") };

			VertexBuffer* vertexBuffer;
			IndexBuffer* indexBuffer;
			m_inputVertexLayout = m.fillBuffers(&vertexBuffer, &indexBuffer);
			m_vertexBuffer.reset(vertexBuffer);
			m_indexBuffer.reset(indexBuffer);

			m_indexCount = m.indexCount();

			m_outputVertexLayout = OutputVertexLayout::create(1);

			m_renderer.setRenderTargetClearValue(Math::Vector4{ 0.0f, 0.0f, 0.0f, 1.0f });
			m_renderer.setDepthBufferClearValue(1.0f);

			m_texture.generateMipMaps();
						
			m_textureUnit0.setTexture(&m_texture);			
			m_textureUnit0.setMinificationSampler(&m_minSampler);
			m_textureUnit0.setMagnificationSampler(&m_magSampler);

			m_renderer.setTextureUnit(0, &m_textureUnit0);
			m_renderer.setConstantBuffer(0, &m_constantBuffer0);
		}

		virtual ~DemoApp() = default;

		virtual void renderFrame(long long deltaTime) override {

			m_renderer.clearRenderTarget();
			m_renderer.clearDepthBuffer();
			
			*(m_constantBuffer0.getField(0).asMatrix4()) = m_camera.projView();

			PipelineState pipelineState{ m_inputVertexLayout, m_vertexShader, m_outputVertexLayout, m_pixelShader };
			m_renderer.setPipelineState(&pipelineState);

			m_renderer.setVertexBuffer(m_vertexBuffer.get());
			m_renderer.setIndexBuffer(m_indexBuffer.get());
			m_renderer.setDepthBuffer(&m_depthBuffer);
			m_renderer.setRenderTarget(&m_renderTarget);
			m_renderer.setViewPort(&m_viewPort);
			
			m_renderer.drawIndexed(m_indexCount);

			m_renderer.wait();

			m_renderTarget.present();
		}

	private:
		size_t m_indexCount;
		TextCoordVertexShader m_vertexShader{};
		TextCoordPixelShader m_pixelShader{};
		std::unique_ptr<VertexBuffer> m_vertexBuffer{ nullptr };
		std::unique_ptr<IndexBuffer> m_indexBuffer{ nullptr };
		LinearSampler m_magSampler{};
		AdjMipMapLinearSampler m_minSampler{};
		ConstantBuffer m_constantBuffer0{ std::vector<size_t>{16} }; //projViewWorld
		TextureUnit m_textureUnit0{};
		Texture2D<Math::Vector4> m_texture;
	};
}