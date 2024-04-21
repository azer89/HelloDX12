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

	const auto resourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(
			resourcesShared_->GetRenderTarget(ctx.GetFrameIndex()),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &resourceBarrier);
}