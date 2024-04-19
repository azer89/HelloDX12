#include "ResourcesLights.h"

void ResourcesLights::Destroy()
{
	buffer_.Destroy();
}

void ResourcesLights::AddLights(DX12Context& ctx, const std::vector<LightData>& lights)
{
	lights_ = lights;
	const uint64_t bufferSize = lights.size() * sizeof(LightData);
	buffer_.CreateBuffer(ctx, bufferSize);  // TODO Create device only buffer
	buffer_.UploadData(lights_.data());
}