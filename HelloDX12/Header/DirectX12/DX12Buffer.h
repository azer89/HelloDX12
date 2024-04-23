#ifndef HELLO_DX12_BUFFER
#define HELLO_DX12_BUFFER

#include "DX12Context.h"

class DX12Buffer
{
public:
	void CreateBuffer(DX12Context& ctx, uint64_t bufferSize);
	void CreateConstantBuffer(DX12Context& ctx, uint64_t bufferSize);
	void CreateVertexBuffer(DX12Context& ctx, void* data, uint64_t bufferSize, uint32_t stride);
	void CreateIndexBuffer(DX12Context& ctx, void* data, uint64_t bufferSize, DXGI_FORMAT format);
	void CreateImage(
		DX12Context& ctx,
		void* imageData,
		uint32_t width,
		uint32_t height,
		uint16_t mipmapCount,
		uint32_t bytesPerPixel,
		DXGI_FORMAT imageFormat,
		D3D12_RESOURCE_FLAGS flags);

	void UploadData(void* data);

	void Destroy();

private:
	void CreateUploadHeap(DX12Context& ctx,
		uint64_t bufferSize,
		uint16_t mipLevel,
		ID3D12Resource** bufferUploadHeap,
		D3D12MA::Allocation** bufferUploadHeapAllocation);

	static uint32_t GetConstantBufferByteSize(uint64_t byteSize);

public:
	uint64_t bufferSize_ = 0;
	ID3D12Resource* resource_ = nullptr;
	D3D12MA::Allocation* dmaAllocation_ = nullptr;
	
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_ = {};
	
	uint64_t constantBufferSize_ = 0;
	unsigned char* mappedData_ = nullptr;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress_ = 0;
};

#endif