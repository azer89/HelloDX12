#ifndef HELLO_DX12_DESCRIPTOR
#define HELLO_DX12_DESCRIPTOR

#include "DX12Buffer.h"

#include <vector>

struct DX12Descriptor
{
public:
	D3D12_DESCRIPTOR_RANGE_TYPE type_;
	D3D12_DESCRIPTOR_RANGE_FLAGS rangeFlags_;
	D3D12_SHADER_VISIBILITY shaderVisibility_;
	DX12Buffer* buffer_;
	union
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDescription_;
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDescription_;
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDescription_;
	};
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle_;
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle_;
};

// Descriptors for an array of textures, always an SRV
struct DX12DescriptorArray
{
public:
	D3D12_DESCRIPTOR_RANGE_FLAGS rangeFlags_;
	D3D12_SHADER_VISIBILITY shaderVisibility_;
	std::vector<DX12Buffer*> buffers_ = {};
	std::vector<D3D12_SHADER_RESOURCE_VIEW_DESC> srvDescriptions_ = {};
	std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> cpuHandles_ = {};
	std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> gpuHandles_ = {};

	bool HasBuffers() const
	{
		return !buffers_.empty();
	}

	uint32_t DescriptorCount() const
	{
		return static_cast<uint32_t>(buffers_.size());
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetFirstGPUHandle() const
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle = gpuHandles_.empty() ? CD3DX12_GPU_DESCRIPTOR_HANDLE() : gpuHandles_[0];
		return gpuHandle;
	}
};

#endif