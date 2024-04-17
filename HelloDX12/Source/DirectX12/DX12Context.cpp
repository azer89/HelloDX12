#include "DX12Context.h"
#include "DX12Exception.h"
#include "Win32Application.h"
#include "Configs.h"

DX12Context::~DX12Context()
{
}

void DX12Context::Init(uint32_t swapchainWidth, uint32_t swapchainHeight)
{
	swapchainWidth_ = swapchainWidth;
	swapchainHeight_ = swapchainHeight;
	frameIndex_ = 0;

	uint32_t dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));
	{
		GetHardwareAdapter(factory.Get(), &adapter_);

		ThrowIfFailed(D3D12CreateDevice(
			adapter_.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&device_)
		));
	}

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue_)));

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc =
	{
		.Width = swapchainWidth_,
		.Height = swapchainHeight_,
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = AppConfig::FrameCount,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD
	};
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		commandQueue_.Get(), // Swap chain needs the queue so that it can force a flush on it.
		Win32Application::GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	// This sample does not support fullscreen transitions.
	ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain.As(&swapchain_));
	frameIndex_ = swapchain_->GetCurrentBackBufferIndex();

	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		ThrowIfFailed(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators_[i])));
	}
	//ThrowIfFailed(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&oneTimeCommandAllocator_)));

	// TODO Why only one command list?
	ThrowIfFailed(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators_[frameIndex_].Get(), nullptr, IID_PPV_ARGS(&commandList_)));
	commandList_->Close();

	// Memory allocator
	{
		D3D12MA::ALLOCATOR_DESC desc = {
			.pDevice = device_.Get(),
			.pAdapter = adapter_.Get(),
		};

		ThrowIfFailed(D3D12MA::CreateAllocator(&desc, &dmaAllocator_));
	}
}

void DX12Context::WaitForGpu()
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(commandQueue_->Signal(fence_.Get(), fenceValues_[frameIndex_]));

	// Wait until the fence has been processed.
	ThrowIfFailed(fence_->SetEventOnCompletion(fenceValues_[frameIndex_], fenceEvent_));
	WaitForSingleObjectEx(fenceEvent_, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	fenceValues_[frameIndex_]++;
}

void DX12Context::MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = fenceValues_[frameIndex_];
	ThrowIfFailed(commandQueue_->Signal(fence_.Get(), currentFenceValue));

	// Update the frame index.
	frameIndex_ = swapchain_->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (fence_->GetCompletedValue() < fenceValues_[frameIndex_])
	{
		ThrowIfFailed(fence_->SetEventOnCompletion(fenceValues_[frameIndex_], fenceEvent_));
		WaitForSingleObjectEx(fenceEvent_, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	fenceValues_[frameIndex_] = currentFenceValue + 1;
}

void DX12Context::ResetCommandAllocator()
{
	ThrowIfFailed(commandAllocators_[frameIndex_]->Reset());
}

void DX12Context::ResetCommandList()
{
	ThrowIfFailed(commandList_->Reset(commandAllocators_[frameIndex_].Get(), nullptr));
}

void DX12Context::EndCommandListRecordingAndSubmit()
{
	ThrowIfFailed(commandList_->Close());
	ID3D12CommandList* ppCommandLists[] = { GetCommandList() };
	commandQueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// TODO Find a way without stalling GPU
	WaitForGpu();
}

void DX12Context::SetPipelineState(ID3D12PipelineState* pipeline)
{
	commandList_->SetPipelineState(pipeline);
}

void DX12Context::CreateFence()
{
	ThrowIfFailed(device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));
	fenceValues_[frameIndex_]++;

	// Create an event handle to use for frame synchronization.
	fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent_ == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

	// Wait for the command list to execute; we are reusing the same command 
	// list in our main loop but for now, we just want to wait for setup to 
	// complete before continuing.
	WaitForGpu();
}

// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, *ppAdapter will be set to nullptr.
_Use_decl_annotations_
void DX12Context::GetHardwareAdapter(
	IDXGIFactory1* pFactory,
	IDXGIAdapter1** ppAdapter,
	bool requestHighPerformanceAdapter)
{
	*ppAdapter = nullptr;

	ComPtr<IDXGIAdapter1> adapter;

	ComPtr<IDXGIFactory6> factory6;
	if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (
			uint32_t adapterIndex = 0;
			SUCCEEDED(factory6->EnumAdapterByGpuPreference(
				adapterIndex,
				requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
				IID_PPV_ARGS(&adapter)));
				++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}

			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	if (adapter.Get() == nullptr)
	{
		for (uint32_t adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}

			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	*ppAdapter = adapter.Detach();
}

CD3DX12_VIEWPORT DX12Context::GetViewport()
{
	return
		CD3DX12_VIEWPORT(
			0.0f,
			0.0f,
			static_cast<float>(swapchainWidth_),
			static_cast<float>(swapchainHeight_));
}

CD3DX12_RECT DX12Context::GetScissor()
{
	return
		CD3DX12_RECT(
			0,
			0,
			static_cast<LONG>(swapchainWidth_),
			static_cast<LONG>(swapchainHeight_));
}