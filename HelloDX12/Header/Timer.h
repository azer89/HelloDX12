#ifndef HELLO_DX12_TIMER
#define HELLO_DX12_TIMER

#include <cstdint>
#include <vector>

class Timer
{
public:
	void Init();
	void Update();

	[[nodiscard]] float GetDelta() const { return deltaSecondsFloat_; }
	[[nodiscard]] float GetFPS() const { return fpsCurrent_; }
	[[nodiscard]] float GetFPSDelayed() const { return fpsDelayed_; }
	[[nodiscard]] float GetDeltaDelayed() const { return deltaDelayed_; }

	[[nodiscard]] const float* GetGraph() const { return dataForGraph_.data(); }
	static int GetGraphLength() { return LENGTH_FOR_GRAPH; }

private:
	int64_t startTime_ = 0;
	int64_t elapsed_ = 0;

	int64_t delta_ = 0;
	float deltaSecondsFloat_ = 0.0f;
	float deltaDelayed_ = 0.0f; // TODO Set as smoothed

	int64_t frequency_ = 0;
	float frequencyFloat_ = 0.0f;

	float fpsCurrent_ = 0.0f;
	float fpsDelayed_ = 0.0f;  // TODO Set as smoothed

	std::vector<float> dataForGraph_ = {};
	float graphTimer_= 0.0f;

	static constexpr size_t LENGTH_FOR_GRAPH = 100;
	static constexpr float GRAPH_DELAY = 0.1f;
};

#endif