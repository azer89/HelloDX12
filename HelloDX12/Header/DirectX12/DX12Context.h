#ifndef HELLO_DX12_CONTEXT
#define HELLO_DX12_CONTEXT

#include <d3d12.h>
#include <dxgi1_6.h>
#include <windows.h>
#include <wrl.h>

#include <array>

#include "d3dx12.h" // Agility SDK
#include "dxcapi.h" // DXC
#include "D3D12MemAlloc.h"

#include "Configs.h"

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
	void CreateSwapchain(IDXGIFactory4* factory, uint32_t swapchainWidth, uint32_t swapchainHeight);
	void ResizeSwapchain(uint32_t swapchainWidth, uint32_t swapchainHeight);
	void Destroy();

	[[nodiscard]] uint32_t GetFrameIndex() const { return frameIndex_; }
	[[nodiscard]] ID3D12Device* GetDevice() const { return device_; }
	[[nodiscard]] D3D12MA::Allocator* GetDMAAllocator() const { return dmaAllocator_; }
	[[nodiscard]] IDxcUtils* GetDXCUtils() const { return dxcUtils_.Get(); }
	[[nodiscard]] IDxcCompiler3* GetDXCCompiler() const { return dxcCompiler_.Get(); }
	[[nodiscard]] ID3D12GraphicsCommandList* GetCommandList() const { return commandList_; }

	[[nodiscard]] uint32_t GetSwapchainWidth() const { return swapchainWidth_; }
	[[nodiscard]] uint32_t GetSwapchainHeight() const { return swapchainHeight_; }
	[[nodiscard]] DXGI_FORMAT GetSwapchainFormat() const { return swapchainFormat_; }
	[[nodiscard]] IDXGISwapChain3* GetSwapchain() const { return swapchain_; }
	[[nodiscard]] ID3D12Resource* GetSwapchainResource(uint32_t frameIndex) const { return swapchainResources_[frameIndex]; }
	
	void CreateFence();
	void WaitForGPU();
	void MoveToNextFrame();
	void WaitForAllFrames();
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

	void CreateDXC();

	void ObtainSwapchainResources();
	void ReleaseSwapchainResources();

private:
	uint32_t swapchainWidth_{};
	uint32_t swapchainHeight_{};
	DXGI_FORMAT swapchainFormat_{ DXGI_FORMAT_R8G8B8A8_UNORM };
	std::array<ID3D12Resource*, AppConfig::FrameCount> swapchainResources_{};
	IDXGISwapChain4* swapchain_{};

	ID3D12Device* device_{};
	IDXGIAdapter1* adapter_{};
	
	std::array<ID3D12CommandAllocator*, AppConfig::FrameCount> commandAllocators_{};
	ID3D12CommandQueue* commandQueue_{};
	ID3D12GraphicsCommandList* commandList_{};

	// Synchronization objects.
	uint32_t frameIndex_{};
	HANDLE fenceCompletionEvent_{};
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_{};
	uint64_t fenceValues_[AppConfig::FrameCount]{};

	// D12 Memory Allocator
	D3D12MA::Allocator* dmaAllocator_{};

	// DXC
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_{};
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_{};
};

#endif