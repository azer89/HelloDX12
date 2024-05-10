#include "DX12RootSignature.h"

void DX12RootSignature::Destroy()
{
	if (rootSignature_)
	{
		rootSignature_->Release();
		rootSignature_ = nullptr;
	}
}

// TODO Rename to CreateRootSignature()
void DX12RootSignature::Create(DX12Context& ctx,
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

void DX12RootSignature::Create(DX12Context& ctx,
	const D3D12_STATIC_SAMPLER_DESC& samplerDesc,
	const std::span<DX12Descriptor> descriptors,
	const std::span<DX12Descriptor> textureArrayDescriptors,
	uint32_t rootConstantCount,
	const D3D12_ROOT_SIGNATURE_FLAGS& rootSignatureFlags)
{
	uint32_t cvbRegister = 0;
	uint32_t srvRegister = 0;
	uint32_t uavRegister = 0;

	std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges = {};
	for (int i = 0; i < descriptors.size(); ++i)
	{
		if (descriptors[i].type_ == D3D12_DESCRIPTOR_RANGE_TYPE_CBV)
		{
			ranges.emplace_back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, cvbRegister++, 0, descriptors[i].rangeFlags_);
		}
		else if (descriptors[i].type_ == D3D12_DESCRIPTOR_RANGE_TYPE_SRV)
		{
			ranges.emplace_back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, srvRegister++, 0, descriptors[i].rangeFlags_);
		}
		else if (descriptors[i].type_ == D3D12_DESCRIPTOR_RANGE_TYPE_UAV)
		{
			ranges.emplace_back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, uavRegister++, 0, descriptors[i].rangeFlags_);
		}
	}

	// Bindless
	if (textureArrayDescriptors.size() > 0)
	{
		ranges.emplace_back().Init(
			D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 
			textureArrayDescriptors.size(),
			srvRegister++, 
			0, 
			textureArrayDescriptors[0].rangeFlags_, // TODO
			0
		);
	}

	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters = {};
	if (rootConstantCount > 0)
	{
		rootParameters.emplace_back().InitAsConstants(rootConstantCount, 0);
	}
	
	for (int i = 0; i < descriptors.size(); ++i)
	{
		rootParameters.emplace_back().InitAsDescriptorTable(1, ranges.data() + i, descriptors[i].shaderVisibility_);
	}

	if (textureArrayDescriptors.size() > 0)
	{
		rootParameters.emplace_back().InitAsDescriptorTable(
			1, 
			ranges.data() + descriptors.size(), 
			textureArrayDescriptors[0].shaderVisibility_);
	}

	Create(ctx, samplerDesc, rootParameters, rootSignatureFlags);
}