#pragma once
#include<string>
#include<exception>
#include<Windows.h>
#include<Windowsx.h>
#include"Utils.h"

namespace WindowsDemo {

	struct InitWindowException : public std::exception {
		using std::exception::exception;
	};

	template<typename Derived>
	class SingleWindowApp {

		static_assert(Utils::TypeTraits::isBaseOf<SingleWindowApp, Derived>(), "Template argument must derive from SingleWindowApp.");

	public:

		virtual ~SingleWindowApp() {}

		//Run the application, i.e. perform initialization and enter the main loop
		virtual int run() final;

	protected:

		static constexpr unsigned int defaultWindowSize();

		explicit SingleWindowApp(HINSTANCE hInstance, unsigned int width = defaultWindowSize(), unsigned int height = defaultWindowSize(),
								 std::wstring windowName = L"Window", std::wstring className = L"Default Window Class");

		//avoid copy
		SingleWindowApp(const SingleWindowApp&) = delete;
		SingleWindowApp& operator=(const SingleWindowApp&) = delete;
		//allow move
		SingleWindowApp(SingleWindowApp&&) = default;
		SingleWindowApp& operator=(SingleWindowApp&&) = default;

		HWND getWindowHandle()const;
		unsigned int getWidth()const;
		unsigned int getHeight() const;
		bool isPaused() const;
		bool isResizing()const;

		//handle window's messages
		virtual LRESULT handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) final;

		//mouse events, return true if consume the event and false otherwise
		enum class MouseButton : char {
			LEFT, RIGHT, MIDDLE
		};

		struct MousePos {
			short mouseX;
			short mouseY;
		};

		virtual bool onMouseDown(MouseButton mouseButton, MousePos mousePos);
		virtual bool onMouseUp(MouseButton mouseButton, MousePos mousePos);
		virtual bool onMouseMove(MousePos mousePos);

		//keyboard events, return true if consume the event and false otherwise
		virtual bool onKeyDown(WPARAM keyCode);
		virtual bool onKeyUp(WPARAM keyCode);

		//resize events
		virtual void onResize();

		//Perform initialization tasks.
		virtual void onStart();
		//Perform application tasks.
		virtual void onUpdate();
		//Perform cleanup tasks.
		virtual void onStop();

	private:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		bool m_paused{ false };
		unsigned int m_width{ 0 };
		unsigned int m_height{ 0 };
		HWND m_hWindow{ 0 };

