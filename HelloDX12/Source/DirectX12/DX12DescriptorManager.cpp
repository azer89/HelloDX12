#include "DX12DescriptorManager.h"

void DX12DescriptorManager::Destroy()
{
	if (rootSignature_)
	{
		rootSignature_->Release();
		rootSignature_ = nullptr;
	}

	if (descriptorHeap_)
	{
		descriptorHeap_->Release();
		descriptorHeap_ = nullptr;
	}
}

// TODO Rename to CreateRootSignature()
void DX12DescriptorManager::CreateRootSignature(DX12Context& ctx,
	const D3D12_STATIC_SAMPLER_DESC& samplerDesc,
	const std::span<CD3DX12_ROOT_PARAMETER1> rootParameters,
	const D3D12_ROOT_SIGNATURE_FLAGS& rootSignatureFlags)
{
	// Root signature
	ID3DBlob* signature = nullptr;
	ID3DBlob* error = nullptr;
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(
		static_cast<UINT>(rootParameters.size()),
		rootParameters.data(),
		1,
		&samplerDesc,
		rootSignatureFlags);

	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(ctx.GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(
		&rootSignatureDesc,
		featureData.HighestVersion,
		&signature,
		&error))
		ThrowIfFailed(ctx.GetDevice()->CreateRootSignature(
			0,
			signature->GetBufferPointer(),
			signature->GetBufferSize(),
			IID_PPV_ARGS(&rootSignature_)))

	signature->Release();
	if (error)
	{
		error->Release();
	}
}

void DX12DescriptorManager::CreateDescriptorHeap(DX12Context& ctx, uint32_t descriptorCount)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = descriptorCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};
	ctx.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap_));
}