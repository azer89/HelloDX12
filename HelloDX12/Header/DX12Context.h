#ifndef HELLO_DX12_CONTEXT
#define HELLO_DX12_CONTEXT

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include "include/d3dx12/d3dx12.h"

#include <windows.h>
#include <wrl.h>
#include <shellapi.h>

using Microsoft::WRL::ComPtr;

class DX12Context
{
public:
	[[nodiscard]] ID3D12Device* GetDevice() const { return device_.Get(); }
	[[nodiscard]] ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }

	void Init(uint32_t swapchainWidth, uint32_t swapchainHeight);

	void WaitForPreviousFrame();

private:
	void GetHardwareAdapter(
		_In_ IDXGIFactory1* pFactory,
		_Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
		bool requestHighPerformanceAdapter = false);

// TODO Set to private
public:
	uint32_t swapchainWidth_ = 0;
	uint32_t swapchainHeight_ = 0;
	
	// Pipeline objects.
	ComPtr<IDXGISwapChain3> swapchain_;
	ComPtr<ID3D12Device> device_;
	ComPtr<ID3D12CommandAllocator> commandAllocator_;
	ComPtr<ID3D12CommandQueue> commandQueue_;
	ComPtr<ID3D12GraphicsCommandList> commandList_;

	// Synchronization objects.
	uint32_t frameIndex_;
	HANDLE fenceEvent_;
	ComPtr<ID3D12Fence> fence_;
	uint64_t fenceValue_;
};

#endif