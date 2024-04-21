#include "PipelineClear.h"

PipelineClear::PipelineClear(
	DX12Context& ctx,
	ResourcesShared* resourcesShared) :
	PipelineBase(ctx),
	resourcesShared_(resourcesShared)
{
}

void PipelineClear::PopulateCommandList(DX12Context& ctx)
{
}