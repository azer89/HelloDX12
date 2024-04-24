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

	if (multiSampledRTVHeap_) 
	{ 
		multiSampledRTVHeap_->Release(); 
	}
	multiSampledImage_.Destroy();

	if (singleSampledRTVHeap_) 
	{ 
		singleSampledRTVHeap_->Release(); 
	}
	singleSampledImage_.Destroy();

	for (auto& rt : swapchainRenderTargets_)
	{
		if (rt) { rt->Release(); }
	}
}

void ResourcesShared::Init(DX12Context& ctx)
{
	rtvIncrementSize_ = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CreateSwapchainRTV(ctx);
	CreateSingleSampledRTV(ctx);
	CreateMultiSampledRTV(ctx);
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

void ResourcesShared::CreateMultiSampledRTV(DX12Context& ctx)
{
	// Create Image
	uint32_t msaaCount = AppConfig::MSAACount;
	multiSampledImage_.CreateColorAttachment(ctx, msaaCount);

	{
		// Start recording 
		ctx.ResetCommandList();
		auto commandList = ctx.GetCommandList();

		// Barrier
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			multiSampledImage_.GetResource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 
			D3D12_RESOURCE_STATE_RENDER_TARGET);
		
		commandList->ResourceBarrier(1, &barrier);

		ctx.SubmitCommandListAndWaitForGPU();
	}

	// Create RTV
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = 1,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&multiSampledRTVHeap_)))

	// Create a RTV for each frame
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(multiSampledRTVHeap_->GetCPUDescriptorHandleForHeapStart());
	ctx.GetDevice()->CreateRenderTargetView(multiSampledImage_.buffer_.resource_, nullptr, rtvHandle);

	// Create handle
	multiSampledRTVHandle_ = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		multiSampledRTVHeap_->GetCPUDescriptorHandleForHeapStart(),
		0,
		rtvIncrementSize_);
}

void ResourcesShared::CreateSingleSampledRTV(DX12Context& ctx)
{
	// Create Image
	uint32_t msaaCount = 1;
	singleSampledImage_.CreateColorAttachment(ctx, msaaCount);

	// Create RTV
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = 1,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&singleSampledRTVHeap_)))

	// Create a RTV for each frame
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(singleSampledRTVHeap_->GetCPUDescriptorHandleForHeapStart());
	ctx.GetDevice()->CreateRenderTargetView(singleSampledImage_.buffer_.resource_, nullptr, rtvHandle);

	// Create handle
	singleSampledRTVHandle_ = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		singleSampledRTVHeap_->GetCPUDescriptorHandleForHeapStart(),
		0,
		rtvIncrementSize_);
}

void ResourcesShared::CreateDSV(DX12Context& ctx)
{
	uint32_t msaaCount = AppConfig::MSAACount;
	depthImage_.CreateDepthAttachment(ctx, msaaCount);

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
		.ViewDimension = msaaCount == 1 ? D3D12_DSV_DIMENSION_TEXTURE2D : D3D12_DSV_DIMENSION_TEXTURE2DMS,
		.Flags = D3D12_DSV_FLAG_NONE
	};

	ctx.GetDevice()->CreateDepthStencilView(
		depthImage_.GetResource(),
		&depthStencilDesc,
		dsvHeap_->GetCPUDescriptorHandleForHeapStart());

	// Create handle
	dsvHandle_ = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap_->GetCPUDescriptorHandleForHeapStart());
}