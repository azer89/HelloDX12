#include "ResourcesIBL.h"
#include "PipelineEquirect2Cube.h"
#include "PipelineSpecularMap.h"
#include "PipelineBRDFLUT.h"
#include "Utility.h"

constexpr uint32_t CUBE_SIZE = 1024;
constexpr uint32_t LUT_SIZE = 256;

ResourcesIBL::ResourcesIBL(DX12Context& ctx, const std::string& hdrFile)
{
	hdrImage_.LoadHDR(ctx, hdrFile);
	hdrImage_.buffer_.SetName("HDR_Image");

	constexpr uint32_t envMipmapCount = 1;
	environmentCubemap_.CreateCubemap(ctx, CUBE_SIZE, CUBE_SIZE, envMipmapCount);
	environmentCubemap_.buffer_.SetName("Environment_Cubemap");

	constexpr uint32_t diffuseMipmapCount = 1;
	diffuseCubemap_.CreateCubemap(ctx, CUBE_SIZE, CUBE_SIZE, diffuseMipmapCount);
	diffuseCubemap_.buffer_.SetName("Diffuse_Cubemap");

	const uint32_t specularMipmapCount = Utility::MipMapCount(CUBE_SIZE);
	specularCubemap_.CreateCubemap(ctx, CUBE_SIZE, CUBE_SIZE, specularMipmapCount);
	specularCubemap_.buffer_.SetName("Specular_Cubemap");

	brdfLutImage_.Create(ctx, LUT_SIZE, LUT_SIZE, 1, 1, DXGI_FORMAT_R16G16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	brdfLutImage_.buffer_.SetName("BRDF_LUT");

	PipelineEquirect2Cube pipE2C(ctx);
	pipE2C.GenerateCubemapFromHDR(ctx, &hdrImage_, &environmentCubemap_);

	PipelineBRDFLUT pipLUT(ctx);
	pipLUT.Execute(ctx, &brdfLutImage_);

	PipelineSpecularMap pipSpecular(ctx);
	pipSpecular.Execute(ctx, &environmentCubemap_, &specularCubemap_);
}

ResourcesIBL::~ResourcesIBL()
{
	Destroy();
}

void ResourcesIBL::Destroy()
{
	hdrImage_.Destroy();
	environmentCubemap_.Destroy();
	diffuseCubemap_.Destroy();
	specularCubemap_.Destroy();
	brdfLutImage_.Destroy();
}