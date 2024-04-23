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
	void Load(DX12Context& ctx, std::string filename);
	void CreateColorAttachment(DX12Context& ctx);
	void CreateDepthAttachment(DX12Context& ctx);
	ID3D12Resource* GetResource() const { return buffer_.resource_; }
	D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDescription();
	D3D12_STATIC_SAMPLER_DESC GetSampler();

private:
	std::vector<uint8_t> GenerateCheckerboard(DX12Context& ctx);

public:
	uint32_t width_ = 0;
	uint32_t height_ = 0;
	uint32_t pixelSize_ = 0;
	uint32_t mipmapCount_ = 0;
	DXGI_FORMAT format_ = DXGI_FORMAT_UNKNOWN;

	DX12Buffer buffer_ = {};
};

#endif