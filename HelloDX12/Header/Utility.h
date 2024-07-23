#ifndef HELLO_DX12_UTILITY
#define HELLO_DX12_UTILITY

#include <string>

namespace Utility
{
	std::wstring WStringConvert(std::string s);

	inline uint32_t MipMapCount(uint32_t w, uint32_t h)
	{
		int levels{ 1 };
		while ((w | h) >> levels)
		{
			levels += 1;
		}
		return levels;
	}

	inline uint32_t MipMapCount(uint32_t size)
	{
		int levels{ 1 };
		while (size >> levels)
		{
			levels += 1;
		}
		return levels;
	}
}

#endif