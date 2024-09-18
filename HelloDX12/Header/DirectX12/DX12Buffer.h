#ifndef HELLO_DX12_BUFFER
#define HELLO_DX12_BUFFER

#include "DX12Context.h"

class DX12Buffer
{
public:
	void CreateHostVisibleBuffer(DX12Context& ctx, uint32_t elementCount, uint64_t bufferSize, uint32_t stride);
	void CreateDeviceOnlyBuffer(DX12Context& ctx, void* data, uint32_t elementCount, uint64_t bufferSize, uint32_t stride);
	void CreateConstantBuffer(DX12Context& ctx, uint64_t bufferSize);
	void CreateVertexBuffer(DX12Context& ctx, void* data, uint64_t bufferSize, uint32_t stride);
	void CreateIndexBuffer(DX12Context& ctx, void* data, uint64_t bufferSize, DXGI_FORMAT format);

	void CreateImage(
		DX12Context& ctx,
		uint32_t width,
		uint32_t height,
		uint32_t mipmapCount,
		uint32_t layerCount,
		DXGI_FORMAT imageFormat,
		D3D12_RESOURCE_FLAGS flags);
	void CreateImageFromData(
		DX12Context& ctx,
		const void* imageData,
		uint32_t width,
		uint32_t height,
		uint32_t mipmapCount,
		uint32_t bytesPerPixel,
		DXGI_FORMAT imageFormat,
		D3D12_RESOURCE_FLAGS flags);
	void CreateColorAttachment(
		DX12Context& ctx,
		uint32_t width,
		uint32_t height,
		uint32_t mipmapCount,
		uint32_t msaaCount,
		DXGI_FORMAT imageFormat,
		D3D12_RESOURCE_FLAGS flags);
	void CreateDepthAttachment(
		DX12Context& ctx,
		uint32_t width,
		uint32_t height,
		uint32_t msaaCount,
		DXGI_FORMAT imageFormat);
	void SetAsSwapchainBuffer(
		DX12Context& ctx, 
		CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle, 
		uint32_t frameIndex);
	
	// Alternative to UploadData()
	template<typename T>
	T* As()
	{
		return reinterpret_cast<T*>(mappedData_);
	}

	void UploadData(const void* data) const;

	void Destroy();

	D3D12_CONSTANT_BUFFER_VIEW_DESC GetCBVDescription() const
	{
		return
		{
			.BufferLocation = gpuAddress_,
			.SizeInBytes = static_cast<UINT>(constantBufferSize_)
		};
	}

	void UAVBarrier(ID3D12GraphicsCommandList* commandList) const;
	void TransitionCommand(
		ID3D12GraphicsCommandList* commandList,
		D3D12_RESOURCE_STATES afterState);
	void TransitionCommand(
		ID3D12GraphicsCommandList* commandList,
		D3D12_RESOURCE_STATES beforeState,
		D3D12_RESOURCE_STATES afterState);

	void SetName(const std::string& objectName) const;

	// Getters
	[[nodiscard]] D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDescription(uint32_t mipLevel) const;
	[[nodiscard]] D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDescription() const { return srvDescription_;  }
	[[nodiscard]] ID3D12Resource* GetResource() const { return resource_; }

private:
	void CreateUploadHeap(DX12Context& ctx,
		uint64_t bufferSize,
		uint32_t mipLevel,
		ID3D12Resource** bufferUploadHeap,
		D3D12MA::Allocation** bufferUploadHeapAllocation);

	D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDescriptionFromBuffer(uint32_t elementCount, uint32_t stride) const;
	D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDescriptionFromImage(DXGI_FORMAT format, uint32_t layerCount, uint32_t mipmapCount) const;

	static uint32_t GetConstantBufferByteSize(uint64_t byteSize);

private:
	uint64_t bufferSize_{}; // TODO Set as width_
	ID3D12Resource* resource_{};
	D3D12MA::Allocation* dmaAllocation_{};
	
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescription_{};
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	
	uint64_t constantBufferSize_{};
	uint8_t* mappedData_{};
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress_{};

	D3D12_RESOURCE_STATES state_{ D3D12_RESOURCE_STATE_COMMON }; // TODO currently does not track mipmap levels
	bool isSwapchainBuffer_{ false };
};

#endif