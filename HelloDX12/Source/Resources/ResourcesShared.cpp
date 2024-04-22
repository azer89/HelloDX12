#include "ResourcesShared.h"
#include "DX12Exception.h"

void ResourcesShared::Init(DX12Context& ctx)
{
	CreateRTV(ctx);
	CreateDSV(ctx);
}

void ResourcesShared::CreateRTV(DX12Context& ctx)
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = AppConfig::FrameCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap_)))

	// Create a RTV for each frame
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap_->GetCPUDescriptorHandleForHeapStart());
	rtvIncrementSize_ = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (uint32_t n = 0; n < AppConfig::FrameCount; n++)
	{
		ThrowIfFailed(ctx.GetSwapchain()->GetBuffer(n, IID_PPV_ARGS(&renderTargets_[n])))
			ctx.GetDevice()->CreateRenderTargetView(renderTargets_[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, rtvIncrementSize_);
	}
}

void ResourcesShared::CreateDSV(DX12Context& ctx)
{
	// Describe and create a depth stencil view (DSV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		.NumDescriptors = 1,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap_)))

	rtvIncrementSize_ = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc =
	{
		.Format = DXGI_FORMAT_D32_FLOAT,
		.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
		.Flags = D3D12_DSV_FLAG_NONE
	};

	CD3DX12_HEAP_PROPERTIES heapProperties =
		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	// Performance tip: Deny shader resource access to resources that don't need shader resource views.
	CD3DX12_RESOURCE_DESC resourceDescription =
		CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT,
			ctx.GetSwapchainWidth(),
			ctx.GetSwapchainHeight(),
			1,
			0,
			1,
			0,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);
	// Performance tip: Tell the runtime at resource creation the desired clear value.
	CD3DX12_CLEAR_VALUE clearValue =
		CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	ThrowIfFailed(ctx.GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDescription,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&depthStencil_)
	))

	ctx.GetDevice()->CreateDepthStencilView(
		depthStencil_.Get(),
		&depthStencilDesc,
		dsvHeap_->GetCPUDescriptorHandleForHeapStart());
}

ID3D12Resource* ResourcesShared::GetRenderTarget(uint32_t frameIndex) const
{
	return renderTargets_[frameIndex].Get();
}

CD3DX12_CPU_DESCRIPTOR_HANDLE ResourcesShared::GetRTVHandle(uint32_t frameIndex) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap_->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvIncrementSize_);
}

CD3DX12_CPU_DESCRIPTOR_HANDLE ResourcesShared::GetDSVHandle() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap_->GetCPUDescriptorHandleForHeapStart());
}