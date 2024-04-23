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
	buffer_.CreateBuffer(ctx, bufferSize);  // TODO Create device only buffer
	buffer_.UploadData(lights_.data());
}

D3D12_SHADER_RESOURCE_VIEW_DESC ResourcesLights::GetSRVDescription() const
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
	{
		.Format = DXGI_FORMAT_UNKNOWN,
		.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	};
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = static_cast<UINT>(lights_.size());
	srvDesc.Buffer.StructureByteStride = sizeof(LightData);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	return srvDesc;
}