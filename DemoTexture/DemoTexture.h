#pragma once
#include <DemoAppBase.h>

namespace SoftRPDemo
{
	using namespace SoftRP;
	using namespace Math;

	class DemoApp : public DemoAppBase
	{

	public:
		explicit DemoApp(HINSTANCE hInstance, unsigned int width = defaultWindowSize(), unsigned int height = defaultWindowSize()) :
			DemoAppBase(hInstance, width, height), m_texture{ loadTexture("Resources/crate.gif") }
		{

			Mesh m{ MeshFactory::createCube<true>() };

			VertexBuffer* vertexBuffer;
			IndexBuffer* indexBuffer;
			m_inputVertexLayout = m.fillBuffers(&vertexBuffer, &indexBuffer);
			m_vertexBuffer.reset(vertexBuffer);
			m_indexBuffer.reset(indexBuffer);

			m_indexCount = m.indexCount();

			m_outputVertexLayout = OutputVertexLayout::create(1);//position, textCoord

			m_texture.generateMipMaps();

			m_textureUnit0.setTexture(&m_texture);

			updateMagSampler();
			updateMinSampler();

			m_renderer.setTextureUnit(0, &m_textureUnit0);
			m_renderer.setConstantBuffer(0, &m_constantBuffer0);

			m_renderer.setRenderTargetClearValue(Math::Vector4{ 0.0f, 0.0f, 0.0f, 1.0f });
			m_renderer.setDepthBufferClearValue(1.0f);
		}

		virtual ~DemoApp() = default;

		virtual bool onKeyUp(WPARAM keyCode) override
		{

			switch (keyCode)
			{
			case VK_NUMPAD4:
				m_currMagSampler = (m_currMagSampler + 1) % m_magSamplerCount;
				updateMagSampler();
				return true;
			case VK_NUMPAD6:
				m_currMinSampler = (m_currMinSampler + 1) % m_minSamplerCount;
				updateMinSampler();
				return true;
			default:
				return DemoAppBase::onKeyUp(keyCode);
			}
		}

		virtual void renderFrame(long long deltaTime) override
		{

			m_renderer.clearDepthBuffer();
			m_renderer.clearRenderTarget();

			*(m_constantBuffer0.getField(0).asMatrix4()) = m_camera.projView();

			PipelineState pipelineState{ m_inputVertexLayout, m_vertexShader, m_outputVertexLayout, m_pixelShader };
			m_renderer.setPipelineState(&pipelineState);

			m_renderer.setVertexBuffer(m_vertexBuffer.get());
			m_renderer.setIndexBuffer(m_indexBuffer.get());
			m_renderer.setViewPort(&m_viewPort);
			m_renderer.setDepthBuffer(&m_depthBuffer);
			m_renderer.setRenderTarget(&m_renderTarget);

			m_renderer.drawIndexed(m_indexCount);

			setWindowText(m_magSamplerMessage + std::wstring{ L",  " } +m_minSamplerMessage + std::wstring{ L",  " } +m_useString);

			m_renderer.wait();

			m_renderTarget.present();
		}

	private:

		void updateMagSampler()
		{
			switch (m_currMagSampler)
			{
			case 0:
				m_magSampler.reset(new PointSampler{});
				m_magSamplerMessage = L"Magnification Sampler : PointSampler";
				break;
			case 1:
				m_magSampler.reset(new LinearSampler{});
				m_magSamplerMessage = L"Magnification Sampler : LinearSampler";
				break;
			default:
				throw std::runtime_error{ "An unexpected error happend" };
			}
			m_textureUnit0.setMagnificationSampler(m_magSampler.get());
		}

		void updateMinSampler()
		{
			switch (m_currMinSampler)
			{
			case 0:
				m_minSampler.reset(new MipMapPointSampler{});
				m_minSamplerMessage = L"Minification Sampler : MipMapPointSampler";
				break;
			case 1:
				m_minSampler.reset(new MipMapLinearSampler{});
				m_minSamplerMessage = L"Minification Sampler : MipMapLinearSampler";
				break;
			case 2:
				m_minSampler.reset(new AdjMipMapPointSampler{});
				m_minSamplerMessage = L"Minification Sampler : AdjMipMapPointSampler";
				break;
			case 3:
				m_minSampler.reset(new AdjMipMapLinearSampler{});
				m_minSamplerMessage = L"Minification Sampler : AdjMipMapLinearSampler";
				break;
			default:
				throw std::runtime_error{ "An unexpected error happend" };
			}
			m_textureUnit0.setMinificationSampler(m_minSampler.get());
		}

		unsigned char m_currMagSampler{ 0 };
		unsigned char m_currMinSampler{ 0 };
		const unsigned char m_magSamplerCount{ 2 };
		const unsigned char m_minSamplerCount{ 4 };
		size_t m_indexCount;
		TextCoordVertexShader m_vertexShader{};
		TextCoordPixelShader m_pixelShader{};
		std::unique_ptr<VertexBuffer> m_vertexBuffer{ nullptr };
		std::unique_ptr<IndexBuffer> m_indexBuffer{ nullptr };
		ConstantBuffer m_constantBuffer0{ std::vector<size_t>{16} }; //projViewWorld	
		std::unique_ptr<Sampler> m_magSampler{ nullptr };
		std::unique_ptr<Sampler> m_minSampler{ nullptr };
		TextureUnit m_textureUnit0{};
		Texture2D<Math::Vector4> m_texture;
		std::wstring m_minSamplerMessage{};
		std::wstring m_magSamplerMessage{};
		std::wstring m_useString{ L"Use NUMPAD4/NUMPAD6 to change Magnification/Minification Sampler" };
	};
}
