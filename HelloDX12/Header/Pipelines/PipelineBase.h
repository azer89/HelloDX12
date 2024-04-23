#ifndef HELLO_DX12_PIPELINE_BASE
#define HELLO_DX12_PIPELINE_BASE

#include "DX12Context.h"
#include "DX12Shader.h"

class PipelineBase
{
public:
	explicit PipelineBase(DX12Context& ctx);
	virtual ~PipelineBase()
	{
		Destroy();
	}

	virtual void Update(DX12Context& ctx) = 0;
	virtual void PopulateCommandList(DX12Context& ctx) = 0;

	virtual void Destroy()
	{
		vertexShader_.Destroy();
		fragmentShader_.Destroy();
		computeShader_.Destroy();
		if (srvHeap_) { srvHeap_->Release(); }
		if (rootSignature_) { rootSignature_->Release(); }
		if (pipelineState_) { pipelineState_->Release(); }
	}

protected:
	ID3D12PipelineState* pipelineState_ = nullptr;
	ID3D12RootSignature* rootSignature_ = nullptr;
	ID3D12DescriptorHeap* srvHeap_ = nullptr; // TODO

	DX12Shader vertexShader_ = {};
	DX12Shader fragmentShader_ = {};
	DX12Shader computeShader_ = {};

	CD3DX12_VIEWPORT viewport_ = {};
	CD3DX12_RECT scissor_ = {};
};

#endif