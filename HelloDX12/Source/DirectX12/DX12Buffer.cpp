#include "DX12Buffer.h"
#include "DX12Exception.h"
#include <cassert>

#include "d3dx12_resource_helpers.h"

void DX12Buffer::Destroy()
{
	if (dmaAllocation_ != nullptr)
	{
		resource_->Release();
		dmaAllocation_->Release();
		resource_ = nullptr;
		dmaAllocation_ = nullptr;
	}
}

void DX12Buffer::CreateBuffer(DX12Context& ctx, uint64_t bufferSize)
{
	bufferSize_ = bufferSize;

	D3D12MA::ALLOCATION_DESC constantBufferUploadAllocDesc =
	{
		.HeapType = D3D12_HEAP_TYPE_UPLOAD
	};

	D3D12_RESOURCE_DESC resourceDesc =
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = bufferSize_,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;

	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&constantBufferUploadAllocDesc,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		&dmaAllocation_,
		IID_PPV_ARGS(&resource_)));
	resource_->SetName(L"Buffer");
	dmaAllocation_->SetName(L"Buffer_Allocation_DMA");

	// Mapping
	ThrowIfFailed(resource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData_)))

	// GPU virtual address
	gpuAddress_ = resource_->GetGPUVirtualAddress();
}

void DX12Buffer::CreateConstantBuffer(DX12Context& ctx, uint64_t bufferSize)
{
	bufferSize_ = bufferSize;
	constantBufferSize_ = GetConstantBufferByteSize(bufferSize_);

	D3D12MA::ALLOCATION_DESC constantBufferUploadAllocDesc = 
	{
		.HeapType = D3D12_HEAP_TYPE_UPLOAD
	};

	D3D12_RESOURCE_DESC constantBufferResourceDesc = 
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = constantBufferSize_,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};
	constantBufferResourceDesc.SampleDesc.Count = 1;
	constantBufferResourceDesc.SampleDesc.Quality = 0;

	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&constantBufferUploadAllocDesc,
		&constantBufferResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		&dmaAllocation_,
		IID_PPV_ARGS(&resource_)));
	resource_->SetName(L"Constant_Buffer");
	dmaAllocation_->SetName(L"Constant_Buffer_Allocation_DMA");

	// Mapping
	ThrowIfFailed(resource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData_)))

	// GPU virtual address
	gpuAddress_ = resource_->GetGPUVirtualAddress();
}

void DX12Buffer::UploadData(void* data)
{
	memcpy(mappedData_, data, bufferSize_);
}

void DX12Buffer::CreateVertexBuffer(DX12Context& ctx, void* data, uint64_t bufferSize, uint32_t stride)
{
	bufferSize_ = bufferSize;

	constexpr D3D12MA::ALLOCATION_DESC allocDesc = 
	{
		.HeapType = D3D12_HEAP_TYPE_DEFAULT
	};
	
	D3D12_RESOURCE_DESC resourceDesc = 
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = bufferSize_,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;

	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&allocDesc,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		&dmaAllocation_,
		IID_PPV_ARGS(&resource_)))

	resource_->SetName(L"Vertex_Buffer_Resource");
	dmaAllocation_->SetName(L"Vertex_Buffer_Allocation_DMA");

	// Upload heap
	ID3D12Resource* bufferUploadHeap;
	D3D12MA::Allocation* bufferUploadHeapAllocation;
	CreateUploadHeap(ctx, static_cast<uint64_t>(bufferSize_), 1, &bufferUploadHeap, &bufferUploadHeapAllocation);
	bufferUploadHeap->SetName(L"Vertex_Buffer_Upload_Heap");
	bufferUploadHeapAllocation->SetName(L"Vertex Buffer_Upload_Heap_Allocation_DMA");

	// Store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA subresourceData = 
	{
		.pData = reinterpret_cast<BYTE*>(data), // Pointer to our vertex array
		.RowPitch = static_cast<LONG_PTR>(bufferSize_), // Size of all our triangle vertex data
		.SlicePitch = static_cast<LONG_PTR>(bufferSize_), // Also the size of our triangle vertex data
	};
	
	// Start recording 
	ctx.ResetCommandList();

	// Copy data
	uint64_t r = UpdateSubresources(
		ctx.GetCommandList(),
		resource_, 
		bufferUploadHeap, 
		0, 
		0, 
		1, 
		&subresourceData);
	assert(r);

	// Transition the vertex buffer data from copy destination state to vertex buffer state
	D3D12_RESOURCE_BARRIER barrier = 
	{
		.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION
	};
	barrier.Transition.pResource = resource_;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	ctx.GetCommandList()->ResourceBarrier(1, &barrier);

	// End recording
	ctx.SubmitCommandListAndWaitForGPU();

	// Release
	bufferUploadHeap->Release();
	bufferUploadHeapAllocation->Release();

	// Create view
	vertexBufferView_.BufferLocation = resource_->GetGPUVirtualAddress();
	vertexBufferView_.StrideInBytes = stride;
	vertexBufferView_.SizeInBytes = static_cast<uint32_t>(bufferSize_);
}

