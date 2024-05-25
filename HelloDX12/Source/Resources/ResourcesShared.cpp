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

	for (auto& sw : swapchainBuffers_)
	{
		sw.Destroy();
	}
}

void ResourcesShared::Init(DX12Context& ctx)
{
	rtvIncrementSize_ = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CreateDescriptorHeaps(ctx);
	GrabSwapchain(ctx);
	CreateSingleSampledRTV(ctx);
	CreateMultiSampledRTV(ctx);
	CreateDSV(ctx);
}

void ResourcesShared::OnWindowResize(uint32_t width, uint32_t height)
{
}

void ResourcesShared::CreateDescriptorHeaps(DX12Context& ctx)
{
	{
		constexpr D3D12_DESCRIPTOR_HEAP_DESC desc =
		{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			.NumDescriptors = AppConfig::FrameCount,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
		};
		ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&swapchainRTVHeap_)));
	}

	{
		constexpr D3D12_DESCRIPTOR_HEAP_DESC desc =
		{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			.NumDescriptors = 1,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
		};
		ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&multiSampledRTVHeap_)));
	}

	{
		constexpr D3D12_DESCRIPTOR_HEAP_DESC desc =
		{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			.NumDescriptors = 1,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
		};
		ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&singleSampledRTVHeap_)));
	}

	{
		constexpr D3D12_DESCRIPTOR_HEAP_DESC desc =
		{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			.NumDescriptors = 1,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
		};
		ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&dsvHeap_)));
	}
}

void ResourcesShared::GrabSwapchain(DX12Context& ctx)
{
	// Handle
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(swapchainRTVHeap_->GetCPUDescriptorHandleForHeapStart());
	for (uint32_t i = 0; i < AppConfig::FrameCount; i++)
	{
		swapchainBuffers_[i].SetAsSwapchainBuffer(ctx, rtvHandle, i);
		rtvHandle.Offset(1, rtvIncrementSize_);
	}

	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		swapchainCPUHandles_[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(
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
	multiSampledImage_.SetName("Multi_Sampled_Image");

	// Barrier
	{
		ctx.ResetCommandList();
		multiSampledImage_.TransitionCommand(ctx.GetCommandList(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		ctx.SubmitCommandListAndWaitForGPU();
	}

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
	singleSampledImage_.SetName("Single_Sampled_Image");

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
	depthImage_.SetName("Depth_Image");

	// DSV
	constexpr D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc =
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