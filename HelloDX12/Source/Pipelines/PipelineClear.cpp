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

	resourcesShared_->GetSwapchainBuffer(ctx.GetFrameIndex())->TransitionCommand(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Clear swapchain
	const auto swapchainRtvHandle = resourcesShared_->GetSwapchainCPUHandle(ctx.GetFrameIndex());
	commandList->ClearRenderTargetView(swapchainRtvHandle, AppConfig::ClearColor , 0, nullptr);

	// Clear offscreen attachment
	const auto offscreenRtvHandle = resourcesShared_->GetMultiSampledRTVHandle();
	commandList->ClearRenderTargetView(offscreenRtvHandle, AppConfig::ClearColor, 0, nullptr);

	const auto dsvHandle = resourcesShared_->GetDSVHandle();
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}