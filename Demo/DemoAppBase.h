#pragma once
#include "SoftRP.h"
#include "Includes.h"
#include "SingleWindowApp.h"
#include<string>
#include<chrono>
#include<Windows.h>

namespace SoftRPDemo {
	class DemoAppBase;
}

namespace WindowsDemo {
	namespace Utils {
		namespace TypeTraits {
			template<>
			struct IsBaseOf<SingleWindowApp<SoftRPDemo::DemoAppBase>, SoftRPDemo::DemoAppBase> : std::true_type {};
		}
	}
}

namespace SoftRPDemo {	

	class DemoAppBase : public WindowsDemo::SingleWindowApp<DemoAppBase> {
	public:

		//ctor
		explicit DemoAppBase(HINSTANCE hInstance, unsigned int width = defaultWindowSize(), unsigned int height = defaultWindowSize());

		//dtor
		virtual ~DemoAppBase() = default;

		//copy
		DemoAppBase(const DemoAppBase&) = delete;
		DemoAppBase& operator=(const DemoAppBase&) = delete;

		//move
		DemoAppBase(DemoAppBase&&) = delete;
		DemoAppBase& operator=(DemoAppBase&&) = delete;
						
		virtual void onUpdate() override;
		virtual void onResize() override;
		virtual bool onMouseDown(MouseButton mouseButton, MousePos mousePos) override;
		virtual bool onMouseUp(MouseButton mouseButton, MousePos mousePos) override;
		virtual bool onMouseMove(MousePos mousePos)override;

		virtual void renderFrame(long long deltaTime) = 0;
		
	protected:

		void updateView();
		void setWindowText(const std::wstring& text);
		void setWindowText(std::wstring&& text);

		bool m_mouseCaptured{ false };
		bool m_leftButtonDown{ true };
		unsigned int m_frameCount{ 0 };
		std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTime{};
		std::chrono::milliseconds m_millis{ 0 };
		std::wstring m_fpsString{};
		std::wstring m_messageString{};
		float m_radius{ 1.0f };
		float m_minRadius{ 0.1f };
		float m_maxRadius{ 5.0f };
		float m_theta{ 0.0f };
		float m_phi{ PI / 2.0f };
		MousePos m_lastMousePos{};
		SoftRP::Math::Vector3 m_lookAt{};
		Camera m_camera{};
		GDIRenderTarget m_renderTarget;
		SoftRP::Texture2D<float> m_depthBuffer;
		SoftRP::ViewPort m_viewPort;
		//Here, order of declarations matters. VertexLayout manages Vertex allocations
		SoftRP::InputVertexLayout m_inputVertexLayout;
		SoftRP::OutputVertexLayout m_outputVertexLayout;
		SoftRP::SHClipperFactory m_clipperFactory{};
		SoftRP::BinRasterizerFactory m_rasterizerFactory{};
		SoftRP::Renderer m_renderer{m_clipperFactory,  m_rasterizerFactory};
	};

}