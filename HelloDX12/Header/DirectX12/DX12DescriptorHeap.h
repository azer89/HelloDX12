#ifndef HELLO_DX12_DESCRIPTOR_HEAP
#define HELLO_DX12_DESCRIPTOR_HEAP

#include "DX12Context.h"

class DX12DescriptorHeap
{
public:
	void Destroy();
	void Create(DX12Context& ctx, uint32_t descriptorCount);

public:
	ID3D12DescriptorHeap* descriptorHeap_ = nullptr;
};

#endif