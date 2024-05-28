#include "AppSimple.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	AppSimple sample{};
	return Win32Application::Run(&sample, hInstance, nCmdShow);
}