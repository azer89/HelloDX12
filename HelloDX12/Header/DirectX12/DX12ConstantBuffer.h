#ifndef HELLO_DX12_CONSTANT_BUFFER
#define HELLO_DX12_CONSTANT_BUFFER

#include "DX12Context.h"
#include "DX12Exception.h"

class DX12ConstantBuffer
{
public:
	void Init(DX12Context& ctx, uint32_t bufferSize)
	{
		bufferSize_ = bufferSize;
		paddedBufferSize_ = GetConstantBufferByteSize(bufferSize_);

		const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		const D3D12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(paddedBufferSize_);
		ThrowIfFailed(ctx.GetDevice()->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&constantBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(resource_.ReleaseAndGetAddressOf())))

			// Mapping
			ThrowIfFailed(resource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData_)))

			// GPU virtual address
			gpuAddress_ = resource_->GetGPUVirtualAddress();
	}

	void UploadData(void* data)
	{
		memcpy(mappedData_, data, bufferSize_);
	}

private:
	static uint32_t GetConstantBufferByteSize(uint32_t byteSize)
	{
		// Constant buffers must be a multiple of the minimum hardware
		// allocation size (usually 256 bytes).  So round up to nearest
		// multiple of 256.  We do this by adding 255 and then masking off
		// the lower 2 bytes which store all bits < 256.
		// Example: Suppose byteSize = 300.
		// (300 + 255) & ~255
		// 555 & ~255
		// 0x022B & ~0x00ff
		// 0x022B & 0xff00
		// 0x0200
		// 512
		return (byteSize + 255) & ~255;
	}

public:
	uint32_t bufferSize_;
	uint32_t paddedBufferSize_;
	unsigned char* mappedData_;
	ComPtr<ID3D12Resource> resource_;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress_;
};

#endif