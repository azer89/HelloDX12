#include "ResourcesShared.h"
#include "DX12Exception.h"

ResourcesShared::~ResourcesShared()
{
	Destroy();
}

void ResourcesShared::Destroy()
{
	if (swapchainRTVHeap_) { swapchainRTVHeap_->Release(); }

	if (depthStencil_) { depthStencil_->Release(); }
	if (dsvHeap_) { dsvHeap_->Release(); }
	
	if (offscreenRTVHeap_) { offscreenRTVHeap_->Release(); }
	offcreenImage_.Destroy();

	for (auto& rt : swapchainRenderTargets_)
	{
		if (rt) { rt->Release(); }
	}
}

void ResourcesShared::Init(DX12Context& ctx)
{
	CreateSwapchainRTV(ctx);
	CreateOffscreenRTV(ctx);
	CreateDSV(ctx);
}

void ResourcesShared::CreateSwapchainRTV(DX12Context& ctx)
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = AppConfig::FrameCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&swapchainRTVHeap_)))

	// Create a RTV for each frame
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(swapchainRTVHeap_->GetCPUDescriptorHandleForHeapStart());
	swapchainRTVIncrementSize_ = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (uint32_t i = 0; i < AppConfig::FrameCount; i++)
	{
		ThrowIfFailed(ctx.GetSwapchain()->GetBuffer(i, IID_PPV_ARGS(&swapchainRenderTargets_[i])))
		ctx.GetDevice()->CreateRenderTargetView(swapchainRenderTargets_[i], nullptr, rtvHandle);
		rtvHandle.Offset(1, swapchainRTVIncrementSize_);
	}
}

void ResourcesShared::CreateOffscreenRTV(DX12Context& ctx)
{
	// Create Image
	offcreenImage_.CreateColorAttachment(ctx);

	// Create RTV
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = 1,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&offscreenRTVHeap_)))

	// Create a RTV for each frame
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(offscreenRTVHeap_->GetCPUDescriptorHandleForHeapStart());
	offscreenRTVIncrementSize_ = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	ctx.GetDevice()->CreateRenderTargetView(offcreenImage_.buffer_.resource_, nullptr, rtvHandle);
}

// TODO Use DMA
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

	swapchainRTVIncrementSize_ = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

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
		depthStencil_,
		&depthStencilDesc,
		dsvHeap_->GetCPUDescriptorHandleForHeapStart());
}

ID3D12Resource* ResourcesShared::GetSwapchainRenderTarget(uint32_t frameIndex) const
{
	return swapchainRenderTargets_[frameIndex];
}

CD3DX12_CPU_DESCRIPTOR_HANDLE ResourcesShared::GetSwapchainRTVHandle(uint32_t frameIndex) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(swapchainRTVHeap_->GetCPUDescriptorHandleForHeapStart(), frameIndex, swapchainRTVIncrementSize_);
}

CD3DX12_CPU_DESCRIPTOR_HANDLE ResourcesShared::GetDSVHandle() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap_->GetCPUDescriptorHandleForHeapStart());
}