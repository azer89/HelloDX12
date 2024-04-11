#include "Utility.h"

std::wstring Utility::WStringConvert(std::string s)
{
	std::wstring out;
	std::mbtowc(nullptr, 0, 0);
	int offset;
	size_t index = 0;
	for (wchar_t wc;
		(offset = std::mbtowc(&wc, &s[index], s.size() - index)) > 0;
		index += offset)
	{
		out.push_back(wc);
	}
	return out;
}