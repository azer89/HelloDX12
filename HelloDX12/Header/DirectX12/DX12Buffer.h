#ifndef HELLO_DX12_BUFFER
#define HELLO_DX12_BUFFER

#include "DX12Context.h"


class DX12Buffer
{
public:
	void CreateVertexBuffer(DX12Context& ctx, void* data, uint64_t size);
	void CreateIndexBuffer(DX12Context& ctx, void* data, uint64_t size);

public:
	uint64_t size_;
	ComPtr<ID3D12Resource> resource_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_ = {};

};

#endif