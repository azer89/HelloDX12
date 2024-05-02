#include "DX12DescriptorHeap.h"

void DX12DescriptorHeap::Destroy()
{
	if (descriptorHeap_)
	{
		descriptorHeap_->Release();
		descriptorHeap_ = nullptr;
	}
}

void DX12DescriptorHeap::Create(DX12Context& ctx, uint32_t descriptorCount)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = descriptorCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};
	ctx.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap_));
}