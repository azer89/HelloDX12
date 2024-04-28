#include "PipelineSkybox.h"

PipelineSkybox::PipelineSkybox(
	DX12Context& ctx,
	ResourcesIBL* resourcesIBL) :
	PipelineBase(ctx),
	resourcesIBL_(resourcesIBL)
{
}

void PipelineSkybox::Update(DX12Context& ctx)
{
}

void PipelineSkybox::PopulateCommandList(DX12Context& ctx)
{
}

void PipelineSkybox::GenerateShader(DX12Context& ctx)
{
}

void PipelineSkybox::CreatePipeline(DX12Context& ctx)
{
}