#ifndef HELLO_DX12_PIPELINE_SIMPLE
#define HELLO_DX12_PIPELINE_SIMPLE

#include "DX12Context.h"
#include "DX12Shader.h"
#include "Scene.h"
#include "Configs.h"

class PipelineSimple
{
public:
	PipelineSimple(DX12Context& ctx, Scene* scene);
	~PipelineSimple() = default;

	void PopulateCommandList(DX12Context& ctx);

private:
	void CreateSRV(DX12Context& ctx);
	void CreateRTV(DX12Context& ctx);
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
	
	UINT rtvDescriptorSize_;
	ComPtr<ID3D12Resource> renderTargets_[AppConfig::FrameCount];

	Scene* scene_;

	CD3DX12_VIEWPORT viewport_;
	CD3DX12_RECT scissor_;
};

#endif