#ifndef HELLO_DX12_RESOURCES_SHARED
#define HELLO_DX12_RESOURCES_SHARED

#include "DX12Context.h"
#include "ResourcesBase.h"

class ResourcesShared final : ResourcesBase
{
public:
	void Init(DX12Context& ctx);

	ID3D12Resource* GetRenderTarget(uint32_t frameIndex);
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(uint32_t frameIndex);
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDSVHandle();

private:
	// Render target
	void CreateRTV(DX12Context& ctx);

	// Depth stencil
	void CreateDSV(DX12Context& ctx);

public:
	// Render target
	uint32_t rtvIncrementSize_;
	ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	ComPtr<ID3D12Resource> renderTargets_[AppConfig::FrameCount];

	// Depth stencil
	ComPtr<ID3D12Resource> depthStencil_;
	ComPtr<ID3D12DescriptorHeap> dsvHeap_;
};

#endif