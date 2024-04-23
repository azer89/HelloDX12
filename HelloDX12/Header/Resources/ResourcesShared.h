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
	ID3D12Resource* GetSwapchainRenderTarget(uint32_t frameIndex) const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetSwapchainRTVHandle(uint32_t frameIndex) const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() const;

private:
	// Render target
	void CreateSwapchainRTV(DX12Context& ctx);
	void CreateOffscreenRTV(DX12Context& ctx);

	// Depth stencil
	void CreateDSV(DX12Context& ctx);

private:
	// Swapchain RTV
	uint32_t swapchainRTVIncrementSize_ = 0;
	ID3D12DescriptorHeap* swapchainRTVHeap_ = nullptr;
	std::array<ID3D12Resource*, AppConfig::FrameCount> swapchainRenderTargets_ = { nullptr };

	// Offcreen RTV
	DX12Image offcreenImage_;
	uint32_t offscreenRTVIncrementSize_ = 0;
	ID3D12DescriptorHeap* offscreenRTVHeap_ = nullptr;

	// Depth stencil
	ID3D12Resource* depthStencil_ = nullptr;
	ID3D12DescriptorHeap* dsvHeap_ = nullptr;
};

#endif