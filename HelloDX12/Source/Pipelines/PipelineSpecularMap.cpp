#include "PipelineSpecularMap.h"
#include "Utility.h"

constexpr uint32_t ROOT_CONSTANT_COUNT = 1;

PipelineSpecularMap::PipelineSpecularMap(
	DX12Context& ctx) :
	PipelineBase(ctx)
{
}

PipelineSpecularMap::~PipelineSpecularMap()
{
	Destroy();
}

void PipelineSpecularMap::Destroy()
{
	descriptorHeap_.Destroy();
}

void PipelineSpecularMap::Execute(DX12Context& ctx,
	DX12Image* environmentMap,
	DX12Image* specularMap)
{

	/*CreateDescriptors(ctx, environmentMap, specularMap);
	GenerateShader(ctx);
	CreatePipeline(ctx);

	// Start recording 
	ctx.ResetCommandList();
	auto commandList = ctx.GetCommandList();

	environmentMap->TransitionCommand(commandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	specularMap->TransitionCommand(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	const uint32_t mipmapCount = Utility::MipMapCount(specularMap->width_, specularMap->height_);
	for (int i = static_cast<int>(mipmapCount - 1u); i >= 0; --i)
	{
	}

	environmentMap->TransitionCommand(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	specularMap->TransitionCommand(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	ctx.SubmitCommandListAndWaitForGPU();*/
}

void PipelineSpecularMap::CreateDescriptors(DX12Context& ctx,
	DX12Image* environmentMap,
	DX12Image* specularMap)
{
	const uint32_t mipmapCount = Utility::MipMapCount(specularMap->width_, specularMap->height_);
	const uint32_t descriptorCount = mipmapCount + 1; // Add one for the input
	std::vector<DX12Descriptor> descriptorsA(descriptorCount);
	descriptorsA[0] =
	{
		.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
		.shaderVisibility_ = D3D12_SHADER_VISIBILITY_ALL,
		.buffer_ = &(environmentMap->buffer_),
		.srvDescription_ = environmentMap->buffer_.GetSRVDescription()
	};

	for (int i = static_cast<int>(mipmapCount - 1u); i >= 0; --i)
	{
		descriptorsA[i + 1] =
		{
			.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
			.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE,
			.shaderVisibility_ = D3D12_SHADER_VISIBILITY_ALL,
			.buffer_ = &(specularMap->buffer_),
			.uavDescription_ = specularMap->buffer_.GetUAVDescription(i)
		};
	}

	descriptorHeap_.descriptors_ = descriptorsA;
	descriptorHeap_.Create(ctx);

	std::vector<CD3DX12_STATIC_SAMPLER_DESC> samplerArray = { { 1, D3D12_FILTER_MIN_MAG_MIP_LINEAR } };
	samplerArray[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerArray[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerArray[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	std::vector<DX12Descriptor> descriptorsB = { descriptorsA[0], descriptorsA[1] };
	rootSignature_.Create(ctx, samplerArray, descriptorsB, {}, ROOT_CONSTANT_COUNT, rootSignatureFlags);
}

void PipelineSpecularMap::GenerateShader(DX12Context& ctx)
{
	computeShader_.Create(ctx, AppConfig::ShaderFolder + "IBL/Specular.hlsl", ShaderType::Compute);
}

void PipelineSpecularMap::CreatePipeline(DX12Context& ctx)
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