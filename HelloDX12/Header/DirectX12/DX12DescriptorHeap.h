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
	void SetupHandles(DX12Context& ctx);

	void BindHeap(ID3D12GraphicsCommandList* commandList) const;
	void BindDescriptorsGraphics(ID3D12GraphicsCommandList* commandList, uint32_t startRootParamIndex) const;
	void BindDescriptorsCompute(ID3D12GraphicsCommandList* commandList, uint32_t startRootParamIndex) const;
	void BindSingleDescriptorCompute(
		ID3D12GraphicsCommandList* commandList, 
		uint32_t rootParamIndex,
		uint32_t descriptorIndex) const;

public:
	ID3D12DescriptorHeap* handle_{};
	std::vector<DX12Descriptor> descriptors_{};
	DX12DescriptorArray descriptorArray_{};
};

#endif