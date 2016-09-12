#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#include <memory>

namespace SoftRPDemo
{
	template<typename DemoType, typename... Args>
	inline int runDemo(Args... args)
	{
#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif	

		int res = -1;
		try
		{
			int demoRes = std::make_unique<DemoType>(std::forward<Args>(args)...)->run();
			res = demoRes;
		}
		catch (std::exception& e)
		{
			MessageBox(0, (std::wstring{ L"An exception was thrown : " } +WindowsDemo::Utils::makeWideString(e.what())).c_str(), 0, MB_ICONERROR | MB_DEFAULT_DESKTOP_ONLY);
		}
		catch (...)
		{
			MessageBox(0, L"An unknown exception was thrown!", 0, MB_ICONERROR | MB_DEFAULT_DESKTOP_ONLY);
		}

		return res;
	}
}
