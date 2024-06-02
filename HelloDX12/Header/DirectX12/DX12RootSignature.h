#ifndef HELLO_DX12_ROOT_SIGNATURE
#define HELLO_DX12_ROOT_SIGNATURE

#include "DX12Context.h"
#include "DX12Exception.h"
#include "DX12Descriptor.h"

#include <span>

class DX12RootSignature
{
public:
	void Destroy();

	void Create(DX12Context& ctx,
		const std::span<CD3DX12_STATIC_SAMPLER_DESC> samplerDescArray,
		const std::span<CD3DX12_ROOT_PARAMETER1> rootParameters,
		const D3D12_ROOT_SIGNATURE_FLAGS& rootSignatureFlags);

	void Create(DX12Context& ctx,
		const std::span<CD3DX12_STATIC_SAMPLER_DESC> samplerDescArray,
		const std::span<DX12Descriptor> descriptors,
		const DX12DescriptorArray& descriptorArray,
		uint32_t rootConstantCount,
		const D3D12_ROOT_SIGNATURE_FLAGS& rootSignatureFlags);

public:
	ID3D12RootSignature* handle_{};
};

#endif
