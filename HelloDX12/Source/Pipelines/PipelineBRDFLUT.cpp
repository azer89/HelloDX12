#include "PipelineBRDFLUT.h"

#include <algorithm>

PipelineBRDFLUT::PipelineBRDFLUT(
	DX12Context& ctx) :
	PipelineBase(ctx)
{
}

PipelineBRDFLUT::~PipelineBRDFLUT()
{
	Destroy();
}

void PipelineBRDFLUT::Destroy()
{
	descriptorHeap_.Destroy();
}

void PipelineBRDFLUT::Execute(DX12Context& ctx,
	DX12Image* lut)
{
	CreateDescriptors(ctx, lut);
	GenerateShader(ctx);
	CreatePipeline(ctx);

	// Start recording 
	ctx.ResetCommandList();
	auto commandList = ctx.GetCommandList();

	lut->TransitionCommand(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandList->SetPipelineState(pipelineState_);
	commandList->SetComputeRootSignature(rootSignature_.handle_);

	// Descriptors
	descriptorHeap_.BindHeap(commandList);
	descriptorHeap_.BindDescriptorsCompute(commandList, 0);

	commandList->Dispatch(
		lut->width_ / 32,
		lut->height_ / 32,
		1);

	lut->TransitionCommand(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	ctx.SubmitCommandListAndWaitForGPU();
}

void PipelineBRDFLUT::CreateDescriptors(DX12Context& ctx,
	DX12Image* lut)
{
	std::vector<DX12Descriptor> descriptors =
	{ {

		.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
		.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE,
		.shaderVisibility_ = D3D12_SHADER_VISIBILITY_ALL,
		.buffer_ = &(lut->buffer_),
		.uavDescription_ = lut->buffer_.GetUAVDescription(0)
	} };

	descriptorHeap_.descriptors_ = descriptors;
	descriptorHeap_.Create(ctx);

	CD3DX12_STATIC_SAMPLER_DESC sampler{ 1, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	rootSignature_.Create(ctx, sampler, descriptors, {}, 0, rootSignatureFlags);
}

void PipelineBRDFLUT::GenerateShader(DX12Context& ctx)
{
	computeShader_.Create(ctx, AppConfig::ShaderFolder + "IBL/BRDFLUT.hlsl", ShaderType::Compute);
}

void PipelineBRDFLUT::CreatePipeline(DX12Context& ctx)
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