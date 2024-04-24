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
	ID3D12GraphicsCommandList* commandList = ctx.GetCommandList();

	const auto resourceBarrier1 =
		CD3DX12_RESOURCE_BARRIER::Transition(
			resourcesShared_->GetSwapchainRenderTarget(ctx.GetFrameIndex()),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &resourceBarrier1);

	const auto resourceBarrier2 =
		CD3DX12_RESOURCE_BARRIER::Transition(
			resourcesShared_->GetOffscreenRenderTarget(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &resourceBarrier2);

	const auto rtvHandle = resourcesShared_->GetSwapchainRTVHandle(ctx.GetFrameIndex());
	const auto dsvHandle = resourcesShared_->GetDSVHandle();

	constexpr float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}