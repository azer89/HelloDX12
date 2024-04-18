#ifndef HELLO_DX12_PIPELINE_BASE
#define HELLO_DX12_PIPELINE_BASE

#include "DX12Context.h"
#include "DX12Shader.h"

class PipelineBase
{
public:
	explicit PipelineBase();
	virtual ~PipelineBase();

	virtual void Update(DX12Context& ctx) = 0;
	virtual void PopulateCommandList(DX12Context& ctx) = 0;

protected:
	ComPtr<ID3D12PipelineState> pipelineState_;
	ComPtr<ID3D12RootSignature> rootSignature_;

	DX12Shader vertexShader_;
	DX12Shader fragmentShader_;

	CD3DX12_VIEWPORT viewport_;
	CD3DX12_RECT scissor_;
};

#endif