#ifndef HELLO_DX12_RESOURCES_SHARED
#define HELLO_DX12_RESOURCES_SHARED

#include "DX12Context.h"
#include "DX12Buffer.h"
#include "DX12Image.h"
#include "ResourcesBase.h"

#include <array>

class ResourcesShared final : public ResourcesBase
{
public:
	ResourcesShared() = default;
	~ResourcesShared();

	void Init(DX12Context& ctx);
	void Destroy() override;
	
	[[nodiscard]] DX12Buffer* GetMultiSampledBuffer() { return &(multiSampledImage_.buffer_); }
	[[nodiscard]] ID3D12Resource* GetMultiSampledRenderTarget() const { return multiSampledImage_.GetResource(); }
	[[nodiscard]] CD3DX12_CPU_DESCRIPTOR_HANDLE GetMultiSampledRTVHandle() const { return multiSampledRTVHandle_; }
	[[nodiscard]] D3D12_SHADER_RESOURCE_VIEW_DESC GetMultiSampledSRVDescription() const { return multiSampledImage_.GetSRVDescription(); }

	[[nodiscard]] DX12Buffer* GetSingleSampledBuffer() { return &(singleSampledImage_.buffer_); }
	[[nodiscard]] ID3D12Resource* GetSingleSampledRenderTarget() const { return singleSampledImage_.GetResource(); }
	[[nodiscard]] CD3DX12_CPU_DESCRIPTOR_HANDLE GetSingleSampledRTVHandle() const { return singleSampledRTVHandle_; }
	[[nodiscard]] D3D12_SHADER_RESOURCE_VIEW_DESC GetSingleSampledSRVDescription() const { return singleSampledImage_.GetSRVDescription(); }
	
	//[[nodiscard]] ID3D12Resource* GetSwapchainRenderTarget(uint32_t frameIndex) const { return swapchainRenderTargets_[frameIndex]; }
	//[[nodiscard]] CD3DX12_CPU_DESCRIPTOR_HANDLE GetSwapchainRTVHandle(uint32_t frameIndex) const { return swapchainRTVHandles_[frameIndex]; }
	[[nodiscard]] DX12Buffer* GetSwapchainBuffer(uint32_t frameIndex) { return &(swapchainBuffers_[frameIndex]); }
	[[nodiscard]] ID3D12Resource* GetSwapchainResource(uint32_t frameIndex) const { return swapchainBuffers_[frameIndex].resource_; }
	[[nodiscard]] CD3DX12_CPU_DESCRIPTOR_HANDLE GetSwapchainCPUHandle(uint32_t frameIndex) const { return swapchainCPUHandles_[frameIndex]; }

	[[nodiscard]] CD3DX12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() const { return dsvHandle_; }

private:
	// Render target
	void GrabSwapchain(DX12Context& ctx);
	void CreateSingleSampledRTV(DX12Context& ctx);
	void CreateMultiSampledRTV(DX12Context& ctx);

	// Depth stencil
	void CreateDSV(DX12Context& ctx);

private:
	uint32_t rtvIncrementSize_ = 0;

	// Swapchain RTV
	ID3D12DescriptorHeap* swapchainRTVHeap_ = nullptr;
	//std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, AppConfig::FrameCount> swapchainRTVHandles_ = {};
	//std::array<ID3D12Resource*, AppConfig::FrameCount> swapchainRenderTargets_ = { nullptr }; // TODO Set as DX12Buffer
	std::array<DX12Buffer, AppConfig::FrameCount> swapchainBuffers_ = {};
	std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, AppConfig::FrameCount> swapchainCPUHandles_ = {};

	// Offscreen Multisampled RTV (MSAA)
	DX12Image multiSampledImage_;
	CD3DX12_CPU_DESCRIPTOR_HANDLE multiSampledRTVHandle_ = {};
	ID3D12DescriptorHeap* multiSampledRTVHeap_ = nullptr;

	// Resolve target
	DX12Image singleSampledImage_;
	CD3DX12_CPU_DESCRIPTOR_HANDLE singleSampledRTVHandle_ = {};
	ID3D12DescriptorHeap* singleSampledRTVHeap_ = nullptr;

	// Depth stencil
	DX12Image depthImage_;
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle_;
	ID3D12DescriptorHeap* dsvHeap_ = nullptr;
};

#endif