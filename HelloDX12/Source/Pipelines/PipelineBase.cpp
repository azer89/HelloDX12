#include "PipelineBase.h"
#include "DX12Exception.h"

PipelineBase::PipelineBase(DX12Context& ctx)
{
	viewport_ = ctx.GetViewport();
	scissor_ = ctx.GetScissor();
}

PipelineBase::~PipelineBase()
{
	Destroy();
}

void PipelineBase::Destroy()
{
	vertexShader_.Destroy();
	fragmentShader_.Destroy();
	computeShader_.Destroy();
	rootSignature_.Destroy();
	if (pipelineState_) { pipelineState_->Release(); }
	if (commandSignature_) { commandSignature_->Release(); }
}

void PipelineBase::CreateCommandSignature(DX12Context& ctx)
{
	// Each command consists of a CBV update and a DrawInstanced call.
	D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
	argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
	argumentDescs[0].ConstantBufferView.RootParameterIndex = 0;
	argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;

	D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = 
	{
		.ByteStride = sizeof(IndirectCommand),
		.NumArgumentDescs = _countof(argumentDescs),
		.pArgumentDescs = argumentDescs,
	};
	
	ThrowIfFailed(ctx.GetDevice()->CreateCommandSignature(
		&commandSignatureDesc,
		rootSignature_.rootSignature_,
		IID_PPV_ARGS(&commandSignature_)));
}