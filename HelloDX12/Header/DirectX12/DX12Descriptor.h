#ifndef HELLO_DX12_DESCRIPTOR
#define HELLO_DX12_DESCRIPTOR

#include "DX12Context.h"

#include "DX12Exception.h"
#include <span>

class DX12Descriptor
{
public:
	void Destroy();

	void CreateRootDescriptor(DX12Context& ctx,
		const D3D12_STATIC_SAMPLER_DESC& samplerDesc,
		const std::span<CD3DX12_ROOT_PARAMETER1> rootParameters,
		const D3D12_ROOT_SIGNATURE_FLAGS& rootSignatureFlags);

	[[nodiscard]] ID3D12RootSignature* GetRootSignature() const { return rootSignature_; };

private:
	ID3D12RootSignature* rootSignature_ = nullptr;
};

#endif
