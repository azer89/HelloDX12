#include "PipelineEquirect2Cube.h"

PipelineEquirect2Cube::PipelineEquirect2Cube(
	DX12Context& ctx) :
	PipelineBase(ctx)
{
}

void PipelineEquirect2Cube::GenerateCubemapFromHDR(DX12Context& ctx, 
	DX12Image* hdrImage, 
	DX12Image* cubemapImage,
	const D3D12_UNORDERED_ACCESS_VIEW_DESC& cubemapUAVDesc)
{
	CreateDescriptorHeap(ctx, hdrImage, cubemapImage, cubemapUAVDesc);
	GenerateShader(ctx);
	CreateRootSignature(ctx);
	CreatePipeline(ctx);
	Execute(ctx, hdrImage, cubemapImage);
}

void PipelineEquirect2Cube::CreateDescriptorHeap(DX12Context& ctx,
	DX12Image* hdrImage,
	DX12Image* cubemapImage,
	const D3D12_UNORDERED_ACCESS_VIEW_DESC& cubemapUAVDesc)
{
	uint32_t descriptorCount = 2;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvSrcDesc = hdrImage->GetSRVDescription();

	descriptorManager_.CreateDescriptorHeap(ctx, descriptorCount);

	UINT incrementSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle1(descriptorManager_.descriptorHeap_->GetCPUDescriptorHandleForHeapStart(), 0, incrementSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle2(descriptorManager_.descriptorHeap_->GetCPUDescriptorHandleForHeapStart(), 1, incrementSize);

	ctx.GetDevice()->CreateShaderResourceView(hdrImage->GetResource(), &srvSrcDesc, handle1);
	ctx.GetDevice()->CreateUnorderedAccessView(cubemapImage->GetResource(), nullptr, &cubemapUAVDesc, handle2);
}

void PipelineEquirect2Cube::GenerateShader(DX12Context& ctx)
{
	computeShader_.Create(ctx, AppConfig::ShaderFolder + "Equirect2Cube.hlsl", ShaderType::Compute);
}

void PipelineEquirect2Cube::CreateRootSignature(DX12Context& ctx)
{
	std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges = {};
	ranges.emplace_back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
	ranges.emplace_back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

	uint32_t paramOffset = 0;
	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters = {};
	rootParameters.emplace_back().InitAsDescriptorTable(1, ranges.data() + paramOffset++, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters.emplace_back().InitAsDescriptorTable(1, ranges.data() + paramOffset++, D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_STATIC_SAMPLER_DESC samplerDesc{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// Root signature
	descriptorManager_.CreateRootDescriptor(ctx, samplerDesc, rootParameters, rootSignatureFlags);
}

void PipelineEquirect2Cube::CreatePipeline(DX12Context& ctx)
{
	// PSO
	const D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc =
	{
		.pRootSignature = descriptorManager_.rootSignature_,
		.CS = CD3DX12_SHADER_BYTECODE(computeShader_.GetHandle())
	};
	ctx.GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_));
}

void PipelineEquirect2Cube::Execute(
	DX12Context& ctx,
	DX12Image* hdrImage,
	DX12Image* cubemapImage)
{
	// Start recording 
	ctx.ResetCommandList();
	auto commandList = ctx.GetCommandList();

	commandList->SetComputeRootSignature(descriptorManager_.rootSignature_);
	commandList->SetPipelineState(pipelineState_);
	commandList->SetDescriptorHeaps(1, &(descriptorManager_.descriptorHeap_));

	// Barrier
	auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(cubemapImage->buffer_.resource_, 
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandList->ResourceBarrier(1, &barrier1);

	// Barrier
	auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(hdrImage->buffer_.resource_,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &barrier2);

	const uint32_t incrementSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const CD3DX12_GPU_DESCRIPTOR_HANDLE handle1(descriptorManager_.descriptorHeap_->GetGPUDescriptorHandleForHeapStart(), 0, incrementSize);
	const CD3DX12_GPU_DESCRIPTOR_HANDLE handle2(descriptorManager_.descriptorHeap_->GetGPUDescriptorHandleForHeapStart(), 1, incrementSize);

	uint32_t rootParamIndex = 0;
	commandList->SetComputeRootDescriptorTable(rootParamIndex++, handle1);
	commandList->SetComputeRootDescriptorTable(rootParamIndex++, handle2);

	commandList->Dispatch(cubemapImage->width_ / 32, cubemapImage->height_ / 32, 6);

	// Barrier
	auto barrier3 = CD3DX12_RESOURCE_BARRIER::Transition(cubemapImage->buffer_.resource_,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &barrier3);

	// Barrier
	auto barrier4 = CD3DX12_RESOURCE_BARRIER::Transition(hdrImage->buffer_.resource_,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &barrier4);

	ctx.SubmitCommandListAndWaitForGPU();
}