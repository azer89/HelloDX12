#ifndef HELLO_DX12_PIPELINE_BASE
#define HELLO_DX12_PIPELINE_BASE

#include "DX12Context.h"
#include "DX12Shader.h"
#include "DX12RootSignature.h"
#include "DX12DescriptorHeap.h"

class PipelineBase
{
public:
	explicit PipelineBase(DX12Context& ctx);
	virtual ~PipelineBase();

	virtual void Destroy();
	virtual void Update(DX12Context& ctx) = 0;
	virtual void PopulateCommandList(DX12Context& ctx) = 0;

protected:
	ID3D12PipelineState* pipelineState_ = nullptr;
	
	DX12RootSignature rootSignature_ = {};
	//DX12DescriptorHeap descriptorHeap_ = {};

	DX12Shader vertexShader_ = {};
	DX12Shader fragmentShader_ = {};
	DX12Shader computeShader_ = {};

	CD3DX12_VIEWPORT viewport_ = {};
	CD3DX12_RECT scissor_ = {};
};

#endif