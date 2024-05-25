#include "DX12Context.h"
#include "DX12Exception.h"
#include "Win32Application.h"
#include "Configs.h"

#include <iostream>

using Microsoft::WRL::ComPtr;

void DX12Context::Destroy()
{
	if (dmaAllocator_ != nullptr)
	{
		dmaAllocator_->Release();
		dmaAllocator_ = nullptr;
	}
	CloseHandle(fenceCompletionEvent_);
}

void DX12Context::Init(uint32_t swapchainWidth, uint32_t swapchainHeight)
{
	frameIndex_ = 0;

	uint32_t dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ID3D12Debug* debugController = nullptr;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
		if (debugController) { debugController->Release(); }
	}
#endif

	IDXGIFactory4* factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));
	{
		GetHardwareAdapter(factory, &adapter_);

		ThrowIfFailed(D3D12CreateDevice(
			adapter_.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&device_)
		));
	}

	// Debug callback
	SetInfoQueue();

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ThrowIfFailed(device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue_)));

	// Swapchain
	CreateSwapchain(factory, swapchainWidth, swapchainHeight);

	// Command buffer
	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		ThrowIfFailed(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators_[i])));
	}

	ThrowIfFailed(device_->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocators_[0].Get(), // TODO
		nullptr,
		IID_PPV_ARGS(&commandList_)));
	commandList_->Close();

	// Memory allocator
	{
		D3D12MA::ALLOCATOR_DESC desc = {
			.pDevice = device_.Get(),
			.pAdapter = adapter_.Get(),
		};

		ThrowIfFailed(D3D12MA::CreateAllocator(&desc, &dmaAllocator_));
	}

	// MSAA
	{
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msLevels;
		msLevels.Format = swapchainFormat_;
		msLevels.SampleCount = AppConfig::MSAACount;
		msLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		device_->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msLevels, sizeof(msLevels));
		std::cout << "MSAA = " << msLevels.SampleCount << '\n';
	}

	// Shader compiler
	CreateDXC();

	// Release
	factory->Release();
}

void DX12Context::CreateSwapchain(IDXGIFactory4* factory, uint32_t swapchainWidth, uint32_t swapchainHeight)
{
	swapchainWidth_ = swapchainWidth;
	swapchainHeight_ = swapchainHeight;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc =
	{
		.Width = swapchainWidth_,
		.Height = swapchainHeight_,
		.Format = swapchainFormat_,
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

	ThrowIfFailed(swapChain.As(&swapchain_));
	frameIndex_ = swapchain_->GetCurrentBackBufferIndex();

	// This sample does not support fullscreen transitions.
	ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));
}

void DX12Context::ResizeSwapchain(uint32_t swapchainWidth, uint32_t swapchainHeight)
{
	ThrowIfFailed(swapchain_->ResizeBuffers(
		AppConfig::FrameCount,
		swapchainWidth,
		swapchainHeight,
		swapchainFormat_,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH |
		DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING |
		DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT));
}

void DX12Context::CreateDXC()
{
	ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_)));
	ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_)));
}

// Debug callback
// github.com/microsoft/DirectX-Specs/blob/master/d3d/MessageCallback.md
void DX12Context::SetInfoQueue()
{
	ComPtr<ID3D12InfoQueue> infoQueue;
	if (device_->QueryInterface(IID_PPV_ARGS(infoQueue.GetAddressOf())) >= 0)
	{
		ThrowIfFailed(infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true));
		ThrowIfFailed(infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true));
		ThrowIfFailed(infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true));

		ComPtr<ID3D12InfoQueue1> infoQueue1;
		if (infoQueue->QueryInterface(IID_PPV_ARGS(infoQueue1.ReleaseAndGetAddressOf())) >= 0)
		{
			auto MessageCallback = [](
				D3D12_MESSAGE_CATEGORY category,
				D3D12_MESSAGE_SEVERITY severity,
				D3D12_MESSAGE_ID id,
				LPCSTR pDescription,
				void* pContext)
			{
				std::cerr << "Validation Layer: " << pDescription;
			};

			DWORD callbackCookie = 0;
			ThrowIfFailed(infoQueue1->RegisterMessageCallback(
				MessageCallback,
				D3D12_MESSAGE_CALLBACK_FLAG_NONE,
				this,
				&callbackCookie));
		}
	}
}

void DX12Context::WaitForGPU()
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(commandQueue_->Signal(fence_.Get(), fenceValues_[frameIndex_]));

	// Wait until the fence has been processed.
	ThrowIfFailed(fence_->SetEventOnCompletion(fenceValues_[frameIndex_], fenceCompletionEvent_));
	WaitForSingleObjectEx(fenceCompletionEvent_, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	fenceValues_[frameIndex_]++;
}

void DX12Context::PresentSwapchain() const
{
	ThrowIfFailed(swapchain_->Present(1, 0));
}

void DX12Context::MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	const uint64_t currentFenceValue = fenceValues_[frameIndex_];
	ThrowIfFailed(commandQueue_->Signal(fence_.Get(), currentFenceValue));

	// Update the frame index.
	frameIndex_ = swapchain_->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (fence_->GetCompletedValue() < fenceValues_[frameIndex_])
	{
		ThrowIfFailed(fence_->SetEventOnCompletion(fenceValues_[frameIndex_], fenceCompletionEvent_));
		WaitForSingleObjectEx(fenceCompletionEvent_, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	fenceValues_[frameIndex_] = currentFenceValue + 1;
}

void DX12Context::ResetCommandAllocator() const
{
	ThrowIfFailed(commandAllocators_[frameIndex_]->Reset());
}

void DX12Context::ResetCommandList() const
{
	ThrowIfFailed(commandList_->Reset(commandAllocators_[frameIndex_].Get(), nullptr));
}

void DX12Context::CloseCommandList() const
{
	ThrowIfFailed(commandList_->Close());
}

void DX12Context::SubmitCommandList() const
{
	ThrowIfFailed(commandList_->Close());
	ID3D12CommandList* ppCommandLists[] = { GetCommandList() };
	commandQueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void DX12Context::SubmitCommandListAndWaitForGPU()
{
	SubmitCommandList();
	WaitForGPU();
}

void DX12Context::CreateFence()
{
	ThrowIfFailed(device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));
	fenceValues_[frameIndex_]++;

	// Create an event handle to use for frame synchronization.
	fenceCompletionEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceCompletionEvent_ == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
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

CD3DX12_VIEWPORT DX12Context::GetViewport() const
{
	return
		CD3DX12_VIEWPORT(
			0.0f,
			0.0f,
			static_cast<float>(swapchainWidth_),
			static_cast<float>(swapchainHeight_));
}

CD3DX12_RECT DX12Context::GetScissor() const
{
	return
		CD3DX12_RECT(
			0,
			0,
			static_cast<LONG>(swapchainWidth_),
			static_cast<LONG>(swapchainHeight_));
}