
#include <windows.h>

#include <fcntl.h>
#include <io.h>

#include <iostream>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	AllocConsole();
	HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	int hConsole = _open_osfhandle(reinterpret_cast<intptr_t>(stdHandle), _O_TEXT);
	FILE* fp = _fdopen(hConsole, "w");
	freopen_s(&fp, "CONOUT$", "w", stdout);

	std::cout << "Hello World\n";

	return 0;
}
