#include "PipelineSpecularMap.h"

PipelineSpecularMap::PipelineSpecularMap(
	DX12Context& ctx) :
	PipelineBase(ctx)
{
}

void PipelineSpecularMap::Execute(DX12Context& ctx,
	DX12Image* environmentMap,
	DX12Image* specularMap)
{
}

void PipelineSpecularMap::CreateDescriptors(DX12Context& ctx,
	DX12Image* environmentMap,
	DX12Image* specularMap)
{
}

void PipelineSpecularMap::GenerateShader(DX12Context& ctx)
{
}

void PipelineSpecularMap::CreatePipeline(DX12Context& ctx)
{
}