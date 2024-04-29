#include "ResourcesIBL.h"
#include "PipelineEquirect2Cube.h"

ResourcesIBL::ResourcesIBL(DX12Context& ctx, const std::string& hdrFile)
{
	hdrImage_.LoadHDR(ctx, hdrFile);

	environmentCubemap_.CreateCubemap(ctx, 1024, 1024);
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