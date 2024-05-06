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
	// RTV
	const D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = AppConfig::FrameCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&swapchainRTVHeap_)))

	// Handle
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(swapchainRTVHeap_->GetCPUDescriptorHandleForHeapStart());
	for (uint32_t i = 0; i < AppConfig::FrameCount; i++)
	{
		ThrowIfFailed(ctx.GetSwapchain()->GetBuffer(i, IID_PPV_ARGS(&swapchainRenderTargets_[i])))
		ctx.GetDevice()->CreateRenderTargetView(swapchainRenderTargets_[i], nullptr, rtvHandle);
		rtvHandle.Offset(1, rtvIncrementSize_);
	}

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
	constexpr uint32_t msaaCount = AppConfig::MSAACount;
	multiSampledImage_.CreateColorAttachment(ctx, msaaCount);
	multiSampledImage_.GetResource()->SetName(L"Multi_Sampled_Image");

	// Barrier
	{
		ctx.ResetCommandList();
		multiSampledImage_.TransitionCommand(ctx.GetCommandList(), D3D12_RESOURCE_STATE_RENDER_TARGET);
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

	// Handle
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(multiSampledRTVHeap_->GetCPUDescriptorHandleForHeapStart());
	ctx.GetDevice()->CreateRenderTargetView(multiSampledImage_.buffer_.resource_, nullptr, rtvHandle);
	
	multiSampledRTVHandle_ = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		multiSampledRTVHeap_->GetCPUDescriptorHandleForHeapStart(),
		0,
		rtvIncrementSize_);
}

void ResourcesShared::CreateSingleSampledRTV(DX12Context& ctx)
{
	// Create Image
	constexpr uint32_t msaaCount = 1;
	singleSampledImage_.CreateColorAttachment(ctx, msaaCount);
	singleSampledImage_.GetResource()->SetName(L"Single_Sampled_Image");

	// RTV
	const D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = 1,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&singleSampledRTVHeap_)))

	// Handle
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(singleSampledRTVHeap_->GetCPUDescriptorHandleForHeapStart());
	ctx.GetDevice()->CreateRenderTargetView(singleSampledImage_.buffer_.resource_, nullptr, rtvHandle);

	singleSampledRTVHandle_ = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		singleSampledRTVHeap_->GetCPUDescriptorHandleForHeapStart(),
		0,
		rtvIncrementSize_);
}

void ResourcesShared::CreateDSV(DX12Context& ctx)
{
	constexpr uint32_t msaaCount = AppConfig::MSAACount;
	depthImage_.CreateDepthAttachment(ctx, msaaCount);
	depthImage_.GetResource()->SetName(L"Depth_Image");

	// Heap
	const D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		.NumDescriptors = 1,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap_)))

	// DSV
	const D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc =
	{
		.Format = DXGI_FORMAT_D32_FLOAT,
		.ViewDimension = msaaCount == 1 ? D3D12_DSV_DIMENSION_TEXTURE2D : D3D12_DSV_DIMENSION_TEXTURE2DMS,
		.Flags = D3D12_DSV_FLAG_NONE
	};

	ctx.GetDevice()->CreateDepthStencilView(
		depthImage_.GetResource(),
		&depthStencilDesc,
		dsvHeap_->GetCPUDescriptorHandleForHeapStart());

	dsvHandle_ = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap_->GetCPUDescriptorHandleForHeapStart());
}