#ifndef HELLO_DX12_RESOURCES_SHARED
#define HELLO_DX12_RESOURCES_SHARED

#include "DX12Context.h"
#include "ResourcesBase.h"

class ResourcesShared final : ResourcesBase
{
	void Init(DX12Context& ctx);

private:
	void CreateRTV(DX12Context& ctx);
	void CreateDSV(DX12Context& ctx);

public:
	// Render target
	uint32_t rtvIncrementSize_;
	ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	ComPtr<ID3D12Resource> renderTargets_[AppConfig::FrameCount];

	// Depth stencil view
	ComPtr<ID3D12Resource> depthStencil_;
	ComPtr<ID3D12DescriptorHeap> dsvHeap_;
};

#endif