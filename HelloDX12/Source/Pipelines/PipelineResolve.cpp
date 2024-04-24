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

	// Multi-sampled
	const auto resourceBarrier1 =
		CD3DX12_RESOURCE_BARRIER::Transition(
			resourcesShared_->GetMultiSampledRenderTarget(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
	commandList->ResourceBarrier(1, &resourceBarrier1);

	// Single-sampled
	const auto resourceBarrier2 =
		CD3DX12_RESOURCE_BARRIER::Transition(
			resourcesShared_->GetSingleSampledRenderTarget(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RESOLVE_DEST);
	commandList->ResourceBarrier(1, &resourceBarrier2);

	commandList->ResolveSubresource(resourcesShared_->GetSingleSampledRenderTarget(),
		0,
		resourcesShared_->GetMultiSampledRenderTarget(),
		0,
		ctx.GetSwapchainFormat());

	// Multi-sampled
	const auto resourceBarrier3 =
		CD3DX12_RESOURCE_BARRIER::Transition(
			resourcesShared_->GetMultiSampledRenderTarget(),
			D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &resourceBarrier3);

	// Single-sampled
	const auto resourceBarrier4 =
		CD3DX12_RESOURCE_BARRIER::Transition(
			resourcesShared_->GetSingleSampledRenderTarget(),
			D3D12_RESOURCE_STATE_RESOLVE_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &resourceBarrier4);
}