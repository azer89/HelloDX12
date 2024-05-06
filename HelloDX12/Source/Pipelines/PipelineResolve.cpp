#include "PipelineResolve.h"

PipelineResolve::PipelineResolve(
	DX12Context& ctx,
	ResourcesShared* resourcesShared) :
	PipelineBase(ctx),
	resourcesShared_(resourcesShared)
{
}

void PipelineResolve::PopulateCommandList(DX12Context& ctx)
{
	ID3D12GraphicsCommandList* commandList = ctx.GetCommandList();

	resourcesShared_->GetMultiSampledBuffer()->TransitionCommand(commandList, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
	resourcesShared_->GetSingleSampledBuffer()->TransitionCommand(commandList, D3D12_RESOURCE_STATE_RESOLVE_DEST);

	commandList->ResolveSubresource(
		resourcesShared_->GetSingleSampledRenderTarget(),
		0,
		resourcesShared_->GetMultiSampledRenderTarget(),
		0,
		ctx.GetSwapchainFormat());

	resourcesShared_->GetMultiSampledBuffer()->TransitionCommand(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	resourcesShared_->GetSingleSampledBuffer()->TransitionCommand(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}