void DX12Buffer::CreateIndexBuffer(DX12Context& ctx, void* data, uint64_t bufferSize, DXGI_FORMAT format)
{
	bufferSize_ = bufferSize;

	// Create default heap to hold index buffer
	constexpr D3D12MA::ALLOCATION_DESC allocDesc = 
	{
		.HeapType = D3D12_HEAP_TYPE_DEFAULT
	};
	
	D3D12_RESOURCE_DESC resourceDesc = 
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = bufferSize_,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;

	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&allocDesc,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		&dmaAllocation_,
		IID_PPV_ARGS(&resource_)))
	resource_->SetName(L"Index_Buffer_Resource");
	dmaAllocation_->SetName(L"Index_Buffer_Allocation_DMA");

	// Upload heap
	ID3D12Resource* bufferUploadHeap;
	D3D12MA::Allocation* bufferUploadHeapAllocation;
	CreateUploadHeap(ctx, static_cast<uint64_t>(bufferSize_), 1, &bufferUploadHeap, &bufferUploadHeapAllocation);
	bufferUploadHeap->SetName(L"Index_Buffer_Upload_Heap");
	bufferUploadHeapAllocation->SetName(L"Index_Buffer_Upload_Heap_Allocation");

	// Store index buffer in upload heap
	D3D12_SUBRESOURCE_DATA subresourceData = 
	{
		.pData = data, // Pointer to our index array
		.RowPitch = static_cast<LONG_PTR>(bufferSize_), // Size of all our index buffer
		.SlicePitch = static_cast<LONG_PTR>(bufferSize_) // Also the size of our index buffer
	};

	// Start recording 
	ctx.ResetCommandList();

	// Copy data
	uint64_t r = UpdateSubresources(
		ctx.GetCommandList(),
		resource_,
		bufferUploadHeap,
		0,
		0,
		1,
		&subresourceData);
	assert(r);

	// Transition the index buffer data from copy destination state to vertex buffer state
	D3D12_RESOURCE_BARRIER barrier = 
	{
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION
	};
	barrier.Transition.pResource = resource_;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	ctx.GetCommandList()->ResourceBarrier(1, &barrier);

	// End recording 
	ctx.SubmitCommandListAndWaitForGPU();

	// Release
	bufferUploadHeap->Release();
	bufferUploadHeapAllocation->Release();

	// Create view
	indexBufferView_.BufferLocation = resource_->GetGPUVirtualAddress();
	indexBufferView_.Format = format;
	indexBufferView_.SizeInBytes = static_cast<uint32_t>(bufferSize_);
}

void DX12Buffer::CreateImage(
	DX12Context& ctx,
	uint32_t width,
	uint32_t height,
	uint16_t mipmapCount,
	uint32_t bytesPerPixel,
	DXGI_FORMAT imageFormat,
	D3D12_RESOURCE_FLAGS flags)
{
	D3D12_RESOURCE_DESC textureDesc =
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		.Alignment = 0,
		.Width = width,
		.Height = height,
		.DepthOrArraySize = 1,
		.MipLevels = mipmapCount,
		.Format = imageFormat,
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = flags
	};
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	D3D12MA::ALLOCATION_DESC textureAllocDesc =
	{
		.HeapType = D3D12_HEAP_TYPE_DEFAULT
	};
	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&textureAllocDesc,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr, // pOptimizedClearValue
		&dmaAllocation_,
		IID_PPV_ARGS(&resource_)))
		resource_->SetName(L"Texture");
	dmaAllocation_->SetName(L"Texture_Allocation_DMA");
}

