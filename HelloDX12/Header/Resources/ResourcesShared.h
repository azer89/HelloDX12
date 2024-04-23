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

	[[nodiscard]] ID3D12Resource* GetOffscreenRenderTarget() const { return offcreenImage_.GetResource(); }
	[[nodiscard]] CD3DX12_CPU_DESCRIPTOR_HANDLE GetOffscreenRTVHandle() const { return offscreenRTVHandle_; }
	[[nodiscard]] ID3D12Resource* GetSwapchainRenderTarget(uint32_t frameIndex) const { return swapchainRenderTargets_[frameIndex]; }
	[[nodiscard]] CD3DX12_CPU_DESCRIPTOR_HANDLE GetSwapchainRTVHandle(uint32_t frameIndex) const { return swapchainRTVHandles_[frameIndex]; }
	[[nodiscard]] CD3DX12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() const { return dsvHandle_; }

private:
	// Render target
	void CreateSwapchainRTV(DX12Context& ctx);
	void CreateOffscreenRTV(DX12Context& ctx);

	// Depth stencil
	void CreateDSV(DX12Context& ctx);

private:
	uint32_t rtvIncrementSize_ = 0;

	// Swapchain RTV
	ID3D12DescriptorHeap* swapchainRTVHeap_ = nullptr;
	std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, AppConfig::FrameCount> swapchainRTVHandles_ = {};
	std::array<ID3D12Resource*, AppConfig::FrameCount> swapchainRenderTargets_ = { nullptr };

	// Offcreen RTV
	DX12Image offcreenImage_;
	CD3DX12_CPU_DESCRIPTOR_HANDLE offscreenRTVHandle_ = {};
	ID3D12DescriptorHeap* offscreenRTVHeap_ = nullptr;

	// Depth stencil
	DX12Image depthImage_;
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle_;
	ID3D12DescriptorHeap* dsvHeap_ = nullptr;
};

#endif