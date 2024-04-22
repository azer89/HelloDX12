#include "PipelineMipmap.h"
#include "RootConstParam.h"

PipelineMipmap::PipelineMipmap(
	DX12Context& ctx) :
	PipelineBase(ctx)
{
	GenerateShader(ctx);
	CreatePipeline(ctx);
}

void PipelineMipmap::GenerateShader(DX12Context& ctx)
{
	computeShader_.Create(ctx, AppConfig::ShaderFolder + "Mipmap.hlsl", ShaderType::Compute);
}

void PipelineMipmap::CreatePipeline(DX12Context& ctx)
{
	CD3DX12_DESCRIPTOR_RANGE srvCbvRanges[2] = {};
	CD3DX12_ROOT_PARAMETER rootParameters[3] = {};
	srvCbvRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
	srvCbvRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);
	rootParameters[0].InitAsConstants(2, 0);
	rootParameters[1].InitAsDescriptorTable(1, &srvCbvRanges[0]);
	rootParameters[2].InitAsDescriptorTable(1, &srvCbvRanges[1]);

	// Static sampler used to get the linearly interpolated color for the mipmaps
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
		.pRootSignature = rootSignature_.Get(),
		.CS = CD3DX12_SHADER_BYTECODE(computeShader_.GetHandle())
	};
	ctx.GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_));
}

void PipelineMipmap::GenerateMipmap(DX12Context& ctx, DX12Image* image)
{
	// Prepare the shader resource view description for the source texture
	D3D12_SHADER_RESOURCE_VIEW_DESC srvSrcDesc =
	{
		.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	};

	// Prepare the unordered access view description for the destination texture
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDstDesc =
	{
		.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D
	};

	// Create the descriptor heap with layout: source texture - destination texture
	uint32_t requiredHeapSize = image->mipmapCount_ - 1;
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = 2 * requiredHeapSize,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};
	ID3D12DescriptorHeap* descriptorHeap;
	ctx.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));

	UINT descriptorSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// CPU handle for the first descriptor on the descriptor heap, used to fill the heap
	CD3DX12_CPU_DESCRIPTOR_HANDLE currentCPUHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), 0, descriptorSize);

	// GPU handle for the first descriptor on the descriptor heap, used to initialize the descriptor tables
	CD3DX12_GPU_DESCRIPTOR_HANDLE currentGPUHandle(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), 0, descriptorSize);

	// Start recording 
	ctx.ResetCommandList();
	auto commandList = ctx.GetCommandList();

	commandList->SetComputeRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(pipelineState_.Get());
	commandList->SetDescriptorHeaps(1, &descriptorHeap);

	// Barrier
	auto barrier1 = 
		CD3DX12_RESOURCE_BARRIER::Transition(
			image->GetResource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandList->ResourceBarrier(1, &barrier1);

	//Loop through the mipmaps copying from the bigger mipmap to the smaller one with downsampling in a compute shader
	for (uint32_t currMipLevel = 0; currMipLevel < image->mipmapCount_ - 1; ++currMipLevel)
	{
		// Mipmap dimensions
		uint32_t dstWidth = max(image->width_ >> (currMipLevel + 1), 1);
		uint32_t dstHeight = max(image->height_ >> (currMipLevel + 1), 1);

		// SRV for source texture
		srvSrcDesc.Format = image->format_;
		srvSrcDesc.Texture2D.MipLevels = 1;
		srvSrcDesc.Texture2D.MostDetailedMip = currMipLevel;
		ctx.GetDevice()->CreateShaderResourceView(image->GetResource(), &srvSrcDesc, currentCPUHandle);
		currentCPUHandle.Offset(1, descriptorSize);

		// UAV for destination texture
		uavDstDesc.Format = image->format_;
		uavDstDesc.Texture2D.MipSlice = currMipLevel + 1;
		ctx.GetDevice()->CreateUnorderedAccessView(image->GetResource(), nullptr, &uavDstDesc, currentCPUHandle);
		currentCPUHandle.Offset(1, descriptorSize);

		// Root constants
		commandList->SetComputeRoot32BitConstant(0, RootConstParam(1.0f / static_cast<float>(dstWidth)).uint_, 0);
		commandList->SetComputeRoot32BitConstant(0, RootConstParam(1.0f / static_cast<float>(dstHeight)).uint_, 1);

		// Pass the source and destination texture views to the shader via descriptor tables
		commandList->SetComputeRootDescriptorTable(1, currentGPUHandle);
		currentGPUHandle.Offset(1, descriptorSize);
		commandList->SetComputeRootDescriptorTable(2, currentGPUHandle);
		currentGPUHandle.Offset(1, descriptorSize);

		// Dispatch the compute shader with one thread per 8x8 pixels
		commandList->Dispatch(max(dstWidth / 8, 1u), max(dstHeight / 8, 1u), 1);

		// Barrier
		auto barrier2 = CD3DX12_RESOURCE_BARRIER::UAV(image->GetResource());
		commandList->ResourceBarrier(1, &barrier2);
	}

	// Barrier
	auto barrier3 = CD3DX12_RESOURCE_BARRIER::Transition(
		image->GetResource(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &barrier3);

	ctx.SubmitCommandListAndWaitForGPU();
}