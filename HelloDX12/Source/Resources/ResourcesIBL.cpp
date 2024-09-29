#include "ResourcesIBL.h"
#include "PipelineMipmap.h"
#include "PipelineEquirect2Cube.h"
#include "PipelineSpecularMap.h"
#include "PipelineDiffuseMap.h"
#include "PipelineBRDFLUT.h"
#include "Utility.h"

constexpr uint32_t ENVIRONMENT_CUBE_SIZE = 1024;
constexpr uint32_t SPECULAR_CUBE_SIZE = 256;
constexpr uint32_t DIFFUSE_CUBE_SIZE = 32;
constexpr uint32_t LUT_SIZE = 256;

ResourcesIBL::ResourcesIBL(DX12Context& ctx, const std::string& hdrFile)
{
	hdrImage_.LoadHDR(ctx, hdrFile);
	hdrImage_.buffer_.SetName("HDR_Image");

	// Environment cubemap
	const uint32_t envMipmapCount = Utility::MipMapCount(ENVIRONMENT_CUBE_SIZE);
	environmentCubemap_.CreateCubemap(ctx, ENVIRONMENT_CUBE_SIZE, ENVIRONMENT_CUBE_SIZE, envMipmapCount);
	environmentCubemap_.buffer_.SetName("Environment_Cubemap");
	PipelineEquirect2Cube pipE2C(ctx);
	pipE2C.GenerateCubemapFromHDR(ctx, &hdrImage_, &environmentCubemap_);
	constexpr bool isTextureArray = true;
	const PipelineMipmap pipMipmap(ctx, isTextureArray);
	pipMipmap.GenerateMipmap(ctx, &environmentCubemap_);

	// Specular (prefilter) map
	const uint32_t specularMipmapCount = Utility::MipMapCount(SPECULAR_CUBE_SIZE);
	specularCubemap_.CreateCubemap(ctx, SPECULAR_CUBE_SIZE, SPECULAR_CUBE_SIZE, specularMipmapCount);
	specularCubemap_.buffer_.SetName("Specular_Cubemap");
	PipelineSpecularMap pipSpecular(ctx);
	pipSpecular.Execute(ctx, &environmentCubemap_, &specularCubemap_);

	// Diffuse (irradiance) map
	constexpr uint32_t diffuseMipmapCount = 1;
	diffuseCubemap_.CreateCubemap(ctx, DIFFUSE_CUBE_SIZE, DIFFUSE_CUBE_SIZE, diffuseMipmapCount);
	diffuseCubemap_.buffer_.SetName("Diffuse_Cubemap");
	PipelineDiffuseMap pipDiffuse(ctx);
	pipDiffuse.Execute(ctx, &environmentCubemap_, &diffuseCubemap_);

	// LUT
	brdfLutImage_.Create(ctx, LUT_SIZE, LUT_SIZE, 1, 1, DXGI_FORMAT_R16G16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	brdfLutImage_.buffer_.SetName("BRDF_LUT");
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
	diffuseCubemap_.Destroy();
	specularCubemap_.Destroy();
	brdfLutImage_.Destroy();
}