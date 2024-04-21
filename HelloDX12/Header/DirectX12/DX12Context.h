#ifndef HELLO_DX12_CONTEXT
#define HELLO_DX12_CONTEXT

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <windows.h>
#include <wrl.h>
#include <shellapi.h>
#include <memory>
#include "include/d3dx12/d3dx12.h"

#include "D3D12MemAlloc.h"

#include "Configs.h"

using Microsoft::WRL::ComPtr;

class DX12Context
{
public:
	DX12Context() = default;
	~DX12Context() = default;

	// Not copyable or movable
	DX12Context(const DX12Context&) = delete;
	DX12Context& operator=(const DX12Context&) = delete;
	DX12Context(DX12Context&&) = delete;
	DX12Context& operator=(DX12Context&&) = delete;

	void Init(uint32_t swapchainWidth, uint32_t swapchainHeight);
	void Destroy();

	[[nodiscard]] uint32_t GetFrameIndex() const { return frameIndex_; }
	[[nodiscard]] ID3D12Device* GetDevice() const { return device_.Get(); }
	[[nodiscard]] uint32_t GetSwapchainWidth() const { return swapchainWidth_; }
	[[nodiscard]] uint32_t GetSwapchainHeight() const { return swapchainHeight_; }
	[[nodiscard]] IDXGISwapChain3* GetSwapchain() const { return swapchain_.Get(); }
	[[nodiscard]] D3D12MA::Allocator* GetDMAAllocator() const { return dmaAllocator_; }
	[[nodiscard]] ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }
	
	void CreateFence();
	void WaitForGPU();
	void MoveToNextFrame();
	void PresentSwapchain() const;
	
	void ResetCommandList() const;
	void CloseCommandList() const;
	void SubmitCommandList() const;
	void SubmitCommandListAndWaitForGPU();
	void ResetCommandAllocator() const;
	
	CD3DX12_VIEWPORT GetViewport() const;
	CD3DX12_RECT GetScissor() const;

private:
	void SetInfoQueue();
	
	void GetHardwareAdapter(
		_In_ IDXGIFactory1* pFactory,
		_Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
		bool requestHighPerformanceAdapter = false);

private:
	uint32_t swapchainWidth_ = 0;
	uint32_t swapchainHeight_ = 0;
	
	// Pipeline objects.
	ComPtr<IDXGISwapChain3> swapchain_ = nullptr;
	ComPtr<ID3D12Device> device_ = nullptr;
	ComPtr<IDXGIAdapter1> adapter_ = nullptr;
	ComPtr<ID3D12CommandAllocator> commandAllocators_[AppConfig::FrameCount] = {};
	ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;

	// Synchronization objects.
	uint32_t frameIndex_ = 0;
	HANDLE fenceEvent_ = nullptr;
	ComPtr<ID3D12Fence> fence_ = nullptr;
	uint64_t fenceValues_[AppConfig::FrameCount] = {};

	// D12 Memory Allocator
	D3D12MA::Allocator* dmaAllocator_ = nullptr;
};

#endif