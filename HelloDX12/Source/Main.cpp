#include "AppSimple.h"
#include "ConsoleDisplay.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	HDX12::ConsoleShow();
	AppSimple sample(1280, 720, L"Hello DirectX 12");
	return Win32Application::Run(&sample, hInstance, nCmdShow);
}