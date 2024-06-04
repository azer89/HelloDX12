#include "ResourcesIBL.h"
#include "PipelineEquirect2Cube.h"
#include "PipelineBRDFLUT.h"

constexpr uint32_t CUBE_SIZE = 1024;

ResourcesIBL::ResourcesIBL(DX12Context& ctx, const std::string& hdrFile)
{
	hdrImage_.LoadHDR(ctx, hdrFile);
	hdrImage_.buffer_.SetName("HDR_Image");

	constexpr uint32_t mipmapCount = 1;
	environmentCubemap_.CreateCubemap(ctx, CUBE_SIZE, CUBE_SIZE, mipmapCount);
	environmentCubemap_.buffer_.SetName("Environment_Cubemap");

	brdfLutImage_.Create(ctx, 256, 256, 1, 1, DXGI_FORMAT_R16G16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	brdfLutImage_.buffer_.SetName("BRDF_LUT");

	PipelineEquirect2Cube pipE2C(ctx);
	pipE2C.GenerateCubemapFromHDR(ctx, &hdrImage_, &environmentCubemap_);

	PipelineBRDFLUT pipLUT(ctx);
	pipLUT.Execute(ctx, &brdfLutImage_);
}

ResourcesIBL::~ResourcesIBL()
{
	Destroy();
}

void ResourcesIBL::Destroy()
{
	hdrImage_.Destroy();
	environmentCubemap_.Destroy();
	brdfLutImage_.Destroy();
}