void DX12Buffer::CreateImageFromData(
	DX12Context& ctx,
	void* imageData,
	uint32_t width,
	uint32_t height,
	uint16_t mipmapCount,
	uint32_t bytesPerPixel,
	DXGI_FORMAT imageFormat,
	D3D12_RESOURCE_FLAGS flags)
{
	D3D12_RESOURCE_DESC textureDesc =
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		.Alignment = 0,
		.Width = width,
		.Height = height,
		.DepthOrArraySize = 1,
		.MipLevels = mipmapCount,
		.Format = imageFormat,
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = flags
	};
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	
	D3D12MA::ALLOCATION_DESC textureAllocDesc = 
	{
		.HeapType = D3D12_HEAP_TYPE_DEFAULT
	};
	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&textureAllocDesc,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr, // pOptimizedClearValue
		&dmaAllocation_,
		IID_PPV_ARGS(&resource_)))
	resource_->SetName(L"Texture");
	dmaAllocation_->SetName(L"Texture_Allocation_DMA");

	ctx.GetDevice()->GetCopyableFootprints(
		&textureDesc,
		0, // FirstSubresource
		1, // NumSubresources
		0, // BaseOffset
		nullptr, // pLayouts
		nullptr, // pNumRows
		nullptr, // pRowSizeInBytes
		&bufferSize_); // pTotalBytes

	// Upload heap
	ID3D12Resource* bufferUploadHeap;
	D3D12MA::Allocation* bufferUploadHeapAllocation;
	CreateUploadHeap(ctx, bufferSize_, 1, &bufferUploadHeap, &bufferUploadHeapAllocation);
	bufferUploadHeap->SetName(L"Image_Upload_Heap");
	bufferUploadHeapAllocation->SetName(L"Image_Upload_Heap_Allocation");

	const uint32_t imageBytesPerRow = width * bytesPerPixel;
	D3D12_SUBRESOURCE_DATA subresourceData = 
	{
		.pData = imageData,
		.RowPitch = imageBytesPerRow,
		.SlicePitch = static_cast<LONG_PTR>(imageBytesPerRow * textureDesc.Height)
	};

	// Start recording 
	ctx.ResetCommandList();

	UpdateSubresources(
		ctx.GetCommandList(), 
		resource_, 
		bufferUploadHeap, 
		0, 
		0, 
		1, 
		&subresourceData);

	D3D12_RESOURCE_BARRIER barrier = 
	{
		.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION
	};
	barrier.Transition.pResource = resource_;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	ctx.GetCommandList()->ResourceBarrier(1, &barrier);

	// End recording 
	ctx.SubmitCommandListAndWaitForGPU();

	// Release
	bufferUploadHeap->Release();
	bufferUploadHeapAllocation->Release();
}

void DX12Buffer::CreateUploadHeap(DX12Context& ctx,
	uint64_t bufferSize,
	uint16_t mipLevel,
	ID3D12Resource** bufferUploadHeap,
	D3D12MA::Allocation** bufferUploadHeapAllocation)
{
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
		.MipLevels = mipLevel,
		.Format = DXGI_FORMAT_UNKNOWN,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};
	uploadResourceDesc.SampleDesc.Count = 1;
	uploadResourceDesc.SampleDesc.Quality = 0;

	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&uploadAllocDesc,
		&uploadResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		bufferUploadHeapAllocation,
		IID_PPV_ARGS(bufferUploadHeap)))
}

void DX12Buffer::CreateDepthStencil(
	DX12Context& ctx,
	uint32_t width,
	uint32_t height)
{
	constexpr DXGI_FORMAT dsFormat = DXGI_FORMAT_D32_FLOAT;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = 
	{
		.Format = dsFormat
	};
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	D3D12MA::ALLOCATION_DESC depthStencilAllocDesc = 
	{
		.HeapType = D3D12_HEAP_TYPE_DEFAULT
	};
	D3D12_RESOURCE_DESC depthStencilResourceDesc = 
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		.Alignment = 0,
		.Width = width,
		.Height = height,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = dsFormat,
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	};
	depthStencilResourceDesc.SampleDesc.Count = 1;
	depthStencilResourceDesc.SampleDesc.Quality = 0;
	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&depthStencilAllocDesc,
		&depthStencilResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		&dmaAllocation_,
		IID_PPV_ARGS(&resource_)
	))
	ThrowIfFailed(resource_->SetName(L"Depth_Stencil_Resource"))
	dmaAllocation_->SetName(L"Depth_Stencil_Allocation_DMA");
}

uint32_t DX12Buffer::GetConstantBufferByteSize(uint64_t byteSize)
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
