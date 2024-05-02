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
	descriptorManager_.Destroy();
	if (pipelineState_) { pipelineState_->Release(); }
}