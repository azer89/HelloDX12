#include "DX12DescriptorHeap.h"

void DX12DescriptorHeap::Destroy()
{
	if (descriptorHeap_)
	{
		descriptorHeap_->Release();
		descriptorHeap_ = nullptr;
	}
}

void DX12DescriptorHeap::Create(DX12Context& ctx, uint32_t descriptorCount)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = descriptorCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};
	ctx.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap_));
}

void DX12DescriptorHeap::Create(DX12Context& ctx)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = static_cast<UINT>(descriptors_.size()),
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};
	ctx.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap_));

	// Populate handles
	uint32_t incrementSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (uint32_t i = 0; i < descriptors_.size(); ++i)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(descriptorHeap_->GetCPUDescriptorHandleForHeapStart(), i, incrementSize);
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(descriptorHeap_->GetGPUDescriptorHandleForHeapStart(), i, incrementSize);

		if (descriptors_[i].type_ == D3D12_DESCRIPTOR_RANGE_TYPE_CBV)
		{
			ctx.GetDevice()->CreateConstantBufferView(&(descriptors_[i].cbvDescription_), cpuHandle);
		}
		else if (descriptors_[i].type_ == D3D12_DESCRIPTOR_RANGE_TYPE_SRV)
		{
			ctx.GetDevice()->CreateShaderResourceView(descriptors_[i].buffer_->resource_, &(descriptors_[i].srvDescription_), cpuHandle);
		}
		else if (descriptors_[i].type_ == D3D12_DESCRIPTOR_RANGE_TYPE_UAV)
		{
			ctx.GetDevice()->CreateUnorderedAccessView(descriptors_[i].buffer_->resource_, nullptr, &(descriptors_[i].uavDescription_), cpuHandle);
		}

		descriptors_[i].cpuHandle_ = cpuHandle;
		descriptors_[i].gpuHandle_ = gpuHandle;
	}
}

void DX12DescriptorHeap::BindHeap(ID3D12GraphicsCommandList* commandList)
{
	ID3D12DescriptorHeap* ppHeaps[] = { descriptorHeap_ };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}

void DX12DescriptorHeap::BindDescriptors(ID3D12GraphicsCommandList* commandList, uint32_t startRootParamIndex)
{
	uint32_t rootParamIndex = startRootParamIndex;
	for (uint32_t i = 0; i < descriptors_.size(); ++i)
	{
		commandList->SetGraphicsRootDescriptorTable(rootParamIndex++, descriptors_[i].gpuHandle_);
	}
}