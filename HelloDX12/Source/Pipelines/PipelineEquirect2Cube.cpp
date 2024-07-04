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
	descriptorHeap_.Destroy();
}

void PipelineEquirect2Cube::GenerateCubemapFromHDR(DX12Context& ctx, 
	DX12Image* hdrImage, 
	DX12Image* cubemapImage)
{
	CreateDescriptors(ctx, hdrImage, cubemapImage);
	GenerateShader(ctx);
	CreatePipeline(ctx);
	Execute(ctx, hdrImage, cubemapImage);
}

void PipelineEquirect2Cube::CreateDescriptors(
	DX12Context& ctx,
	DX12Image* hdrImage,
	DX12Image* cubemapImage)
{
	std::vector<DX12Descriptor> descriptors(2);
	descriptors[0] =
	{
		.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE,
		.shaderVisibility_ = D3D12_SHADER_VISIBILITY_ALL,
		.buffer_ = &(hdrImage->buffer_),
		.srvDescription_ = hdrImage->buffer_.GetSRVDescription()
	};

	descriptors[1] =
	{
		.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
		.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE,
		.shaderVisibility_ = D3D12_SHADER_VISIBILITY_ALL,
		.buffer_ = &(cubemapImage->buffer_),
		.uavDescription_ = cubemapImage->buffer_.GetUAVDescription(0)
	};

	descriptorHeap_.descriptors_ = descriptors;
	descriptorHeap_.Create(ctx);

	std::vector<CD3DX12_STATIC_SAMPLER_DESC> samplerArray{ { 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR } };
	samplerArray[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// Root signature
	rootSignature_.Create(ctx, samplerArray, descriptors, {}, 0, rootSignatureFlags);
}

void PipelineEquirect2Cube::GenerateShader(DX12Context& ctx)
{
	computeShader_.Create(ctx, AppConfig::ShaderFolder + "Equirect2Cube.hlsl", ShaderType::Compute);
}

void PipelineEquirect2Cube::CreatePipeline(DX12Context& ctx)
{
	// PSO
	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc =
	{
		.pRootSignature = rootSignature_.handle_,
	};

	// Set shader
	computeShader_.AddShader(psoDesc);

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
	commandList->SetComputeRootSignature(rootSignature_.handle_);

	cubemapImage->TransitionCommand(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	hdrImage->TransitionCommand(commandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	// Descriptors
	descriptorHeap_.BindHeap(commandList);
	descriptorHeap_.BindDescriptorsCompute(commandList, 0);

	commandList->Dispatch(cubemapImage->width_ / 32, cubemapImage->height_ / 32, 6);

	cubemapImage->TransitionCommand(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	hdrImage->TransitionCommand(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	ctx.SubmitCommandListAndWaitForGPU();
}