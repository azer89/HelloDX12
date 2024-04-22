#ifndef HELLO_DX12_ROOT_CONSTANT_PARAM
#define HELLO_DX12_ROOT_CONSTANT_PARAM

#include <cstdint>

// Union used for shader constants
struct RootConstParam
{
	RootConstParam(float f) : float_(f) {}
	RootConstParam(uint32_t u) : uint_(u) {}
	RootConstParam(int i) : int_(i) {}

	void operator= (float f) { float_ = f; }
	void operator= (uint32_t u) { uint_ = u; }
	void operator= (int i) { int_ = i; }

	union
	{
		float float_;
		uint32_t uint_;
		int int_;
	};
};

#endif