#include "PipelineMipmap.h"
#include "DX12Exception.h"
#include "RootConstParam.h"

#include <vector>

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
	std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges = {};
	// Resource needs to be volatile
	ranges.emplace_back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
	ranges.emplace_back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
	
	uint32_t paramOffset = 0;
	constexpr uint32_t rootConstantCount = 2;
	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters= {};
	rootParameters.emplace_back().InitAsConstants(rootConstantCount, 0);
	rootParameters.emplace_back().InitAsDescriptorTable(1, ranges.data() + paramOffset++, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters.emplace_back().InitAsDescriptorTable(1, ranges.data() + paramOffset++, D3D12_SHADER_VISIBILITY_ALL);

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

	constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// Root signature
	rootSignature_.Create(ctx, samplerDesc, rootParameters, rootSignatureFlags);

	// PSO
	const D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc =
	{
		.pRootSignature = rootSignature_.rootSignature_,
		.CS = CD3DX12_SHADER_BYTECODE(computeShader_.GetHandle())
	};
	ctx.GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_));
}

void PipelineMipmap::GenerateMipmap(DX12Context& ctx, DX12Image* image)
{
	// Prepare the shader resource view description for the source texture
	D3D12_SHADER_RESOURCE_VIEW_DESC srvSrcDesc =
	{
		.Format = image->format_,
		.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	};
	srvSrcDesc.Texture2D.MipLevels = 1;

	// Prepare the unordered access view description for the destination texture
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDstDesc =
	{
		.Format = image->format_,
		.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D
	};

	// Descriptor heap
	// TODO Use DX12Descriptor::CreateDescriptorHeap(...)
	uint32_t requiredHeapSize = image->mipmapCount_ - 1;
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = 2 * requiredHeapSize, // Heap size
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};
	ID3D12DescriptorHeap* descriptorHeap;
	ctx.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));

	UINT incrementSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// CPU handles for generating SRV and UAV
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandleStart = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(cpuHandleStart, 0, incrementSize);

	// GPU handle for descriptor tables
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandleStart = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(gpuHandleStart, 0, incrementSize);

	// Start recording 
	ctx.ResetCommandList();
	auto commandList = ctx.GetCommandList();

	image->TransitionCommand(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandList->SetComputeRootSignature(rootSignature_.rootSignature_);
	commandList->SetPipelineState(pipelineState_);
	commandList->SetDescriptorHeaps(1, &descriptorHeap);

	for (uint32_t currMipLevel = 0; currMipLevel < image->mipmapCount_ - 1; ++currMipLevel)
	{
		// Mipmap dimensions
		uint32_t dstWidth = max(image->width_ >> (currMipLevel + 1), 1);
		uint32_t dstHeight = max(image->height_ >> (currMipLevel + 1), 1);

		// SRV for source texture
		srvSrcDesc.Texture2D.MostDetailedMip = currMipLevel;
		ctx.GetDevice()->CreateShaderResourceView(image->GetResource(), &srvSrcDesc, cpuHandle);
		cpuHandle.Offset(1, incrementSize); // Add offset

		// UAV for destination texture
		uavDstDesc.Texture2D.MipSlice = currMipLevel + 1;
		ctx.GetDevice()->CreateUnorderedAccessView(image->GetResource(), nullptr, &uavDstDesc, cpuHandle);
		cpuHandle.Offset(1, incrementSize); // Add offset

		uint32_t rootParamIndex = 0;
		
		// Root constants
		commandList->SetComputeRoot32BitConstant(
			rootParamIndex,
			RootConstParam(1.0f / static_cast<float>(dstWidth)).uint_,
			0);
		commandList->SetComputeRoot32BitConstant(
			rootParamIndex,
			RootConstParam(1.0f / static_cast<float>(dstHeight)).uint_,
			1);

		++rootParamIndex;

		// Pass the source and destination texture views to the shader via descriptor tables
		commandList->SetComputeRootDescriptorTable(rootParamIndex++, gpuHandle);
		gpuHandle.Offset(1, incrementSize); // Add offset
		commandList->SetComputeRootDescriptorTable(rootParamIndex++, gpuHandle);
		gpuHandle.Offset(1, incrementSize); // Add offset

		// Dispatch the compute shader with one thread per 8x8 pixels
		commandList->Dispatch(
			max(dstWidth / 8, 1u),
			max(dstHeight / 8, 1u),
			1);

		// Barrier
		image->UAVBarrier(commandList);
	}

	image->TransitionCommand(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	ctx.SubmitCommandListAndWaitForGPU();

	descriptorHeap->Release();
}