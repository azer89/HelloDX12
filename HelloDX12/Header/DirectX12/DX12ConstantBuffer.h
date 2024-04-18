#ifndef HELLO_DX12_CONSTANT_BUFFER
#define HELLO_DX12_CONSTANT_BUFFER

#include "DX12Context.h"
#include "DX12Exception.h"

class DX12ConstantBuffer
{
public:
	void Init(DX12Context& ctx, uint32_t bufferSize);

	void UploadData(void* data);

private:
	static uint32_t GetConstantBufferByteSize(uint32_t byteSize);

public:
	uint32_t bufferSize_;
	uint32_t paddedBufferSize_;
	unsigned char* mappedData_;
	ComPtr<ID3D12Resource> resource_;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress_;
};

#endif