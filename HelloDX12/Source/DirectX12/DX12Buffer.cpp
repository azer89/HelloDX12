#include "DX12Buffer.h"
#include "DX12Exception.h"
#include "Utility.h"

#include "d3dx12_resource_helpers.h"

#include <cassert>

void DX12Buffer::Destroy()
{
	if (isSwapchainBuffer_)
	{
		resource_->Release();
	}

	if (dmaAllocation_ != nullptr)
	{
		resource_->Release();
		dmaAllocation_->Release();
		resource_ = nullptr;
		dmaAllocation_ = nullptr;
	}
}

void DX12Buffer::CreateHostVisibleBuffer(DX12Context& ctx, uint32_t elementCount, uint64_t bufferSize, uint32_t stride)
{
	bufferSize_ = bufferSize;
	state_ = D3D12_RESOURCE_STATE_GENERIC_READ;
	srvDescription_ = GetSRVDescriptionFromBuffer(elementCount, stride);

	constexpr D3D12MA::ALLOCATION_DESC constantBufferUploadAllocDesc =
	{
		.HeapType = D3D12_HEAP_TYPE_UPLOAD
	};

	const D3D12_RESOURCE_DESC resourceDesc =
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = bufferSize_,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc =
		{
			.Count = 1,
			.Quality = 0
		},
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};

	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&constantBufferUploadAllocDesc,
		&resourceDesc,
		state_,
		nullptr,
		&dmaAllocation_,
		IID_PPV_ARGS(&resource_)));
	SetName("Host_Visible_Buffer");

	// Mapping
	ThrowIfFailed(resource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData_)));

	// GPU virtual address
	gpuAddress_ = resource_->GetGPUVirtualAddress();
}

void DX12Buffer::CreateDeviceOnlyBuffer(
	DX12Context& ctx,
	void* data,
	uint32_t elementCount,
	uint64_t bufferSize,
	uint32_t stride)
{
	bufferSize_ = bufferSize;
	state_ = D3D12_RESOURCE_STATE_COMMON;
	srvDescription_ = GetSRVDescriptionFromBuffer(elementCount, stride);

	constexpr D3D12MA::ALLOCATION_DESC allocDesc =
	{
		.HeapType = D3D12_HEAP_TYPE_DEFAULT
	};

	const D3D12_RESOURCE_DESC resourceDesc =
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = bufferSize_,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc =
		{
			.Count = 1,
			.Quality = 0
		},
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};

	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&allocDesc,
		&resourceDesc,
		state_,
		nullptr,
		&dmaAllocation_,
		IID_PPV_ARGS(&resource_)));
	SetName("Device_Only_Buffer");

	// Upload heap
	ID3D12Resource* bufferUploadHeap;
	D3D12MA::Allocation* bufferUploadHeapAllocation;
	CreateUploadHeap(ctx, static_cast<uint64_t>(bufferSize_), 1, &bufferUploadHeap, &bufferUploadHeapAllocation);

	const D3D12_SUBRESOURCE_DATA subresourceData =
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

	// Transition
	TransitionCommand(ctx.GetCommandList(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

	// End recording
	ctx.SubmitCommandListAndWaitForGPU();

	// Release
	bufferUploadHeap->Release();
	bufferUploadHeapAllocation->Release();
}

void DX12Buffer::CreateConstantBuffer(DX12Context& ctx, uint64_t bufferSize)
{
	bufferSize_ = bufferSize;
	constantBufferSize_ = GetConstantBufferByteSize(bufferSize_);
	state_ = D3D12_RESOURCE_STATE_GENERIC_READ;

	constexpr D3D12MA::ALLOCATION_DESC constantBufferUploadAllocDesc =
	{
		.HeapType = D3D12_HEAP_TYPE_UPLOAD
	};

	const D3D12_RESOURCE_DESC constantBufferResourceDesc =
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = constantBufferSize_,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc =
		{
			.Count = 1,
			.Quality = 0
		},
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};

	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&constantBufferUploadAllocDesc,
		&constantBufferResourceDesc,
		state_,
		nullptr,
		&dmaAllocation_,
		IID_PPV_ARGS(&resource_)));
	SetName("Constant_Buffer");

	// Mapping
	ThrowIfFailed(resource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData_)));

	// GPU virtual address
	gpuAddress_ = resource_->GetGPUVirtualAddress();
}

void DX12Buffer::UploadData(void* data) const
{
	memcpy(mappedData_, data, bufferSize_);
}

