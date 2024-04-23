#include "PipelineTonemap.h"

PipelineTonemap::PipelineTonemap(
	DX12Context& ctx,
	ResourcesShared* resourcesShared) :
	PipelineBase(ctx),
	resourcesShared_(resourcesShared)
{
}

void PipelineTonemap::CreatePipeline(DX12Context& ctx)
{
	CD3DX12_DESCRIPTOR_RANGE srvCbvRanges[2] = {};
	srvCbvRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
	srvCbvRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[2] = {};
	rootParameters[0].InitAsDescriptorTable(1, &srvCbvRanges[0]);
	rootParameters[1].InitAsDescriptorTable(1, &srvCbvRanges[1]);

	D3D12_STATIC_SAMPLER_DESC samplerDesc =
	{
		.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
		.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		.MipLODBias = 0.0f,
		.MaxAnisotropy = 0,
		.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
		.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK,
		.MinLOD = 0.0f,
		.MaxLOD = D3D12_FLOAT32_MAX,
		.ShaderRegister = 0,
		.RegisterSpace = 0,
		.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
	};

	// Create the root signature for the mipmap compute shader from the parameters and sampler above
	ID3DBlob* signature;
	ID3DBlob* error;
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	ctx.GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));

	// Create pipeline state object for the compute shader using the root signature.
	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc =
	{
		.pRootSignature = rootSignature_,
		.CS = CD3DX12_SHADER_BYTECODE(computeShader_.GetHandle())
	};
	ctx.GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_));

	signature->Release();
	if (error)
	{
		error->Release();
	}

	// TODO handles can be precomputed
	uint32_t incrementSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle1(descriptorHeap_->GetCPUDescriptorHandleForHeapStart(), 0, incrementSize); // Texture
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle2(descriptorHeap_->GetCPUDescriptorHandleForHeapStart(), 1, incrementSize); // Lights

	// Source image SRV
	auto srcSRVDesc = resourcesShared_->GetOffscreenSRVDescription();
	auto srcResource = resourcesShared_->GetOffscreenResource();
	ctx.GetDevice()->CreateShaderResourceView(srcResource, &srcSRVDesc, handle1);

	// Destination image UAV
	auto dstUAVDesc = resourcesShared_->GetSwapchainUAVDescription(ctx);
	auto dstResource = resourcesShared_->GetSwapchainRenderTarget(ctx.GetFrameIndex());
	ctx.GetDevice()->CreateUnorderedAccessView(dstResource, nullptr, &dstUAVDesc, handle2);
}

void PipelineTonemap::GenerateShader(DX12Context& ctx)
{
	computeShader_.Create(ctx, AppConfig::ShaderFolder + "Tonemap.hlsl", ShaderType::Compute);
}

void PipelineTonemap::PopulateCommandList(DX12Context& ctx)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = 2,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};
	ID3D12DescriptorHeap* descriptorHeap;
	ctx.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));

	ID3D12GraphicsCommandList* commandList = ctx.GetCommandList();
}