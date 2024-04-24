#include "ResourcesShared.h"
#include "DX12Exception.h"

ResourcesShared::~ResourcesShared()
{
	Destroy();
}

void ResourcesShared::Destroy()
{
	if (swapchainRTVHeap_) 
	{ 
		swapchainRTVHeap_->Release(); 
	}

	if (dsvHeap_) 
	{ 
		dsvHeap_->Release(); 
	}
	depthImage_.Destroy();

	if (offscreenRTVHeap_) 
	{ 
		offscreenRTVHeap_->Release(); 
	}
	offcreenImage_.Destroy();

	for (auto& rt : swapchainRenderTargets_)
	{
		if (rt) { rt->Release(); }
	}
}

void ResourcesShared::Init(DX12Context& ctx)
{
	rtvIncrementSize_ = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
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
	for (uint32_t i = 0; i < AppConfig::FrameCount; i++)
	{
		ThrowIfFailed(ctx.GetSwapchain()->GetBuffer(i, IID_PPV_ARGS(&swapchainRenderTargets_[i])))
		ctx.GetDevice()->CreateRenderTargetView(swapchainRenderTargets_[i], nullptr, rtvHandle);
		rtvHandle.Offset(1, rtvIncrementSize_);
	}

	// Create handles
	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		swapchainRTVHandles_[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			swapchainRTVHeap_->GetCPUDescriptorHandleForHeapStart(),
			i,
			rtvIncrementSize_);
	}
}

void ResourcesShared::CreateOffscreenRTV(DX12Context& ctx)
{
	// Create Image
	offcreenImage_.CreateColorAttachment(ctx);

	/*{
		// Start recording 
		ctx.ResetCommandList();
		auto commandList = ctx.GetCommandList();

		const auto resourceBarrier1 =
			CD3DX12_RESOURCE_BARRIER::Transition(
				offcreenImage_.GetResource(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		commandList->ResourceBarrier(1, &resourceBarrier1);

		ctx.SubmitCommandListAndWaitForGPU();
	}*/

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
	ctx.GetDevice()->CreateRenderTargetView(offcreenImage_.buffer_.resource_, nullptr, rtvHandle);

	// Create handle
	offscreenRTVHandle_ = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		offscreenRTVHeap_->GetCPUDescriptorHandleForHeapStart(),
		0,
		rtvIncrementSize_);
}

void ResourcesShared::CreateDSV(DX12Context& ctx)
{
	depthImage_.CreateDepthAttachment(ctx);

	// Describe and create a depth stencil view (DSV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		.NumDescriptors = 1,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap_)))

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc =
	{
		.Format = DXGI_FORMAT_D32_FLOAT,
		.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
		.Flags = D3D12_DSV_FLAG_NONE
	};

	ctx.GetDevice()->CreateDepthStencilView(
		depthImage_.GetResource(),
		&depthStencilDesc,
		dsvHeap_->GetCPUDescriptorHandleForHeapStart());

	// Create handle
	dsvHandle_ = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap_->GetCPUDescriptorHandleForHeapStart());
}