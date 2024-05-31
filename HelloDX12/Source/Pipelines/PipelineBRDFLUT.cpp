#include "PipelineBRDFLUT.h"

PipelineBRDFLUT::PipelineBRDFLUT(
	DX12Context& ctx) :
	PipelineBase(ctx)
{
}

void PipelineBRDFLUT::Execute(DX12Context& ctx,
	DX12Image* lut)
{

}

void PipelineBRDFLUT::CreateDescriptors(DX12Context& ctx,
	DX12Image* lut)
{
	CD3DX12_STATIC_SAMPLER_DESC sampler{ 1, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
}

void PipelineBRDFLUT::GenerateShader(DX12Context& ctx)
{
}

void PipelineBRDFLUT::CreatePipeline(DX12Context& ctx)
{
}