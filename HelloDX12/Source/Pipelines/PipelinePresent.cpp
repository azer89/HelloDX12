#include "PipelinePresent.h"

PipelinePresent::PipelinePresent(
	DX12Context& ctx,
	ResourcesShared* resourcesShared) :
	PipelineBase(ctx),
	resourcesShared_(resourcesShared)
{
}

void PipelinePresent::PopulateCommandList(DX12Context& ctx)
{
}