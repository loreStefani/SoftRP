#include "GDIRenderTarget.h"

using namespace SoftRPDemo;

void GDIRenderTarget::setWindowHandle(HWND hWnd) {
	m_hWnd = hWnd;
}

GDIRenderTarget::~GDIRenderTarget() {
	if (m_hBitmap) {
		SelectObject(m_hDC, m_hBitmapOriginal);
		DeleteObject(m_hBitmap);
		DeleteDC(m_hDC);
		m_hBitmapOriginal = 0;
		m_hBitmap = 0;
		m_hDC = 0;
	}
	m_hWnd = 0;
}

void GDIRenderTarget::resize(unsigned int width, unsigned int height){

	if (!m_hWnd)
		throw std::runtime_error{ "Invalid Window handle" };

	if (m_hBitmap) {
		SelectObject(m_hDC, m_hBitmapOriginal);
		DeleteObject(m_hBitmap);
		DeleteDC(m_hDC);
	}

	auto hDC = GetDC(m_hWnd);
	m_hDC = CreateCompatibleDC(hDC);
	ReleaseDC(m_hWnd, hDC);

	m_BitmapInfo = {};
	BITMAPINFOHEADER& bmiHeader = m_BitmapInfo.bmiHeader;

	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biPlanes = 1;

	bmiHeader.biWidth = static_cast<LONG>(width);
	//a height negative value identifies a top-down bitmap (the origin is placed in the upper-left corner)
	bmiHeader.biHeight = -static_cast<LONG>(height); 

	bmiHeader.biBitCount = 24;
	bmiHeader.biCompression = BI_RGB;

	bmiHeader.biXPelsPerMeter = 0;//unused
	bmiHeader.biYPelsPerMeter = 0;//unused

	bmiHeader.biSizeImage = 0; //ok for BI_RGB
	bmiHeader.biClrUsed = 0;
	bmiHeader.biClrImportant = 0;

	m_hBitmap = CreateDIBSection(m_hDC, &m_BitmapInfo, DIB_RGB_COLORS, reinterpret_cast<void**>(&m_pixelData), nullptr, 0);
	if (!m_hBitmap)
		throw std::runtime_error{ "CreateDIBSection" };

	const unsigned int scanlineSize = width * 3;
	//scanlines are LONG-aligned		
	m_padSize = ((scanlineSize + (sizeof(LONG) - 1)) & ~(sizeof(LONG) - 1)) - scanlineSize;

	m_hBitmapOriginal = SelectObject(m_hDC, m_hBitmap);

	m_width = width;
	m_height = height;
}

void GDIRenderTarget::set(unsigned int i, unsigned int j, SoftRP::Math::Vector4 value){
	unsigned int index = getIndex(i, j);
	m_pixelData[index++] = castChannel(value[2]);
	m_pixelData[index++] = castChannel(value[1]);
	m_pixelData[index++] = castChannel(value[0]);
	//the implementation ignores alpha components
}

SoftRP::Math::Vector4 GDIRenderTarget::get(unsigned int i, unsigned int j){
	unsigned int index = getIndex(i, j);
	return SoftRP::Math::Vector4{
		static_cast<float>(castChannel(m_pixelData[index])),
		static_cast<float>(castChannel(m_pixelData[index + 1])),
		static_cast<float>(castChannel(m_pixelData[index + 2])),
		1.0f
	};
}

void GDIRenderTarget::clear(SoftRP::Math::Vector4 value) {

	const BYTE blue = castChannel(value[2]);
	const BYTE green = castChannel(value[1]);
	const BYTE red = castChannel(value[0]);
	unsigned int k = 0;
	for (unsigned int i = 0; i < m_height; i++) {
		for (unsigned int j = 0; j < m_width; j++) {
			m_pixelData[k++] = blue;
			m_pixelData[k++] = green;
			m_pixelData[k++] = red;
		}
		k += m_padSize;
	}
}

#ifdef SOFTRP_MULTI_THREAD
void GDIRenderTarget::clear(SoftRP::Math::Vector4 value, SoftRP::ThreadPool& threadPool){
	const BYTE blue = castChannel(value[2]);
	const BYTE green = castChannel(value[1]);
	const BYTE red = castChannel(value[0]);
	constexpr int tileHeight = 256; //clear tiles size : m_width x tileHeigth
	const int tilesCount = static_cast<int>(std::ceil(static_cast<float>(m_height) / static_cast<float>(tileHeight)));
	int tileSize = (3 * m_width + m_padSize) *sizeof(BYTE)*tileHeight;
	int start = 0;
	for (int i = 0; i < tilesCount - 1; i++, start += tileSize)
		threadPool.addTask([this, blue, green, red, tileHeight, start]() {
			clearTask(blue, green, red, tileHeight, start);
		});

	const int remainingHeight = m_height - tileHeight*(tilesCount - 1);
	threadPool.addTask([this, blue, green, red, remainingHeight, start]() {
		clearTask(blue, green, red, remainingHeight, start);
	});
}

void GDIRenderTarget::clearTask(BYTE blue, BYTE green, BYTE red, int height, int start) {
	for (int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < m_width; j++) {
			m_pixelData[start++] = blue;
			m_pixelData[start++] = green;
			m_pixelData[start++] = red;
		}
		start += m_padSize;
	}
}
#endif

void GDIRenderTarget::present() {
	HDC hDC = GetDC(m_hWnd);

	/*if(SetDIBitsToDevice(hDC, 0, 0, getWidth(), getHeight(), 0, 0, 0, getHeight(), m_pixelData, &m_BitmapInfo, DIB_RGB_COLORS) == 0)
	throw std::runtime_error{ "SetDIBBitsToDevice" };*/
	GdiFlush();
	if (!BitBlt(hDC, 0, 0, static_cast<int>(m_width), static_cast<int>(m_height), m_hDC, 0, 0, SRCCOPY))
		throw std::runtime_error{ "BitBlt" };

	ReleaseDC(m_hWnd, hDC);
}

