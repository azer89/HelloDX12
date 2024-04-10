#ifndef HELLO_DX12_CONTEXT
#define HELLO_DX12_CONTEXT

#include "stdafx.h"

using Microsoft::WRL::ComPtr;

class DX12Context
{
// TODO Set to private
public:
	static const UINT FrameCount = 2;
	
	static const UINT TextureWidth = 256;
	static const UINT TextureHeight = 256;
	static const UINT TexturePixelSize = 4;
	
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
	
	// App resources.
	ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	ComPtr<ID3D12Resource> texture_;

	// Synchronization objects.
	UINT frameIndex_;
	HANDLE fenceEvent_;
	ComPtr<ID3D12Fence> fence_;
	UINT64 fenceValue_;
};

#endif