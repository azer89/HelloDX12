#include "DX12Image.h"
#include "DX12Exception.h"
#include "Utility.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <sstream>

DX12Image::DX12Image()
{
}

void DX12Image::Destroy()
{
	buffer_.Destroy();
}

void DX12Image::Load(DX12Context& ctx, const std::string& filename)
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
	layerCount_ = 1;
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	buffer_.CreateImageFromData(
		ctx, 
		pixels, 
		width_, 
		height_, 
		mipmapCount_, 
		pixelSize_, 
		format_,
		flags
	);
	stbi_image_free(pixels);
}

void DX12Image::Load(DX12Context& ctx, void* data, uint32_t width, uint32_t height)
{
	width_ = width;
	height_ = height;
	pixelSize_ = 4;
	format_ = ctx.GetSwapchainFormat();
	mipmapCount_ = Utility::MipMapCount(width_, height_);
	layerCount_ = 1;
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	buffer_.CreateImageFromData(
		ctx,
		data,
		width_,
		height_,
		mipmapCount_,
		pixelSize_,
		format_,
		flags
	);
}

void DX12Image::LoadHDR(DX12Context& ctx, const std::string& filename)
{
	stbi_set_flip_vertically_on_load(false);

	int texWidth, texHeight, texChannels;
	float* pixels = stbi_loadf(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!pixels)
	{
		std::stringstream ss;
		ss << "Failed to load image " << filename;
		throw std::runtime_error(ss.str());
	}

	width_ = texWidth;
	height_ = texHeight;
	pixelSize_ = 4 * sizeof(float); // TODO Is this correct?
	format_ = DXGI_FORMAT_R32G32B32A32_FLOAT;
	mipmapCount_ = 1;
	layerCount_ = 1;

	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

	buffer_.CreateImageFromData(
		ctx,
		pixels,
		width_,
		height_,
		mipmapCount_,
		pixelSize_,
		format_,
		flags
	);
	stbi_image_free(pixels);
}

void DX12Image::CreateCubemap(DX12Context& ctx, uint32_t width, uint32_t height, uint32_t mipmapCount)
{
	width_ = width;
	height_ = height;
	pixelSize_ = 4 * sizeof(float);
	format_ = DXGI_FORMAT_R16G16B16A16_FLOAT;
	mipmapCount_ = mipmapCount;
	layerCount_ = 6;
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	buffer_.CreateImage(
		ctx,
		width_,
		height_,
		mipmapCount_,
		layerCount_,
		format_,
		flags);
}

void DX12Image::Create(DX12Context& ctx,
	uint32_t width,
	uint32_t height,
	uint32_t mipmapCount,
	uint32_t layerCount,
	DXGI_FORMAT format,
	D3D12_RESOURCE_FLAGS flags)
{
	width_ = width;
	height_ = height;
	pixelSize_ = 4 * sizeof(float);
	format_ = format;
	mipmapCount_ = mipmapCount;
	layerCount_ = layerCount;

	buffer_.CreateImage(
		ctx,
		width_,
		height_,
		mipmapCount_,
		layerCount_,
		format_,
		flags);
}

void DX12Image::CreateColorAttachment(DX12Context& ctx, uint32_t msaaCount)
{
	width_ = ctx.GetSwapchainWidth();
	height_ = ctx.GetSwapchainHeight();
	pixelSize_ = 4;
	format_ = ctx.GetSwapchainFormat();
	mipmapCount_ = 1;
	layerCount_ = 1;
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	if (msaaCount == 1)
	{
		flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	buffer_.CreateColorAttachment(
		ctx,
		width_,
		height_,
		mipmapCount_,
		msaaCount,
		format_,
		flags
	);
}

void DX12Image::CreateDepthAttachment(DX12Context& ctx, uint32_t msaaCount)
{
	width_ = ctx.GetSwapchainWidth();
	height_ = ctx.GetSwapchainHeight();
	pixelSize_ = sizeof(float);
	mipmapCount_ = 1;
	layerCount_ = 1;
	format_ = DXGI_FORMAT_D32_FLOAT;
	buffer_.CreateDepthAttachment(
		ctx, 
		width_, 
		height_, 
		msaaCount, 
		format_);
}

D3D12_STATIC_SAMPLER_DESC DX12Image::GetDefaultSampler()
{
	constexpr D3D12_TEXTURE_ADDRESS_MODE addressMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	constexpr D3D12_FILTER filter = D3D12_FILTER_ANISOTROPIC;

	return
	{
		.Filter = filter,
		.AddressU = addressMode,
		.AddressV = addressMode,
		.AddressW = addressMode,
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

void DX12Image::UAVBarrier(ID3D12GraphicsCommandList* commandList)
{
	buffer_.UAVBarrier(commandList);
}

void DX12Image::TransitionCommand(
	ID3D12GraphicsCommandList* commandList,
	D3D12_RESOURCE_STATES afterState)
{
	buffer_.TransitionCommand(commandList, afterState);
}

void DX12Image::SetName(const std::string& objectName) const
{
	buffer_.SetName(objectName);
}

// Generate a simple black and white checkerboard texture.
std::vector<uint8_t> DX12Image::GenerateCheckerboard(uint32_t width, uint32_t height, uint32_t pixelSize)
{
	const uint32_t rowPitch = width * pixelSize;
	const uint32_t cellPitch = rowPitch >> 6; // The width of a cell in the checkboard texture.
	const uint32_t cellHeight = width >> 6; // The height of a cell in the checkerboard texture.
	const uint32_t textureSize = rowPitch * height;

	std::vector<uint8_t> data(textureSize);
	uint8_t* pData = &data[0];

	for (uint32_t n = 0; n < textureSize; n += pixelSize)
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