#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#else
#ifndef NDEBUG
#define NDEBUG
#endif
#endif
#include "demoLight.cpp" //include demo .cpp file here
#include <memory>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {	
	try {
		auto res = std::unique_ptr<SoftRPDemo::DemoApp>{ new SoftRPDemo::DemoApp{ hInstance } }.get()->run();		
		_CrtDumpMemoryLeaks();
		return res;
	} catch (std::exception& e) {
		MessageBox(0, (std::wstring{ L"An exception was thrown : " } + Utils::makeWideString(e.what())).c_str(), 0, 0);
	} catch (...) {
		MessageBox(0, L"An unknown exception was thrown!", 0, 0);
	}
	return -1;
}
