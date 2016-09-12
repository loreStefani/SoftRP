#include "DemoLight.h"
#include <Windows.h>
#include "MainHelper.h"

using namespace SoftRPDemo;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	return runDemo<DemoApp>(hInstance);
}