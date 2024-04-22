#ifndef HELLO_DX12_UTILITY
#define HELLO_DX12_UTILITY

#include <string>

namespace Utility
{
	std::wstring WStringConvert(std::string s);

	inline int MipMapCount(int w, int h)
	{
		int levels = 1;
		while ((w | h) >> levels)
		{
			levels += 1;
		}
		return levels;
	}

	inline int MipMapCount(int size)
	{
		int levels = 1;
		while (size >> levels)
		{
			levels += 1;
		}
		return levels;
	}
}

#endif