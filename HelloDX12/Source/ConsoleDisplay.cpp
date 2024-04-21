#include "ConsoleDisplay.h"

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

void HDX12::ConsoleShow()
{
	if (AllocConsole())
	{
		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);
	}
}