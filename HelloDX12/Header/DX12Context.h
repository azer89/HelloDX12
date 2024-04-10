#ifndef HELLO_DX12_CONTEXT
#define HELLO_DX12_CONTEXT

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
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

	void Init(UINT swapchainWidth, UINT swapchainHeight);

	void WaitForPreviousFrame();

private:
	void GetHardwareAdapter(
		_In_ IDXGIFactory1* pFactory,
		_Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
		bool requestHighPerformanceAdapter = false);

// TODO Set to private
public:
	UINT swapchainWidth_ = 0;
	UINT swapchainHeight_ = 0;

	static const UINT FrameCount = 2;
	
	// Pipeline objects.
	CD3DX12_VIEWPORT viewport_;
	CD3DX12_RECT scissor_;
	ComPtr<IDXGISwapChain3> swapchain_;
	ComPtr<ID3D12Device> device_;
	
	ComPtr<ID3D12CommandAllocator> commandAllocator_;
	ComPtr<ID3D12CommandQueue> commandQueue_;
	ComPtr<ID3D12GraphicsCommandList> commandList_;

	ComPtr<ID3D12PipelineState> pipelineState_;
	ComPtr<ID3D12RootSignature> rootSignature_;
	ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	ComPtr<ID3D12DescriptorHeap> srvHeap_;
	UINT rtvDescriptorSize_;
	ComPtr<ID3D12Resource> renderTargets_[FrameCount];

	// Synchronization objects.
	UINT frameIndex_;
	HANDLE fenceEvent_;
	ComPtr<ID3D12Fence> fence_;
	UINT64 fenceValue_;
};

#endif