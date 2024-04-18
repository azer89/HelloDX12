#ifndef HELLO_DX12_IMAGE
#define HELLO_DX12_IMAGE

#include "DX12Context.h"
#include "DX12Buffer.h"

class DX12Image
{
public:
	DX12Image();
	~DX12Image() = default;

	void Destroy()
	{
		buffer_.Destroy();
	}

	void Load(DX12Context& ctx, std::string filename);

	D3D12_STATIC_SAMPLER_DESC GetSampler();

private:
	std::vector<uint8_t> GenerateTextureData(DX12Context& ctx);

public:
	uint32_t width_ = 0;
	uint32_t height_ = 0;
	uint32_t pixelSize_ = 0;
	DXGI_FORMAT format_ = DXGI_FORMAT_UNKNOWN;

	DX12Buffer buffer_ = {};
};

#endif