void DX12Buffer::CreateVertexBuffer(DX12Context& ctx, void* data, uint64_t bufferSize, uint32_t stride)
{
	bufferSize_ = bufferSize;
	state_ = D3D12_RESOURCE_STATE_COMMON;

	constexpr D3D12MA::ALLOCATION_DESC allocDesc =
	{
		.HeapType = D3D12_HEAP_TYPE_DEFAULT
	};

	const D3D12_RESOURCE_DESC resourceDesc =
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = bufferSize_,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc =
		{
			.Count = 1,
			.Quality = 0
		},
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};
	

	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&allocDesc,
		&resourceDesc,
		state_,
		nullptr,
		&dmaAllocation_,
		IID_PPV_ARGS(&resource_)));
	SetName("Vertex_Buffer");

	// Upload heap
	ID3D12Resource* bufferUploadHeap;
	D3D12MA::Allocation* bufferUploadHeapAllocation;
	CreateUploadHeap(ctx, static_cast<uint64_t>(bufferSize_), 1, &bufferUploadHeap, &bufferUploadHeapAllocation);

	// Store vertex buffer in upload heap
	const D3D12_SUBRESOURCE_DATA subresourceData =
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

	// Transition
	TransitionCommand(ctx.GetCommandList(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	// End recording
	ctx.SubmitCommandListAndWaitForGPU();

	// Release
	bufferUploadHeap->Release();
	bufferUploadHeapAllocation->Release();

	// Create view
	vertexBufferView_ =
	{
		.BufferLocation = resource_->GetGPUVirtualAddress(),
		.SizeInBytes = static_cast<uint32_t>(bufferSize_),
		.StrideInBytes = stride,
	};
}

void DX12Buffer::CreateIndexBuffer(DX12Context& ctx, void* data, uint64_t bufferSize, DXGI_FORMAT format)
{
	bufferSize_ = bufferSize;
	state_ = D3D12_RESOURCE_STATE_COMMON;

	// Create default heap to hold index buffer
	constexpr D3D12MA::ALLOCATION_DESC allocDesc =
	{
		.HeapType = D3D12_HEAP_TYPE_DEFAULT
	};

	const D3D12_RESOURCE_DESC resourceDesc =
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = bufferSize_,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc =
		{
			.Count = 1,
			.Quality = 0
		},
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};

	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&allocDesc,
		&resourceDesc,
		state_,
		nullptr,
		&dmaAllocation_,
		IID_PPV_ARGS(&resource_)));
	SetName("Index_Buffer");

	// Upload heap
	ID3D12Resource* bufferUploadHeap;
	D3D12MA::Allocation* bufferUploadHeapAllocation;
	CreateUploadHeap(ctx, static_cast<uint64_t>(bufferSize_), 1, &bufferUploadHeap, &bufferUploadHeapAllocation);

	// Store index buffer in upload heap
	const D3D12_SUBRESOURCE_DATA subresourceData =
	{
		.pData = data, // Pointer to our index array
		.RowPitch = static_cast<LONG_PTR>(bufferSize_), // Size of all our index buffer
		.SlicePitch = static_cast<LONG_PTR>(bufferSize_) // Also the size of our index buffer
	};

	// Start recording 
	ctx.ResetCommandList();

	// Copy data
	const uint64_t r = UpdateSubresources(
		ctx.GetCommandList(),
		resource_,
		bufferUploadHeap,
		0,
		0,
		1,
		&subresourceData);
	assert(r);

	// Transition
	TransitionCommand(ctx.GetCommandList(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	// End recording 
	ctx.SubmitCommandListAndWaitForGPU();

	// Release
	bufferUploadHeap->Release();
	bufferUploadHeapAllocation->Release();

	// Create view
	indexBufferView_ =
	{
		.BufferLocation = resource_->GetGPUVirtualAddress(),
		.SizeInBytes = static_cast<uint32_t>(bufferSize_),
		.Format = format,
	};
}

void DX12Buffer::CreateImage(
	DX12Context& ctx,
	uint32_t width,
	uint32_t height,
	uint32_t mipmapCount,
	uint32_t layerCount,
	DXGI_FORMAT imageFormat,
	D3D12_RESOURCE_FLAGS flags)
{
	state_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	srvDescription_ = GetSRVDescriptionFromImage(imageFormat, layerCount, mipmapCount);

	const D3D12_RESOURCE_DESC textureDesc =
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		.Alignment = 0,
		.Width = width,
		.Height = height,
		.DepthOrArraySize = static_cast<uint16_t>(layerCount),
		.MipLevels = static_cast<uint16_t>(mipmapCount),
		.Format = imageFormat,
		.SampleDesc =
		{
			.Count = 1,
			.Quality = 0
		},
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = flags
	};

	constexpr D3D12MA::ALLOCATION_DESC textureAllocDesc =
	{
		.HeapType = D3D12_HEAP_TYPE_DEFAULT
	};
	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&textureAllocDesc,
		&textureDesc,
		state_,
		nullptr, // pOptimizedClearValue
		&dmaAllocation_,
		IID_PPV_ARGS(&resource_)));
	SetName("Image");
}

