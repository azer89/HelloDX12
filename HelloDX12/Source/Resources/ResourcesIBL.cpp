#include "ResourcesIBL.h"
#include "PipelineEquirect2Cube.h"

ResourcesIBL::ResourcesIBL(DX12Context& ctx, const std::string& hdrFile)
{
	hdrImage_.LoadHDR(ctx, hdrFile);

	PipelineEquirect2Cube pipE2C(ctx, );
}

ResourcesIBL::~ResourcesIBL()
{
	Destroy();
}

void ResourcesIBL::Destroy()
{
	hdrImage_.Destroy();
}