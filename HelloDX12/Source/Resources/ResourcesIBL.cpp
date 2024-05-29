#include "ResourcesIBL.h"
#include "PipelineEquirect2Cube.h"

constexpr uint32_t CUBE_SIZE = 1024;

ResourcesIBL::ResourcesIBL(DX12Context& ctx, const std::string& hdrFile)
{
	hdrImage_.LoadHDR(ctx, hdrFile);
	hdrImage_.buffer_.SetName("HDR_Image");

	environmentCubemap_.CreateCubemap(ctx, CUBE_SIZE, CUBE_SIZE);
	environmentCubemap_.buffer_.SetName("Environment_Cubemap");

	PipelineEquirect2Cube pipE2C(ctx);
	pipE2C.GenerateCubemapFromHDR(ctx, &hdrImage_, &environmentCubemap_);
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