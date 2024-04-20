#include "ResourcesLights.h"
#include "DX12Exception.h"

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

	// Create view
	CreateSRV(ctx);
}

void ResourcesLights::CreateSRV(DX12Context& ctx)
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = 1,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	};
	ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap_)))

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = lights_.size();
	srvDesc.Buffer.StructureByteStride = sizeof(LightData);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	ctx.GetDevice()->CreateShaderResourceView(buffer_.resource_.Get(), &srvDesc, srvHeap_->GetCPUDescriptorHandleForHeapStart());
}