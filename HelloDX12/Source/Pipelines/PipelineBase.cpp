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
	indirectCommand_.Destroy();
	if (pipelineState_) { pipelineState_->Release(); }
	if (commandSignature_) { commandSignature_->Release(); }
}

void PipelineBase::CreateCommandSignature(DX12Context& ctx)
{
	D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
	argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
	argumentDescs[0].Constant.RootParameterIndex = 0;
	argumentDescs[0].Constant.Num32BitValuesToSet = 1;
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
	commandSignature_->SetName(L"Command_Signature");
}

void PipelineBase::CreateIndirectCommandFromArray(DX12Context& ctx, const std::span<IndirectCommand> commandArray)
{
	const uint32_t elementCount = static_cast<uint32_t>(commandArray.size());
	const uint32_t stride = sizeof(IndirectCommand);
	const uint64_t bufferSize = elementCount * stride;
	indirectCommand_.CreateDeviceOnlyBuffer(
		ctx,
		commandArray.data(),
		elementCount,
		bufferSize,
		stride);
}