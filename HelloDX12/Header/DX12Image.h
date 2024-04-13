#ifndef HELLO_DX12_IMAGE
#define HELLO_DX12_IMAGE

#include "DX12Context.h"

class DX12Image
{
public:
	DX12Image(DX12Context& ctx);
	~DX12Image() = default;

private:
	std::vector<UINT8> GenerateTextureData(DX12Context& ctx);

public:
	uint32_t width_ = 0;
	uint32_t height_ = 0;
	uint32_t pixelSize_ = 0;
	ComPtr<ID3D12Resource> image_ = nullptr;

private:
	// Note: ComPtr's are CPU objects but this resource needs to stay in scope until
	// the command list that references it has finished executing on the GPU.
	// We will flush the GPU at the end of this method to ensure the resource is not
	// prematurely destroyed.
	ComPtr<ID3D12Resource> textureUploadHeap_;
};

#endif