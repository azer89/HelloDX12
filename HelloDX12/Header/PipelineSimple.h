#ifndef HELLO_DX12_PIPELINE_SIMPLE
#define HELLO_DX12_PIPELINE_SIMPLE

#include "DX12Context.h"
#include "DX12Shader.h"
#include "Scene.h"
#include "Camera.h"
#include "Configs.h"

#include "glm/glm.hpp"

class PipelineSimple
{
public:
	PipelineSimple(DX12Context& ctx, Scene* scene, Camera* camera);
	~PipelineSimple() = default;

	void PopulateCommandList(DX12Context& ctx);

private:
	void CreateSRV(DX12Context& ctx);
	void CreateRTV(DX12Context& ctx);
	void CreateConstantBuffer(DX12Context& ctx);
	void CreateRootSignature(DX12Context& ctx);
	void CreateShaders(DX12Context& ctx);
	void CreateGraphicsPipeline(DX12Context& ctx);

public:
	ComPtr<ID3D12PipelineState> pipelineState_;
	ComPtr<ID3D12RootSignature> rootSignature_;

	DX12Shader vertexShader_;
	DX12Shader fragmentShader_;

	ComPtr<ID3D12DescriptorHeap> rtvHeap_; // Render target views
	ComPtr<ID3D12DescriptorHeap> srvHeap_; // Shader resource views
	
	uint32_t rtvDescriptorSize_;
	ComPtr<ID3D12Resource> renderTargets_[AppConfig::FrameCount];

	Scene* scene_;
	Camera* camera_;

	CD3DX12_VIEWPORT viewport_;
	CD3DX12_RECT scissor_;

private:
	// Constant buffer
	struct ConstantBuffer
	{
		glm::mat4 worldMatrix;        // 64 bytes
		glm::mat4 viewMatrix;         // 64 bytes
		glm::mat4 projectionMatrix;   // 64 bytes
	};

    // Create a union with the correct size and enough room for one ConstantBuffer
	union PaddedConstantBuffer
	{
		ConstantBuffer constants;
		uint8_t bytes[2 * D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT];
	};

	// Check the exact size of the PaddedConstantBuffer to make sure it will align properly
	static_assert(sizeof(PaddedConstantBuffer) == 2 * D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, "PaddedConstantBuffer is not aligned properly");

	PaddedConstantBuffer* mappedConstantData_;
	ComPtr<ID3D12Resource> perFrameConstants_;
	D3D12_GPU_VIRTUAL_ADDRESS constantDataGpuAddr_;
};

#endif