		enum class WindowSizeState : char {
			NONE,
			MINIMIZED,
			MAXIMIZED,
			RESIZING
		};
		WindowSizeState m_windowSizeState{ WindowSizeState::NONE };
	};

	template<typename Derived>
	inline constexpr unsigned int SingleWindowApp<Derived>::defaultWindowSize() {
		return CW_USEDEFAULT;
	}

	template<typename Derived>
	inline SingleWindowApp<Derived>::SingleWindowApp(HINSTANCE hInstance, unsigned int width, unsigned int height, std::wstring windowName, std::wstring className)
		:m_width{ width }, m_height{ height } {
		WNDCLASSEX wndClass{};
		wndClass.cbSize = sizeof(WNDCLASSEX);
		wndClass.hInstance = hInstance;
		wndClass.lpfnWndProc = WndProc;
		wndClass.lpszClassName = className.c_str();
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.style = CS_HREDRAW | CS_VREDRAW;

		//defaults
		//wndClass.hIcon = 0;
		//wndClass.cbClsExtra = 0;
		//wndClass.cbWndExtra = 0;
		//wndClass.hbrBackground = 0;
		//wndClass.hIconSm = 0;
		//wndClass.lpszMenuName = 0;

		ATOM registeredClass = RegisterClassEx(&wndClass);

		if (registeredClass == 0)
			throw InitWindowException{ "RegisterClassEx" };

		m_hWindow = CreateWindowEx(
			0,
			className.c_str(),
			windowName.c_str(),
			//WS_VISIBLE | WS_OVERLAPPEDWINDOW,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			static_cast<int>(width),
			static_cast<int>(height),
			0,
			0,
			hInstance,
			this);

		if (m_hWindow == NULL)
			throw InitWindowException{ "CreateWindowEx" };

		RECT windowRect{};
		if (!GetWindowRect(m_hWindow, static_cast<LPRECT>(&windowRect)))
			throw InitWindowException{ "GetWindowRect" };

		m_width = static_cast<unsigned int>(windowRect.right - windowRect.left);
		m_height = static_cast<unsigned int>(windowRect.bottom - windowRect.top);
	}

	template<typename Derived>
	inline void WindowsDemo::SingleWindowApp<Derived>::onStart() {
		ShowWindow(m_hWindow, SW_SHOW);
	}

	template<typename Derived>
	inline int WindowsDemo::SingleWindowApp<Derived>::run() {
		MSG msg{};

		try {

			onStart();

			while (true) {

				if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);

					if (msg.message == WM_QUIT)
						break;
				} else if (!m_paused)
					onUpdate();
			}

			onStop();

		} catch (std::exception& e) {
			MessageBox(0, (std::wstring{ L"An exception was thrown : " } +Utils::makeWideString(e.what())).c_str(), 0, 0);
		} catch (...) {
			MessageBox(0, L"An unknown exception was thrown!", 0, 0);
		}

		return static_cast<char>(msg.wParam);
	}

	template<typename Derived>
	inline LRESULT CALLBACK SingleWindowApp<Derived>::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

		if (uMsg == WM_DESTROY) {
			//Send WM_QUIT; this means that as soon as a window is destroyed, the application should terminate.
			PostQuitMessage(0);
			return 0;
		}

		Derived *pWindow = nullptr;

		if (uMsg == WM_NCCREATE) {
			CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
			pWindow = static_cast<Derived*>(pCreate->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
			//Very important, the CreateWindowEx function sends WM_NCCREATE, WM_NCCALCSIZE, and WM_CREATE messages 
			//to the window being created. When WM_CREATE is sent, pWindow is valid and the handleMessage method is called.
			//if m_hWindow is not set here, it could be used as a (invalid) handle during the handleMessage execution.
			pWindow->m_hWindow = hWnd;
		} else
			pWindow = reinterpret_cast<Derived*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		if (pWindow)
			return pWindow->handleMessage(uMsg, wParam, lParam);
		else
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	template<typename Derived>
	inline LRESULT SingleWindowApp<Derived>::handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
		case WM_LBUTTONDOWN:
			if (onMouseDown(MouseButton::LEFT, MousePos{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) }))
				return 0;
			break;
		case WM_LBUTTONUP:
			if (onMouseUp(MouseButton::LEFT, MousePos{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) }))
				return 0;
			break;
		case WM_RBUTTONDOWN:
			if (onMouseDown(MouseButton::RIGHT, MousePos{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) }))
				return 0;
			break;
		case WM_RBUTTONUP:
			if (onMouseUp(MouseButton::RIGHT, MousePos{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) }))
				return 0;
			break;
		case WM_MBUTTONDOWN:
			if (onMouseDown(MouseButton::MIDDLE, MousePos{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) }))
				return 0;
			break;
		case WM_MBUTTONUP:
			if (onMouseUp(MouseButton::MIDDLE, MousePos{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) }))
				return 0;
			break;
		case WM_MOUSEMOVE:
			if (onMouseMove(MousePos{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) }))
				return 0;
			break;
		case WM_KEYDOWN:
			if (onKeyDown(wParam))
				return 0;
			break;
		case WM_KEYUP:
			if (onKeyUp(wParam))
				return 0;
			break;
		case WM_SIZE:

			m_width = LOWORD(lParam);
			m_height = HIWORD(lParam);

			if (wParam == SIZE_MINIMIZED) {
				m_paused = true;
				m_windowSizeState = WindowSizeState::MINIMIZED;
			} else if (wParam == SIZE_MAXIMIZED) {
				m_paused = false;
				m_windowSizeState = WindowSizeState::MAXIMIZED;
				onResize();
			} else if (wParam == SIZE_RESTORED) {

				switch (m_windowSizeState) {
				case WindowSizeState::MINIMIZED:
				case WindowSizeState::MAXIMIZED:
					m_paused = false;
					m_windowSizeState = WindowSizeState::NONE;
					onResize();
					break;
				case WindowSizeState::RESIZING:
					break;
				case WindowSizeState::NONE:
					onResize();
					break;
				}
			}

			return 0;
		case WM_ENTERSIZEMOVE:
			m_windowSizeState = WindowSizeState::RESIZING;
			m_paused = true;
			return 0;
		case WM_EXITSIZEMOVE:
			m_windowSizeState = WindowSizeState::RESIZING;
			m_paused = false;
			onResize();
			return 0;
		}


		return DefWindowProc(getWindowHandle(), uMsg, wParam, lParam);
	}

	template<typename Derived>
	inline HWND SingleWindowApp<Derived>::getWindowHandle()const { return m_hWindow; }

	template<typename Derived>
	inline unsigned int SingleWindowApp<Derived>::getWidth()const { return m_width; }

	template<typename Derived>
	inline unsigned int SingleWindowApp<Derived>::getHeight() const { return m_height; }

	template<typename Derived>
	inline bool SingleWindowApp<Derived>::isPaused() const { return m_paused; }

	template<typename Derived>
	inline bool SingleWindowApp<Derived>::isResizing() const { return m_windowSizeState == WindowSizeState::RESIZING; }

	template<typename Derived>
	inline bool SingleWindowApp<Derived>::onMouseDown(MouseButton mouseButton, MousePos mousePos) { return false; }

	template<typename Derived>
	inline bool SingleWindowApp<Derived>::onMouseUp(MouseButton mouseButton, MousePos mousePos) { return false; }

	template<typename Derived>
	inline bool SingleWindowApp<Derived>::onMouseMove(MousePos mousePos) { return false; }

	template<typename Derived>
	inline bool SingleWindowApp<Derived>::onKeyDown(WPARAM keyCode) { return false; }

	template<typename Derived>
	inline bool SingleWindowApp<Derived>::onKeyUp(WPARAM keyCode) { return false; }

	template<typename Derived>
	inline void SingleWindowApp<Derived>::onResize() {}

	template<typename Derived>
	inline void SingleWindowApp<Derived>::onUpdate() {}

	template<typename Derived>
	inline void SingleWindowApp<Derived>::onStop() {}

}
