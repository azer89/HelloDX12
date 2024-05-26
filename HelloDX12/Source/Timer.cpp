#include "Timer.h"

#include <windows.h>
#include <winnt.h>
#include <wtypes.h>

#include <stdexcept>

inline void Win32Call(BOOL retVal)
{
	if (retVal == 0)
	{
		throw std::runtime_error("Error");
	}
}

void Timer::Init()
{
	// Query for the performance counter frequency
	LARGE_INTEGER largeInt;
	Win32Call(QueryPerformanceFrequency(&largeInt));
	frequency_ = largeInt.QuadPart;
	frequencyFloat_ = static_cast<float>(frequency_);

	Win32Call(QueryPerformanceCounter(&largeInt));
	startTime_ = largeInt.QuadPart;
	elapsed_ = largeInt.QuadPart - startTime_;
}

void Timer::Update()
{
	LARGE_INTEGER largeInt;
	Win32Call(QueryPerformanceCounter(&largeInt));
	uint32_t currentTime = largeInt.QuadPart - startTime_;
	delta_ = currentTime - elapsed_;
	deltaSecondsFloat_ = delta_ / frequencyFloat_;
	elapsed_ = currentTime;
	fpsCurr_ = 1.0f / deltaSecondsFloat_;
}