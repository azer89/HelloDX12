#ifndef HELLO_DX12_PIPELINE_SIMPLE
#define HELLO_DX12_PIPELINE_SIMPLE

#include "DX12Context.h"
#include "DX12Shader.h"
#include "DX12Buffer.h"
#include "PipelineBase.h"
#include "Scene.h"
#include "Camera.h"
#include "Configs.h"

#include "glm/glm.hpp"

#include <array>

class PipelineSimple final : PipelineBase
{
public:
	PipelineSimple(DX12Context& ctx, Scene* scene, Camera* camera);
	~PipelineSimple() = default;

	void Update(DX12Context& ctx) override;
	void PopulateCommandList(DX12Context& ctx) override;

	void Destroy() override;

private:
	void CreateSRV(DX12Context& ctx);
	void CreateRTV(DX12Context& ctx);
	void CreateDSV(DX12Context& ctx);
	void CreateConstantBuffer(DX12Context& ctx);
	void CreateRootSignature(DX12Context& ctx);
	void CreateShaders(DX12Context& ctx);
	void CreateGraphicsPipeline(DX12Context& ctx);

public:
	ComPtr<ID3D12Resource> depthStencil_;

	ComPtr<ID3D12DescriptorHeap> rtvHeap_; // Render target view
	ComPtr<ID3D12DescriptorHeap> srvHeap_; // Shader resource view
	ComPtr<ID3D12DescriptorHeap> dsvHeap_; // Depth stencil view
	
	uint32_t rtvDescriptorSize_;
	ComPtr<ID3D12Resource> renderTargets_[AppConfig::FrameCount];

	Scene* scene_;
	Camera* camera_;

private:
	std::array<DX12Buffer, AppConfig::FrameCount> constantBuffers_;
};

#endif