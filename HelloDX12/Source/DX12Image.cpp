#include "DX12Image.h"
#include "DX12Exception.h"

DX12Image::DX12Image(DX12Context& ctx)
{
	// TODO Temporary
	width_ = 256;
	height_ = 256;
	pixelSize_ = 4;

	// Create the texture.
	{
		// Describe and create a Texture2D.
		D3D12_RESOURCE_DESC textureDesc =
		{
			.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			.Width = width_,
			.Height = height_,
			.MipLevels = 1,
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.Flags = D3D12_RESOURCE_FLAG_NONE
		};
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;

		{
			auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			ThrowIfFailed(ctx.GetDevice()->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&textureDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&image_)));
		}

		const uint64_t uploadBufferSize = GetRequiredIntermediateSize(image_.Get(), 0, 1);

		// Create the GPU upload buffer
		{
			auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
			ThrowIfFailed(ctx.GetDevice()->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&textureUploadHeap_)));
		}

		// Copy data to the intermediate upload heap and then schedule a copy 
		// from the upload heap to the Texture2D.
		std::vector<uint8_t> texture = GenerateTextureData(ctx);

		D3D12_SUBRESOURCE_DATA textureData =
		{
			.pData = &texture[0],
			.RowPitch = width_ * pixelSize_,
		};
		textureData.SlicePitch = textureData.RowPitch * height_;

		UpdateSubresources(ctx.GetCommandList(), image_.Get(), textureUploadHeap_.Get(), 0, 0, 1, &textureData);
		{
			auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(image_.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			ctx.GetCommandList()->ResourceBarrier(1, &resourceBarrier);
		}
	}
}

// Generate a simple black and white checkerboard texture.
std::vector<UINT8> DX12Image::GenerateTextureData(DX12Context& ctx)
{
	const uint32_t rowPitch = width_ * pixelSize_;
	const uint32_t cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
	const uint32_t cellHeight = width_ >> 3;    // The height of a cell in the checkerboard texture.
	const uint32_t textureSize = rowPitch * height_;

	std::vector<uint8_t> data(textureSize);
	uint8_t* pData = &data[0];

	for (uint32_t n = 0; n < textureSize; n += pixelSize_)
	{
		uint32_t x = n % rowPitch;
		uint32_t y = n / rowPitch;
		uint32_t i = x / cellPitch;
		uint32_t j = y / cellHeight;

		if (i % 2 == j % 2)
		{
			pData[n] = 0x00;        // R
			pData[n + 1] = 0x00;    // G
			pData[n + 2] = 0x00;    // B
			pData[n + 3] = 0xff;    // A
		}
		else
		{
			pData[n] = 0xff;        // R
			pData[n + 1] = 0xff;    // G
			pData[n + 2] = 0xff;    // B
			pData[n + 3] = 0xff;    // A
		}
	}

	return data;
}