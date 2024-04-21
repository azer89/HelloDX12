#ifndef HELLO_DX12_RESOURCES_BASE
#define HELLO_DX12_RESOURCES_BASE

class ResourcesBase
{
public:
	explicit ResourcesBase() = default;
	virtual ~ResourcesBase() = default;

public:
	virtual void Destroy() = 0;
};

#endif