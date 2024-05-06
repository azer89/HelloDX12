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
	ID3D12GraphicsCommandList* commandList = ctx.GetCommandList();
	const uint32_t frameIndex = ctx.GetFrameIndex();
	resourcesShared_->GetSwapchainBuffer(frameIndex)->TransitionCommand(commandList, D3D12_RESOURCE_STATE_PRESENT);
}