void DX12Buffer::CreateColorAttachment(
	DX12Context& ctx,
	uint32_t width,
	uint32_t height,
	uint32_t mipmapCount,
	uint32_t msaaCount,
	DXGI_FORMAT imageFormat,
	D3D12_RESOURCE_FLAGS flags)
{
	state_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	srvDescription_ = GetSRVDescriptionFromImage(imageFormat, 1, mipmapCount);

	const D3D12_RESOURCE_DESC textureDesc =
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		.Alignment = 0,
		.Width = width,
		.Height = height,
		.DepthOrArraySize = 1,
		.MipLevels = static_cast<uint16_t>(mipmapCount),
		.Format = imageFormat,
		.SampleDesc =
		{
			.Count = msaaCount,
			.Quality = 0
		},
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = flags
	};

	const D3D12_CLEAR_VALUE clearValue
	{
		.Format = imageFormat,
		// TODO
		.Color{
			AppConfig::ClearColor[0],
			AppConfig::ClearColor[1],
			AppConfig::ClearColor[2],
			AppConfig::ClearColor[3]}
	};

	constexpr D3D12MA::ALLOCATION_DESC textureAllocDesc =
	{
		.HeapType = D3D12_HEAP_TYPE_DEFAULT
	};
	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&textureAllocDesc,
		&textureDesc,
		state_,
		&clearValue, // pOptimizedClearValue
		&dmaAllocation_,
		IID_PPV_ARGS(&resource_)));
	SetName("Color_Attachment");
}

void DX12Buffer::CreateDepthAttachment(
	DX12Context& ctx,
	uint32_t width,
	uint32_t height,
	uint32_t msaaCount,
	DXGI_FORMAT imageFormat) // DXGI_FORMAT_D32_FLOAT
{
	state_ = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	srvDescription_ = GetSRVDescriptionFromImage(imageFormat, 1, 1);

	constexpr float defaultDepthValue = 0.0f; // 0.0f for reverse z, 1.0f otherwise
	D3D12_CLEAR_VALUE clearValue =
	{
		.Format = imageFormat
	};
	clearValue.DepthStencil.Depth = defaultDepthValue;
	clearValue.DepthStencil.Stencil = 0;

	constexpr D3D12MA::ALLOCATION_DESC depthStencilAllocDesc =
	{
		.HeapType = D3D12_HEAP_TYPE_DEFAULT
	};
	const D3D12_RESOURCE_DESC depthStencilResourceDesc =
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		.Alignment = 0,
		.Width = width,
		.Height = height,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = imageFormat,
		.SampleDesc =
		{
			.Count = msaaCount,
			.Quality = 0
		},
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	};

	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&depthStencilAllocDesc,
		&depthStencilResourceDesc,
		state_,
		&clearValue,
		&dmaAllocation_,
		IID_PPV_ARGS(&resource_)
	));
	SetName("Depth_Stencil_Attachment");
}

void DX12Buffer::CreateImageFromData(
DX12Context& ctx,
	void* imageData,
	uint32_t width,
	uint32_t height,
	uint32_t mipmapCount,
	uint32_t bytesPerPixel,
	DXGI_FORMAT imageFormat,
	D3D12_RESOURCE_FLAGS flags)
{
	state_ = D3D12_RESOURCE_STATE_COPY_DEST;
	srvDescription_ = GetSRVDescriptionFromImage(imageFormat, 1, mipmapCount);

	const D3D12_RESOURCE_DESC textureDesc =
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		.Alignment = 0,
		.Width = width,
		.Height = height,
		.DepthOrArraySize = 1,
		.MipLevels = static_cast<uint16_t>(mipmapCount),
		.Format = imageFormat,
		.SampleDesc =
		{
			.Count = 1,
			.Quality = 0
		},
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = flags
	};

	constexpr D3D12MA::ALLOCATION_DESC textureAllocDesc =
	{
		.HeapType = D3D12_HEAP_TYPE_DEFAULT
	};
	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&textureAllocDesc,
		&textureDesc,
		state_,
		nullptr, // pOptimizedClearValue
		&dmaAllocation_,
		IID_PPV_ARGS(&resource_)));
	SetName("Image_From_Data");

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

	const uint32_t imageBytesPerRow{ width * bytesPerPixel };
	const D3D12_SUBRESOURCE_DATA subresourceData =
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

	// Transition
	TransitionCommand(ctx.GetCommandList(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// End recording 
	ctx.SubmitCommandListAndWaitForGPU();

	// Release
	bufferUploadHeap->Release();
	bufferUploadHeapAllocation->Release();
}

void DX12Buffer::SetAsSwapchainBuffer(DX12Context& ctx, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle, uint32_t frameIndex)
{
	resource_ = ctx.GetSwapchainResource(frameIndex);
	ctx.GetDevice()->CreateRenderTargetView(resource_, nullptr, rtvHandle);
	state_ = D3D12_RESOURCE_STATE_PRESENT;
	isSwapchainBuffer_ = true;
}

void DX12Buffer::CreateUploadHeap(DX12Context& ctx,
	uint64_t bufferSize,
	uint32_t mipLevel,
	ID3D12Resource** bufferUploadHeap,
	D3D12MA::Allocation** bufferUploadHeapAllocation)
{
	constexpr D3D12MA::ALLOCATION_DESC uploadAllocDesc =
	{
		.HeapType = D3D12_HEAP_TYPE_UPLOAD
	};

	const D3D12_RESOURCE_DESC uploadResourceDesc =
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = bufferSize,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = static_cast<uint16_t>(mipLevel),
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc =
		{
			.Count = 1,
			.Quality = 0
		},
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};

	ThrowIfFailed(ctx.GetDMAAllocator()->CreateResource(
		&uploadAllocDesc,
		&uploadResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		bufferUploadHeapAllocation,
		IID_PPV_ARGS(bufferUploadHeap)));
}

