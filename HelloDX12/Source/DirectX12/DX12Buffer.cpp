#include "DX12Buffer.h"
#include "DX12Exception.h"
#include <cassert>

#include "d3dx12_resource_helpers.h"

void DX12Buffer::CreateVertexBuffer(DX12Context& ctx, void* data, uint32_t bufferSize, uint32_t stride)
{
	constexpr D3D12MA::ALLOCATION_DESC allocDesc = 
	{
		.HeapType = D3D12_HEAP_TYPE_DEFAULT
	};
	
	D3D12_RESOURCE_DESC resourceDesc = 
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = bufferSize,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	
	ID3D12Resource* vertexBufferPtr;
	ThrowIfFailed(ctx.dmaAllocator_->CreateResource(
		&allocDesc,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		&dmaAllocation_,
		IID_PPV_ARGS(&vertexBufferPtr)));
	resource_.Attach(vertexBufferPtr);

	resource_->SetName(L"Vertex_Buffer_Resource");
	dmaAllocation_->SetName(L"Vertex_Buffer_Allocation_DMA");

	// Upload heap
	constexpr D3D12MA::ALLOCATION_DESC uploadAllocDesc = 
	{
		.HeapType = D3D12_HEAP_TYPE_UPLOAD
	};
	
	D3D12_RESOURCE_DESC uploadResourceDesc = 
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = bufferSize,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};
	uploadResourceDesc.SampleDesc.Count = 1;
	uploadResourceDesc.SampleDesc.Quality = 0;

	ComPtr<ID3D12Resource> vBufferUploadHeap;
	D3D12MA::Allocation* vBufferUploadHeapAllocation = nullptr;
	ThrowIfFailed(ctx.dmaAllocator_->CreateResource(
		&uploadAllocDesc,
		&uploadResourceDesc, // Resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		&vBufferUploadHeapAllocation,
		IID_PPV_ARGS(&vBufferUploadHeap)));
	vBufferUploadHeap->SetName(L"Vertex_Buffer_Upload_Heap");
	vBufferUploadHeapAllocation->SetName(L"Vertex Buffer_Upload_Heap_Allocation_DMA");

	// Store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA vertexData = 
	{
		.pData = reinterpret_cast<BYTE*>(data), // Pointer to our vertex array
		.RowPitch = bufferSize, // Size of all our triangle vertex data
		.SlicePitch = bufferSize, // Also the size of our triangle vertex data
	};
	
	// Start recording 
	ctx.ResetCommandList();

	// Copy data
	UINT64 r = UpdateSubresources(
		ctx.GetCommandList(),
		resource_.Get(), 
		vBufferUploadHeap.Get(), 
		0, 
		0, 
		1, 
		&vertexData);
	assert(r);

	// Transition the vertex buffer data from copy destination state to vertex buffer state
	D3D12_RESOURCE_BARRIER vbBarrier = 
	{
		.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION
	};
	vbBarrier.Transition.pResource = resource_.Get();
	vbBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	vbBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	vbBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	ctx.GetCommandList()->ResourceBarrier(1, &vbBarrier);

	// End recording
	ctx.EndCommandListRecordingAndSubmit();

	// Release
	vBufferUploadHeapAllocation->Release();

	// Create view
	vertexBufferView_.BufferLocation = resource_->GetGPUVirtualAddress();
	vertexBufferView_.StrideInBytes = stride;
	vertexBufferView_.SizeInBytes = static_cast<UINT>(bufferSize);
}

void DX12Buffer::CreateIndexBuffer(DX12Context& ctx, void* data, uint32_t bufferSize, DXGI_FORMAT format)
{
	// Create default heap to hold index buffer
	constexpr D3D12MA::ALLOCATION_DESC indexBufferAllocDesc = 
	{
		.HeapType = D3D12_HEAP_TYPE_DEFAULT
	};
	
	D3D12_RESOURCE_DESC indexBufferResourceDesc = 
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = bufferSize,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};
	indexBufferResourceDesc.SampleDesc.Count = 1;
	indexBufferResourceDesc.SampleDesc.Quality = 0;

	ThrowIfFailed(ctx.dmaAllocator_->CreateResource(
		&indexBufferAllocDesc,
		&indexBufferResourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		&dmaAllocation_,
		IID_PPV_ARGS(&resource_)));
	resource_->SetName(L"Index_Buffer_Resource");
	dmaAllocation_->SetName(L"Index_Buffer_Allocation_DMA");

	// Upload heap
	constexpr D3D12MA::ALLOCATION_DESC iBufferUploadAllocDesc = 
	{
		.HeapType = D3D12_HEAP_TYPE_UPLOAD
	};
	D3D12_RESOURCE_DESC indexBufferUploadResourceDesc = 
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = bufferSize,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE,
	};
	indexBufferUploadResourceDesc.SampleDesc.Count = 1;
	indexBufferUploadResourceDesc.SampleDesc.Quality = 0;
	ComPtr<ID3D12Resource> iBufferUploadHeap;
	D3D12MA::Allocation* iBufferUploadHeapAllocation = nullptr;

	ThrowIfFailed(ctx.dmaAllocator_->CreateResource(
		&iBufferUploadAllocDesc,
		&indexBufferUploadResourceDesc, // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		&iBufferUploadHeapAllocation,
		IID_PPV_ARGS(&iBufferUploadHeap)));
	iBufferUploadHeap->SetName(L"Index_Buffer_Upload_Heap");
	iBufferUploadHeapAllocation->SetName(L"Index_Buffer_Upload_Heap_Allocation");

	// Store index buffer in upload heap
	D3D12_SUBRESOURCE_DATA indexData = 
	{
		.pData = data, // Pointer to our index array
		.RowPitch = bufferSize, // Size of all our index buffer
		.SlicePitch = bufferSize // Also the size of our index buffer
	};

	// Start recording 
	ctx.ResetCommandList();

	// Copy data
	UINT64 r = UpdateSubresources(ctx.GetCommandList(), resource_.Get(), iBufferUploadHeap.Get(), 0, 0, 1, &indexData);
	assert(r);

	// Transition the index buffer data from copy destination state to vertex buffer state
	D3D12_RESOURCE_BARRIER ibBarrier = 
	{
		ibBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION
	};
	ibBarrier.Transition.pResource = resource_.Get();
	ibBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	ibBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
	ibBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	ctx.GetCommandList()->ResourceBarrier(1, &ibBarrier);

	// End recording 
	ctx.EndCommandListRecordingAndSubmit();

	// Release
	iBufferUploadHeapAllocation->Release();

	// Create view
	indexBufferView_.BufferLocation = resource_->GetGPUVirtualAddress();
	indexBufferView_.Format = format;
	indexBufferView_.SizeInBytes = static_cast<UINT>(bufferSize);
}