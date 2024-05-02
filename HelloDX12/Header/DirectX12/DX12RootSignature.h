#ifndef HELLO_DX12_ROOT_SIGNATURE
#define HELLO_DX12_ROOT_SIGNATURE

#include "DX12Context.h"
#include "DX12Exception.h"

#include <span>

class DX12RootSignature
{
public:
	void Destroy();

	void Create(DX12Context& ctx,
		const D3D12_STATIC_SAMPLER_DESC& samplerDesc,
		const std::span<CD3DX12_ROOT_PARAMETER1> rootParameters,
		const D3D12_ROOT_SIGNATURE_FLAGS& rootSignatureFlags);

	//void CreateDescriptorHeap(DX12Context& ctx, uint32_t descriptorCount);

	//[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandleStart() const { return descriptorHeap_->GetCPUDescriptorHandleForHeapStart(); }
	//[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleStart() const { return descriptorHeap_->GetGPUDescriptorHandleForHeapStart(); }

public:
	ID3D12RootSignature* rootSignature_ = nullptr;
	//ID3D12DescriptorHeap* descriptorHeap_ = nullptr;
};

#endif
