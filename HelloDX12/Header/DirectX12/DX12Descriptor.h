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

#endif