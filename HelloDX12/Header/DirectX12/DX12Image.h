#ifndef HELLO_DX12_IMAGE
#define HELLO_DX12_IMAGE

#include "DX12Context.h"
#include "DX12Buffer.h"

class DX12Image
{
public:
	DX12Image();
	~DX12Image() = default;
	void Destroy();
	void Load(DX12Context& ctx, const std::string& filename);
	void Load(DX12Context& ctx, void* data, uint32_t width, uint32_t height);
	void LoadHDR(DX12Context& ctx, const std::string& filename);
	void CreateCubemap(DX12Context& ctx, uint32_t width, uint32_t height);
	void CreateColorAttachment(DX12Context& ctx, uint32_t msaaCount);
	void CreateDepthAttachment(DX12Context& ctx, uint32_t msaaCount);
	ID3D12Resource* GetResource() const { return buffer_.resource_; }
	D3D12_STATIC_SAMPLER_DESC GetSampler();

	void UAVBarrier(ID3D12GraphicsCommandList* commandList);
	void TransitionCommand(
		ID3D12GraphicsCommandList* commandList,
		D3D12_RESOURCE_STATES afterState);

	static std::vector<uint8_t> GenerateCheckerboard(
		uint32_t width,
		uint32_t height,
		uint32_t pixelSize);

public:
	uint32_t width_ = 0;
	uint32_t height_ = 0;
	uint32_t pixelSize_ = 0;
	uint32_t mipmapCount_ = 0;
	uint32_t layerCount_ = 0;
	DXGI_FORMAT format_ = DXGI_FORMAT_UNKNOWN;

	DX12Buffer buffer_ = {};
};

#endif