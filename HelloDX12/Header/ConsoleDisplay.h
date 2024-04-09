#ifndef HELLO_DX12_CONSOLE_DISPLAY
#define HELLO_DX12_CONSOLE_DISPLAY

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

namespace HDX12
{
	void ConsoleShow()
	{
		AllocConsole();
		HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		const int hConsole = _open_osfhandle(reinterpret_cast<intptr_t>(stdHandle), _O_TEXT);
		FILE* fp = _fdopen(hConsole, "w");
		freopen_s(&fp, "CONOUT$", "w", stdout);
	}
}

#endif