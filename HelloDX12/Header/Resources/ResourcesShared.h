#ifndef HELLO_DX12_RESOURCES_SHARED
#define HELLO_DX12_RESOURCES_SHARED

#include "DX12Context.h"
#include "DX12Image.h"
#include "ResourcesBase.h"

#include <array>

class ResourcesShared final : public ResourcesBase
{
public:
	ResourcesShared() = default;
	~ResourcesShared();

	void Destroy() override;
	void Init(DX12Context& ctx);
	ID3D12Resource* GetRenderTarget(uint32_t frameIndex) const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(uint32_t frameIndex) const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() const;

private:
	// Render target
	void CreateRTV(DX12Context& ctx);

	// Depth stencil
	void CreateDSV(DX12Context& ctx);

public:
	// Swapchain RTV
	uint32_t rtvIncrementSize_ = 0;
	ID3D12DescriptorHeap* rtvHeap_ = nullptr;
	std::array<ID3D12Resource*, AppConfig::FrameCount> renderTargets_ = { nullptr };

	// Offcreen RTV
	DX12Image offcreenImage_;

	// Depth stencil
	ID3D12Resource* depthStencil_ = nullptr;
	ID3D12DescriptorHeap* dsvHeap_ = nullptr;
};

#endif