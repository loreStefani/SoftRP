#include "DemoAppBase.h"

using namespace SoftRPDemo;

DemoAppBase::DemoAppBase(HINSTANCE hInstance, unsigned int width, unsigned int height)
		:SingleWindowApp(hInstance, width, height), m_viewPort{ getWidth(), getHeight() },
			m_renderTarget{}, m_depthBuffer{ getWidth(), getHeight() },
			m_inputVertexLayout{ SoftRP::InputVertexLayout::create(std::vector<size_t>{}) },
			m_outputVertexLayout{ SoftRP::OutputVertexLayout::create(0) } {
		
	m_lastTime = std::chrono::high_resolution_clock::now();
	updateView();
}

void DemoAppBase::onUpdate(){

	const auto now = std::chrono::high_resolution_clock::now();
	auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastTime);

	renderFrame(delta.count());

	m_lastTime = now;
	m_millis += delta;
	m_frameCount++;

	std::wstring delta_str = std::to_wstring(delta.count());

	if (m_millis >= std::chrono::milliseconds{ 1000 }) {
		m_fpsString = std::to_wstring(m_frameCount);
		m_frameCount = 1;
		m_millis = std::chrono::milliseconds{ 0 };
	}
	SetWindowText(getWindowHandle(), 
				  (delta_str + std::wstring{ L" ms, " } + m_fpsString + std::wstring{ L" FPS" } + m_messageString).c_str());
}

void DemoAppBase::onResize(){

	unsigned int width = getWidth();
	unsigned int height = getHeight();

	m_viewPort = SoftRP::ViewPort{ width, height };

	m_depthBuffer.resize(width, height);

	m_camera.makePerspective(static_cast<float>(width) / static_cast<float>(height));

	m_renderTarget.setWindowHandle(getWindowHandle());
	m_renderTarget.resize(getWidth(), getHeight());
}

bool DemoAppBase::onMouseDown(MouseButton mouseButton, MousePos mousePos){
	if (mouseButton == MouseButton::MIDDLE)
		return false;
	SetCapture(getWindowHandle());
	m_lastMousePos.mouseX = mousePos.mouseX;
	m_lastMousePos.mouseY = mousePos.mouseY;
	m_mouseCaptured = true;
	m_leftButtonDown = mouseButton == MouseButton::LEFT;
	return true;
}

bool DemoAppBase::onMouseUp(MouseButton mouseButton, MousePos mousePos) {
	if (mouseButton == MouseButton::MIDDLE)
		return false;
	ReleaseCapture();
	m_mouseCaptured = false;
	return true;
}

bool DemoAppBase::onMouseMove(MousePos mousePos){
	if (!m_mouseCaptured)
		return false;

	float deltaX = degsToRadians(static_cast<float>(mousePos.mouseX - m_lastMousePos.mouseX));
	float deltaY = degsToRadians(static_cast<float>(mousePos.mouseY - m_lastMousePos.mouseY));
	m_lastMousePos.mouseX = mousePos.mouseX;
	m_lastMousePos.mouseY = mousePos.mouseY;

	if (m_leftButtonDown) {
		m_theta += deltaX* 0.25f;
		m_phi += deltaY*0.25f;
	} else {
		m_radius += 0.05f  * deltaX;
	}
	updateView();

	return true;
}

void DemoAppBase::updateView() {

	m_phi = max(m_phi, 0.5f);
	m_phi = min(m_phi, PI - 0.5f);

	m_radius = max(m_radius, m_minRadius);
	m_radius = min(m_radius, m_maxRadius);

	const auto cosTheta = std::cos(m_theta);
	const auto sinTheta = std::sin(m_theta);
	const auto cosPhi = std::cos(m_phi);
	const auto sinPhi = std::sin(m_phi);
	const float x = m_radius*cosTheta*sinPhi;
	const float z = m_radius*sinTheta*sinPhi;
	const float y = m_radius*cosPhi;
	m_camera.lookAt(SoftRP::Math::Vector3{ x, y, z }, m_lookAt);
}

void DemoAppBase::setWindowText(const std::wstring& text) {
	m_messageString = std::wstring{L", "} + text;
}

void DemoAppBase::setWindowText(std::wstring&& text) {
	m_messageString = std::wstring{ L", " } + std::move(text);
}