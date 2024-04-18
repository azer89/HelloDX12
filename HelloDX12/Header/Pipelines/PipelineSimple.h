#ifndef HELLO_DX12_PIPELINE_SIMPLE
#define HELLO_DX12_PIPELINE_SIMPLE

#include "DX12Context.h"
#include "DX12Shader.h"
#include "DX12ConstantBuffer.h"
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

	void Update(DX12Context& ctx);
	void PopulateCommandList(DX12Context& ctx);

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
	struct CBMVP
	{
		glm::mat4 worldMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
	};

	std::array<DX12ConstantBuffer, AppConfig::FrameCount> constantBuffers_;
};

#endif