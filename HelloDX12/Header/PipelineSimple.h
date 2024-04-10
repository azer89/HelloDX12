#ifndef HELLO_DX12_PIPELINE_SIMPLE
#define HELLO_DX12_PIPELINE_SIMPLE

#include "DX12Context.h"
#include "Scene.h"

class PipelineSimple
{
public:
	PipelineSimple(DX12Context& ctx, Scene* scene);
	~PipelineSimple() = default;

	void PopulateCommandList(DX12Context& ctx);

public:
	ComPtr<ID3D12PipelineState> pipelineState_;
	ComPtr<ID3D12RootSignature> rootSignature_;

	ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	ComPtr<ID3D12DescriptorHeap> srvHeap_;
	
	UINT rtvDescriptorSize_;
	ComPtr<ID3D12Resource> renderTargets_[DX12Context::FrameCount];

	Scene* scene_;

	CD3DX12_VIEWPORT viewport_;
	CD3DX12_RECT scissor_;
};

#endif