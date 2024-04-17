#include "Mesh.h"
#include "ShapeGenerator.h"
#include "DX12Exception.h"

void Mesh::CreateCube(DX12Context& ctx)
{
	ShapeGenerator::Cube(vertices_, indices_);
	numVertices_ = indices_.size();

	CreateBuffers(ctx);

	image_ = std::make_unique<DX12Image>(ctx);
}

void Mesh::CreateBuffers(DX12Context& ctx)
{
	const uint64_t vertexBufferSize = sizeof(VertexData) * vertices_.size();

	// Note: using upload heaps to transfer static data like vert buffers is not 
	// recommended. Every time the GPU needs it, the upload heap will be marshalled 
	// over. Please read up on Default Heap usage. An upload heap is used here for 
	// code simplicity and because there are very few verts to actually transfer
	{
	auto vHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto vResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	ThrowIfFailed(ctx.GetDevice()->CreateCommittedResource(
		&vHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer_)));
	}

	// Copy the triangle data to the vertex buffer
	uint8_t* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU
	ThrowIfFailed(vertexBuffer_->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, vertices_.data(), vertexBufferSize);
	vertexBuffer_->Unmap(0, nullptr);

	// Initialize the vertex buffer view
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	vertexBufferView_.SizeInBytes = vertexBufferSize;

	const uint64_t indexBufferSize = sizeof(uint32_t) * indices_.size();

	auto iHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto iResourceDescription = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
	ThrowIfFailed(ctx.GetDevice()->CreateCommittedResource(
		&iHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&iResourceDescription,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuffer_)));

	// Copy the cube data to the vertex buffer.
	ThrowIfFailed(indexBuffer_->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, indices_.data(), indexBufferSize);
	indexBuffer_->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	indexBufferView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView_.SizeInBytes = indexBufferSize;
}