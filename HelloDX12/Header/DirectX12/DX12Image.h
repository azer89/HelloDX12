#ifndef HELLO_DX12_IMAGE
#define HELLO_DX12_IMAGE

#include "DX12Context.h"
#include "DX12Buffer.h"

class DX12Image
{
public:
	DX12Image();
	~DX12Image() = default;
	void Destroy();
	void Load(DX12Context& ctx, const std::string& filename);
	void Load(DX12Context& ctx, void* data, uint32_t width, uint32_t height);
	void LoadHDR(DX12Context& ctx, const std::string& filename);
	void CreateCubemap(DX12Context& ctx, uint32_t width, uint32_t height, uint32_t mipmapCount);
	void Create(DX12Context& ctx, 
		uint32_t width, 
		uint32_t height, 
		uint32_t mipmapCount, 
		uint32_t layerCount, 
		DXGI_FORMAT format,
		D3D12_RESOURCE_FLAGS flags);
	void CreateColorAttachment(DX12Context& ctx, uint32_t msaaCount);
	void CreateDepthAttachment(DX12Context& ctx, uint32_t msaaCount);
	ID3D12Resource* GetResource() const { return buffer_.GetResource(); }
	
	void UAVBarrier(ID3D12GraphicsCommandList* commandList) const;
	void TransitionCommand(
		ID3D12GraphicsCommandList* commandList,
		D3D12_RESOURCE_STATES afterState);

	static D3D12_STATIC_SAMPLER_DESC GetDefaultSampler();
	static std::vector<uint8_t> GenerateCheckerboard(
		uint32_t width,
		uint32_t height,
		uint32_t pixelSize);

	void SetName(const std::string& objectName) const;

public:
	uint32_t width_{};
	uint32_t height_{};
	uint32_t pixelSize_{};
	uint32_t mipmapCount_{};
	uint32_t layerCount_{};
	DXGI_FORMAT format_{ DXGI_FORMAT_UNKNOWN };

	DX12Buffer buffer_{};
};

#endif