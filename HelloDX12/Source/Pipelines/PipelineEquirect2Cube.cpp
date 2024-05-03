#include "PipelineEquirect2Cube.h"

PipelineEquirect2Cube::PipelineEquirect2Cube(
	DX12Context& ctx) :
	PipelineBase(ctx)
{
}

PipelineEquirect2Cube::~PipelineEquirect2Cube()
{
	Destroy();
}

void PipelineEquirect2Cube::Destroy()
{
	descriptorHeap2_.Destroy();
}

void PipelineEquirect2Cube::GenerateCubemapFromHDR(DX12Context& ctx, 
	DX12Image* hdrImage, 
	DX12Image* cubemapImage,
	const D3D12_UNORDERED_ACCESS_VIEW_DESC& cubemapUAVDesc)
{
	CreateDescriptors(ctx, hdrImage, cubemapImage, cubemapUAVDesc);
	GenerateShader(ctx);
	CreatePipeline(ctx);
	Execute(ctx, hdrImage, cubemapImage);
}

void PipelineEquirect2Cube::CreateDescriptors(
	DX12Context& ctx,
	DX12Image* hdrImage,
	DX12Image* cubemapImage,
	const D3D12_UNORDERED_ACCESS_VIEW_DESC& cubemapUAVDesc)
{
	std::vector<DX12Descriptor> descriptors(2);
	descriptors[0] =
	{
		.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE,
		.shaderVisibility_ = D3D12_SHADER_VISIBILITY_ALL,
		.buffer_ = &(hdrImage->buffer_),
		.srvDescription_ = hdrImage->GetSRVDescription()
	};

	descriptors[1] =
	{
		.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
		.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE,
		.shaderVisibility_ = D3D12_SHADER_VISIBILITY_ALL,
		.buffer_ = &(cubemapImage->buffer_),
		.uavDescription_ = cubemapUAVDesc
	};

	descriptorHeap2_.descriptors_ = descriptors;
	descriptorHeap2_.Create(ctx);

	CD3DX12_STATIC_SAMPLER_DESC samplerDesc{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// Root signature
	rootSignature_.Create(ctx, samplerDesc, descriptors, 0, rootSignatureFlags);
}

void PipelineEquirect2Cube::CreateDescriptorHeap(DX12Context& ctx,
	DX12Image* hdrImage,
	DX12Image* cubemapImage,
	const D3D12_UNORDERED_ACCESS_VIEW_DESC& cubemapUAVDesc)
{
	uint32_t descriptorCount = 2;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvSrcDesc = hdrImage->GetSRVDescription();

	descriptorHeap_.Create(ctx, descriptorCount);

	UINT incrementSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle1(descriptorHeap_.descriptorHeap_->GetCPUDescriptorHandleForHeapStart(), 0, incrementSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle2(descriptorHeap_.descriptorHeap_->GetCPUDescriptorHandleForHeapStart(), 1, incrementSize);

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
	rootSignature_.Create(ctx, samplerDesc, rootParameters, rootSignatureFlags);
}

void PipelineEquirect2Cube::CreatePipeline(DX12Context& ctx)
{
	// PSO
	const D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc =
	{
		.pRootSignature = rootSignature_.rootSignature_,
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

	commandList->SetPipelineState(pipelineState_);
	commandList->SetComputeRootSignature(rootSignature_.rootSignature_);
	
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

	// Descriptors
	descriptorHeap2_.BindHeap(commandList);
	descriptorHeap2_.BindDescriptorsCompute(commandList, 0);

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