void DX12Buffer::UAVBarrier(ID3D12GraphicsCommandList* commandList)
{
	const auto barrier{ CD3DX12_RESOURCE_BARRIER::UAV(resource_) };
	commandList->ResourceBarrier(1, &barrier);
}

void DX12Buffer::TransitionCommand(
	ID3D12GraphicsCommandList* commandList,
	D3D12_RESOURCE_STATES afterState)
{
	if (state_ == afterState)
	{
		return;
	}

	TransitionCommand(commandList, state_, afterState);
}

void DX12Buffer::TransitionCommand(
	ID3D12GraphicsCommandList* commandList,
	D3D12_RESOURCE_STATES beforeState,
	D3D12_RESOURCE_STATES afterState)
{
	const auto barrier{
		CD3DX12_RESOURCE_BARRIER::Transition(
			resource_,
			beforeState,
			afterState)};

	commandList->ResourceBarrier(1, &barrier);

	state_ = afterState;
}

D3D12_SHADER_RESOURCE_VIEW_DESC DX12Buffer::GetSRVDescriptionFromBuffer(uint32_t elementCount, uint32_t stride) const
{
	// SRV Description
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
	{
		.Format = DXGI_FORMAT_UNKNOWN,
		.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	};

	srvDesc.Buffer =
	{
		.FirstElement = 0,
		.NumElements = static_cast<UINT>(elementCount),
		.StructureByteStride = stride,
		.Flags = D3D12_BUFFER_SRV_FLAG_NONE
	};

	return srvDesc;
}

D3D12_SHADER_RESOURCE_VIEW_DESC DX12Buffer::GetSRVDescriptionFromImage(DXGI_FORMAT format, uint32_t layerCount, uint32_t mipmapCount) const
{
	D3D12_SRV_DIMENSION srvDim;
	switch (layerCount)
	{
		case 1:  srvDim = D3D12_SRV_DIMENSION_TEXTURE2D; break;
		case 6:  srvDim = D3D12_SRV_DIMENSION_TEXTURECUBE; break;
		default: srvDim = D3D12_SRV_DIMENSION_TEXTURE2DARRAY; break;
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
	{
		.Format = format, // Image format
		.ViewDimension = srvDim,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	};
	srvDesc.Texture2D.MipLevels = mipmapCount;

	return srvDesc;
}

void DX12Buffer::SetName(const std::string& objectName) const
{
	if (resource_ == nullptr)
	{
		return;
	}

	ThrowIfFailed(resource_->SetName(Utility::WStringConvert(objectName).c_str()));

	const std::string dmaName{ objectName + "_Allocation_DMA" };
	dmaAllocation_->SetName(Utility::WStringConvert(dmaName).c_str());
}

D3D12_UNORDERED_ACCESS_VIEW_DESC DX12Buffer::GetUAVDescription(uint32_t mipLevel) const
{
	const D3D12_RESOURCE_DESC resourceDescription{ resource_->GetDesc() };
	assert(resourceDescription.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDescription{};
	uavDescription.Format = resourceDescription.Format;
	if (resourceDescription.DepthOrArraySize > 1) 
	{
		uavDescription.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDescription.Texture2DArray.MipSlice = mipLevel;
		uavDescription.Texture2DArray.FirstArraySlice = 0;
		uavDescription.Texture2DArray.ArraySize = resourceDescription.DepthOrArraySize;
	}
	else 
	{
		uavDescription.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDescription.Texture2D.MipSlice = mipLevel;
	}
	return uavDescription;
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