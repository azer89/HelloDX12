#include "DX12Image.h"
#include "DX12Exception.h"
#include "Utility.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <sstream>

DX12Image::DX12Image()
{
}

void DX12Image::Destroy()
{
	buffer_.Destroy();
}

void DX12Image::Load(DX12Context& ctx, std::string filename)
{
	stbi_set_flip_vertically_on_load(false);

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!pixels)
	{
		std::stringstream ss;
		ss << "Failed to load image " << filename;
		throw std::runtime_error(ss.str());
	}

	width_ = texWidth;
	height_ = texHeight;
	pixelSize_ = 4; // texChannels is 3 eventhough STBI_rgb_alpha is used
	format_ = ctx.GetSwapchainFormat();
	mipmapCount_ = Utility::MipMapCount(width_, height_);
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	buffer_.CreateImage(
		ctx, 
		pixels, 
		width_, 
		height_, 
		mipmapCount_, 
		pixelSize_, 
		format_,
		flags
	);
}

void DX12Image::CreateColorAttachment(DX12Context& ctx)
{

}

D3D12_SHADER_RESOURCE_VIEW_DESC DX12Image::GetSRVDescription()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
	{
		.Format = format_, // Image format
		.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	};
	srvDesc.Texture2D.MipLevels = mipmapCount_;
	
	return srvDesc;
}

D3D12_STATIC_SAMPLER_DESC DX12Image::GetSampler()
{
	return
	{
		.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT,
		.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		.MipLODBias = 0,
		.MaxAnisotropy = 0,
		.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
		.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
		.MinLOD = 0.0f,
		.MaxLOD = D3D12_FLOAT32_MAX,
		.ShaderRegister = 0,
		.RegisterSpace = 0,
		.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
	};
}

// Generate a simple black and white checkerboard texture.
std::vector<uint8_t> DX12Image::GenerateCheckerboard(DX12Context& ctx)
{
	const uint32_t rowPitch = width_ * pixelSize_;
	const uint32_t cellPitch = rowPitch >> 6;        // The width of a cell in the checkboard texture.
	const uint32_t cellHeight = width_ >> 6;    // The height of a cell in the checkerboard texture.
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