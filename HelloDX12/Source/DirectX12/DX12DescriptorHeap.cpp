#include "DX12DescriptorHeap.h"

#include <iostream>

void DX12DescriptorHeap::Destroy()
{
	if (handle_)
	{
		handle_->Release();
		handle_ = nullptr;
	}
}

// TODO Not used
void DX12DescriptorHeap::Create(DX12Context& ctx, uint32_t descriptorCount)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = descriptorCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};
	ctx.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&handle_));
}

void DX12DescriptorHeap::Create(DX12Context& ctx)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = static_cast<UINT>(descriptors_.size() + descriptorArray_.DescriptorCount()),
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};
	ctx.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&handle_));

	SetupHandles(ctx);
}

void DX12DescriptorHeap::SetupHandles(DX12Context& ctx)
{
	uint32_t incrementSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (uint32_t i = 0; i < descriptors_.size(); ++i)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(handle_->GetCPUDescriptorHandleForHeapStart(), i, incrementSize);
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(handle_->GetGPUDescriptorHandleForHeapStart(), i, incrementSize);

		if (descriptors_[i].type_ == D3D12_DESCRIPTOR_RANGE_TYPE_CBV)
		{
			ctx.GetDevice()->CreateConstantBufferView(&(descriptors_[i].cbvDescription_), cpuHandle);
		}
		else if (descriptors_[i].type_ == D3D12_DESCRIPTOR_RANGE_TYPE_SRV)
		{
			ctx.GetDevice()->CreateShaderResourceView(descriptors_[i].buffer_->GetResource(), &(descriptors_[i].srvDescription_), cpuHandle);
		}
		else if (descriptors_[i].type_ == D3D12_DESCRIPTOR_RANGE_TYPE_UAV)
		{
			ctx.GetDevice()->CreateUnorderedAccessView(descriptors_[i].buffer_->GetResource(), nullptr, &(descriptors_[i].uavDescription_), cpuHandle);
		}

		descriptors_[i].cpuHandle_ = cpuHandle;
		descriptors_[i].gpuHandle_ = gpuHandle;
	}

	// This part is for unbounded array
	uint32_t descriptorArraySize = descriptorArray_.DescriptorCount();
	uint32_t prevDescriptroCount = static_cast<uint32_t>(descriptors_.size());
	descriptorArray_.cpuHandles_.resize(descriptorArraySize);
	descriptorArray_.gpuHandles_.resize(descriptorArraySize);
	for (uint32_t i = 0; i < descriptorArraySize; ++i)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(handle_->GetCPUDescriptorHandleForHeapStart(), i + prevDescriptroCount, incrementSize);
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(handle_->GetGPUDescriptorHandleForHeapStart(), i + prevDescriptroCount, incrementSize);

		ctx.GetDevice()->CreateShaderResourceView(
			descriptorArray_.buffers_[i]->GetResource(),
			&(descriptorArray_.srvDescriptions_[i]),
			cpuHandle);

		descriptorArray_.cpuHandles_[i] = cpuHandle;
		descriptorArray_.gpuHandles_[i] = gpuHandle;
	}
}

void DX12DescriptorHeap::BindHeap(ID3D12GraphicsCommandList* commandList) const
{
	ID3D12DescriptorHeap* ppHeaps[] = { handle_ };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}

void DX12DescriptorHeap::BindDescriptorsGraphics(ID3D12GraphicsCommandList* commandList, uint32_t startRootParamIndex) const
{
	uint32_t rootParamIndex = startRootParamIndex;
	for (uint32_t i = 0; i < descriptors_.size(); ++i)
	{
		commandList->SetGraphicsRootDescriptorTable(rootParamIndex++, descriptors_[i].gpuHandle_);
	}

	if (descriptorArray_.HasBuffers())
	{
		commandList->SetGraphicsRootDescriptorTable(rootParamIndex++, descriptorArray_.GetFirstGPUHandle());
	}
}

void DX12DescriptorHeap::BindDescriptorsCompute(ID3D12GraphicsCommandList* commandList, uint32_t startRootParamIndex) const
{
	uint32_t rootParamIndex = startRootParamIndex;
	for (uint32_t i = 0; i < descriptors_.size(); ++i)
	{
		commandList->SetComputeRootDescriptorTable(rootParamIndex++, descriptors_[i].gpuHandle_);
	}

	if (descriptorArray_.HasBuffers())
	{
		commandList->SetComputeRootDescriptorTable(rootParamIndex++, descriptorArray_.GetFirstGPUHandle());
	}
}

void DX12DescriptorHeap::BindSingleDescriptorCompute(
	ID3D12GraphicsCommandList* commandList,
	uint32_t rootParamIndex,
	uint32_t descriptorIndex) const
{
	if (descriptorIndex < 0 || descriptorIndex >= descriptors_.size())
	{
		std::cerr << "descriptorIndex " << descriptorIndex << " is invalid\n";
	}

	commandList->SetComputeRootDescriptorTable(rootParamIndex, descriptors_[descriptorIndex].gpuHandle_);
}