#pragma once
#include "RenderTarget.h"
#include "Vector.h"
#include<stdexcept>
#include<cmath>
#include <Windows.h>


namespace SoftRPDemo {

	class GDIRenderTarget : public SoftRP::RenderTarget {
	public:

		//ctor
		GDIRenderTarget() = default;				
		//dtor
		virtual ~GDIRenderTarget();
		//copy
		GDIRenderTarget(const GDIRenderTarget&) = delete;
		GDIRenderTarget& operator=(const GDIRenderTarget&) = delete;
		//move
		GDIRenderTarget(GDIRenderTarget&&) = delete;
		GDIRenderTarget& operator=(GDIRenderTarget&&) = delete;
				
		virtual unsigned int width()const override;
		virtual unsigned int height()const override;

		virtual void resize(unsigned int width, unsigned int height) override;
		virtual void set(unsigned int i, unsigned int j, SoftRP::Math::Vector4 value) override;

		virtual SoftRP::Math::Vector4 get(unsigned int i, unsigned int j) override;

		virtual void clear(SoftRP::Math::Vector4 value) override;

#ifdef SOFTRP_MULTI_THREAD
		virtual void clear(SoftRP::Math::Vector4 value, SoftRP::ThreadPool& threadPool) override;
#endif

		void setWindowHandle(HWND hWnd);
		void present();

	private:

#ifdef SOFTRP_MULTI_THREAD
		void clearTask(BYTE blue, BYTE green, BYTE red, int height, int start);
#endif
		unsigned int getIndex(unsigned int i, unsigned int j) const;
		BYTE castChannel(float ch);
		float castChannel(BYTE ch);

		unsigned int m_width{ 0 };
		unsigned int m_height{ 0 };
		HWND m_hWnd{ 0 };
		HBITMAP m_hBitmap{ 0 };
		HGDIOBJ m_hBitmapOriginal{ 0 };
		BITMAPINFO m_BitmapInfo{};
		HDC m_hDC{ 0 };
		unsigned int m_padSize{ 0 };
		BYTE* m_pixelData{ nullptr };
	};


	inline unsigned int GDIRenderTarget::width() const{ return m_width; }
	inline unsigned int GDIRenderTarget::height() const { return m_height; }

	inline unsigned int GDIRenderTarget::getIndex(unsigned int i, unsigned int j) const {
		return (i*(3 * m_width + m_padSize) + j * 3)*sizeof(BYTE);
	}

	inline BYTE GDIRenderTarget::castChannel(float ch) {
		float fch = std::floor(ch*255.0f + 0.5f);
		fch = max(fch, 0.0f);
		fch = min(fch, 255.0f);
		return static_cast<BYTE>(fch);
	}

	inline float GDIRenderTarget::castChannel(BYTE ch) {
		return static_cast<float>(ch) / 255.0f;
	}


}


