#include "PipelineDiffuseMap.h"

PipelineDiffuseMap::PipelineDiffuseMap(
	DX12Context& ctx) :
	PipelineBase(ctx)
{
}

PipelineDiffuseMap::~PipelineDiffuseMap()
{
	Destroy();
}

void PipelineDiffuseMap::Destroy()
{
	descriptorHeap_.Destroy();
}

void PipelineDiffuseMap::Execute(DX12Context& ctx,
	DX12Image* environmentMap,
	DX12Image* diffuseMap)
{
	CreateDescriptors(ctx, environmentMap, diffuseMap);
	GenerateShader(ctx);
	CreatePipeline(ctx);

	// Start recording 
	ctx.ResetCommandList();
	auto commandList = ctx.GetCommandList();

	environmentMap->TransitionCommand(commandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	diffuseMap->TransitionCommand(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandList->SetComputeRootSignature(rootSignature_.handle_);
	commandList->SetPipelineState(pipelineState_);
	commandList->SetDescriptorHeaps(1, &descriptorHeap_.handle_);

	descriptorHeap_.BindHeap(commandList);
	descriptorHeap_.BindDescriptorsCompute(commandList, 0);

	const uint32_t groupCountX = std::max<uint32_t>(1, diffuseMap->width_ / 4);
	const uint32_t groupCountY = std::max<uint32_t>(1, diffuseMap->height_ / 4);
	constexpr uint32_t groupCountZ = 6;
	commandList->Dispatch(groupCountX, groupCountY, groupCountZ);

	environmentMap->TransitionCommand(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	diffuseMap->TransitionCommand(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	
	ctx.SubmitCommandListAndWaitForGPU();
}

void PipelineDiffuseMap::CreateDescriptors(DX12Context& ctx,
	DX12Image* environmentMap,
	DX12Image* diffuseMap)
{
	std::vector<DX12Descriptor> descriptors =
	{
		{
			.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE,
			.shaderVisibility_ = D3D12_SHADER_VISIBILITY_ALL,
			.buffer_ = &(environmentMap->buffer_),
			.srvDescription_ = environmentMap->buffer_.GetSRVDescription()
		},
		{
			.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
			.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE,
			.shaderVisibility_ = D3D12_SHADER_VISIBILITY_ALL,
			.buffer_ = &(diffuseMap->buffer_),
			.uavDescription_ = diffuseMap->buffer_.GetUAVDescription(0)
		}
	};

	descriptorHeap_.descriptors_ = descriptors;
	descriptorHeap_.Create(ctx);

	std::vector<CD3DX12_STATIC_SAMPLER_DESC> samplerArray = { { 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR } };
	samplerArray[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	rootSignature_.Create(ctx, samplerArray, descriptors, {}, 0, rootSignatureFlags);
}

void PipelineDiffuseMap::GenerateShader(DX12Context& ctx)
{
	computeShader_.Create(ctx, AppConfig::ShaderFolder + "IBL/Diffuse.hlsl", ShaderType::Compute);
}

void PipelineDiffuseMap::CreatePipeline(DX12Context& ctx)
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