#include "ResourcesIBL.h"
#include "PipelineEquirect2Cube.h"

constexpr uint32_t CUBE_SIZE = 1024;

ResourcesIBL::ResourcesIBL(DX12Context& ctx, const std::string& hdrFile)
{
	hdrImage_.LoadHDR(ctx, hdrFile);
	hdrImage_.buffer_.SetName("HDR_Image");

	CreateEnviromentCubemap(ctx);

	PipelineEquirect2Cube pipE2C(ctx);
	pipE2C.GenerateCubemapFromHDR(ctx, &hdrImage_, &environmentCubemap_, envCubemapUAVDesc_);
}

ResourcesIBL::~ResourcesIBL()
{
	Destroy();
}

void ResourcesIBL::Destroy()
{
	hdrImage_.Destroy();
	environmentCubemap_.Destroy();
}

void ResourcesIBL::CreateEnviromentCubemap(DX12Context& ctx)
{
	environmentCubemap_.CreateCubemap(ctx, CUBE_SIZE, CUBE_SIZE);
	environmentCubemap_.buffer_.SetName("Environment_Cubemap");

	envCubemapUAVDesc_ =
	{
		envCubemapUAVDesc_.Format = environmentCubemap_.format_
	};
	envCubemapUAVDesc_.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
	envCubemapUAVDesc_.Texture2DArray.MipSlice = 0;
	envCubemapUAVDesc_.Texture2DArray.FirstArraySlice = 0;
	envCubemapUAVDesc_.Texture2DArray.ArraySize = environmentCubemap_.layerCount_;
}