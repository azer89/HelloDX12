#include "ResourcesLights.h"
#include "DX12Exception.h"

ResourcesLights::~ResourcesLights()
{
	Destroy();
}

void ResourcesLights::Destroy()
{
	buffer_.Destroy();
}

void ResourcesLights::AddLights(DX12Context& ctx, const std::vector<LightData>& lights)
{
	lights_ = lights;
	const uint64_t bufferSize = lights.size() * sizeof(LightData);
	buffer_.CreateDeviceOnlyBuffer(ctx, lights_.data(), lights_.size(), bufferSize, sizeof(LightData));
}

D3D12_SHADER_RESOURCE_VIEW_DESC ResourcesLights::GetSRVDescription() const
{
	return buffer_.srvDesccription_;
}