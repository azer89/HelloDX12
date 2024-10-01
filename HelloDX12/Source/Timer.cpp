#include "Timer.h"

#include <windows.h>
#include <winnt.h>

#include <stdexcept>
#include <algorithm>

inline void Win32Call(BOOL retVal)
{
	if (retVal == 0)
	{
		throw std::runtime_error("Error");
	}
}

void Timer::Init()
{
	LARGE_INTEGER largeInt{};
	Win32Call(QueryPerformanceFrequency(&largeInt));
	frequency_ = largeInt.QuadPart;
	frequencyFloat_ = static_cast<float>(frequency_);

	Win32Call(QueryPerformanceCounter(&largeInt));
	startTime_ = largeInt.QuadPart;
	elapsed_ = largeInt.QuadPart - startTime_;

	dataForGraph_.resize(LENGTH_FOR_GRAPH, 0);
}

void Timer::Update()
{
	LARGE_INTEGER largeInt{};
	Win32Call(QueryPerformanceCounter(&largeInt));
	const uint32_t currentTime = static_cast<uint32_t>(largeInt.QuadPart - startTime_);
	delta_ = currentTime - elapsed_;
	deltaSecondsFloat_ = delta_ / frequencyFloat_;
	elapsed_ = currentTime;
	fpsCurrent_ = 1.0f / deltaSecondsFloat_;

	graphTimer_ += deltaSecondsFloat_;
	if (graphTimer_ >= GRAPH_DELAY)
	{
		fpsDelayed_ = 1.0f / deltaSecondsFloat_;
		deltaDelayed_ = deltaSecondsFloat_ * 1000.f;
		std::ranges::rotate(dataForGraph_.begin(), dataForGraph_.begin() + 1, dataForGraph_.end());
		dataForGraph_[LENGTH_FOR_GRAPH - 1] = fpsDelayed_;
		graphTimer_ = 0.0f;
	}
}