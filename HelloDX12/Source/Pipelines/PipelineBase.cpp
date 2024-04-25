#include "PipelineBase.h"

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
	descriptor_.Destroy();
	if (descriptorHeap_) { descriptorHeap_->Release(); }
	if (pipelineState_) { pipelineState_->Release(); }
}