#ifndef HELLO_DX12_TIMER
#define HELLO_DX12_TIMER

#include <cstdint>

class Timer
{
public:
	void Init();
	void Update();

	[[nodiscard]] float GetDelta() const { return deltaSecondsFloat_; }

private:
	int64_t startTime_;
	int64_t elapsed_;
	int64_t delta_;
	int64_t deltaSeconds_;
	float deltaSecondsFloat_;

	int64_t frequency_;
	float frequencyFloat_;
};

#endif