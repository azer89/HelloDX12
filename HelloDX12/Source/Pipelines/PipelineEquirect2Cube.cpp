#include "PipelineEquirect2Cube.h"

PipelineEquirect2Cube::PipelineEquirect2Cube(
	DX12Context& ctx) :
	PipelineBase(ctx)
{
}

void PipelineEquirect2Cube::GenerateCubemapFromHDR(DX12Context& ctx, DX12Image* hdrImage, DX12Image* cubemapImage)
{
	GenerateShader(ctx);
	CreatePipeline(ctx);
}

void PipelineEquirect2Cube::GenerateShader(DX12Context& ctx)
{
	computeShader_.Create(ctx, AppConfig::ShaderFolder + "Equirect2Cube.hlsl", ShaderType::Compute);
}

void PipelineEquirect2Cube::CreatePipeline(DX12Context& ctx)
{

}