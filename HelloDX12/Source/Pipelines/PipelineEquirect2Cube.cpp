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
	GenerateShader(ctx);
	CreateRootSignature(ctx);
	CreateDescriptorHeap(ctx, hdrImage, cubemapImage, cubemapUAVDesc);
	CreatePipeline(ctx);
}

void PipelineEquirect2Cube::GenerateShader(DX12Context& ctx)
{
	computeShader_.Create(ctx, AppConfig::ShaderFolder + "Equirect2Cube.hlsl", ShaderType::Compute);
}

void PipelineEquirect2Cube::CreateRootSignature(DX12Context& ctx)
{
	std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges = {};
	ranges.emplace_back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
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
	descriptor_.CreateRootDescriptor(ctx, samplerDesc, rootParameters, rootSignatureFlags);
}

void PipelineEquirect2Cube::CreatePipeline(DX12Context& ctx)
{
	// PSO
	const D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc =
	{
		.pRootSignature = descriptor_.GetRootSignature(),
		.CS = CD3DX12_SHADER_BYTECODE(computeShader_.GetHandle())
	};
	ctx.GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_));
}

void PipelineEquirect2Cube::CreateDescriptorHeap(DX12Context& ctx,
	DX12Image* hdrImage,
	DX12Image* cubemapImage,
	const D3D12_UNORDERED_ACCESS_VIEW_DESC& cubemapUAVDesc)
{
	uint32_t descriptorCount = 2;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvSrcDesc = hdrImage->GetSRVDescription();

	descriptor_.CreateDescriptorHeap(ctx, descriptorCount);
}