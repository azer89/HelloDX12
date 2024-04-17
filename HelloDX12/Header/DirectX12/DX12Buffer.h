#ifndef HELLO_DX12_BUFFER
#define HELLO_DX12_BUFFER

#include "DX12Context.h"

class DX12Buffer
{
public:
	void CreateVertexBuffer(DX12Context& ctx, void* data, uint32_t bufferSize, uint32_t stride);
	void CreateIndexBuffer(DX12Context& ctx, void* data, uint32_t bufferSize, DXGI_FORMAT format);
	void CreateImage(DX12Context& ctx, void* imageData, uint32_t width, uint32_t height, uint32_t bytesPerPixel, DXGI_FORMAT imageFormat);

	void Destroy()
	{
		if (dmaAllocation_ != nullptr)
		{
			dmaAllocation_->Release();
			resource_ = nullptr;
			dmaAllocation_ = nullptr;
		}
	}

public:
	uint64_t size_;
	ComPtr<ID3D12Resource> resource_ = nullptr;
	D3D12MA::Allocation* dmaAllocation_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_ = {};
};

#endif