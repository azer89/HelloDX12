#ifndef HELLO_DX12_DESCRIPTOR_HEAP
#define HELLO_DX12_DESCRIPTOR_HEAP

#include "DX12Context.h"
#include "DX12Descriptor.h"

#include <vector>

class DX12DescriptorHeap
{
public:
	void Destroy();
	void Create(DX12Context& ctx, uint32_t descriptorCount);
	void Create(DX12Context& ctx);

	void BindHeap(ID3D12GraphicsCommandList* commandList);
	void BindDescriptors(ID3D12GraphicsCommandList* commandList, uint32_t startRootParamIndex);

public:
	ID3D12DescriptorHeap* descriptorHeap_ = nullptr;
	std::vector<DX12Descriptor> descriptors_ = {};
};

#endif