#ifndef HELLO_DX12_RESOURCES_BASE
#define HELLO_DX12_RESOURCES_BASE

#include "DX12Context.h"

class ResourcesBase
{
public:
	explicit ResourcesBase() = default;
	virtual ~ResourcesBase() = default;

	virtual void OnWindowResize(DX12Context& ctx, uint32_t width, uint32_t height)
	{
	}

public:
	virtual void Destroy() = 0;
};

#endif