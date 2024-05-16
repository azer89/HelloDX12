#ifndef HELLO_DX12_PIPELINE_BASE
#define HELLO_DX12_PIPELINE_BASE

#include "DX12Context.h"
#include "DX12Shader.h"
#include "DX12RootSignature.h"
#include "DX12DescriptorHeap.h"
#include "IndirectCommand.h"

#include <span>

class PipelineBase
{
public:
	explicit PipelineBase(DX12Context& ctx);
	virtual ~PipelineBase();

	virtual void Destroy();
	virtual void Update(DX12Context& ctx) = 0;
	virtual void PopulateCommandList(DX12Context& ctx) = 0;

protected:
	void CreateCommandSignature(DX12Context& ctx);
	void CreateIndirectCommandFromArray(DX12Context& ctx, const std::span<IndirectCommand> commandArray);

protected:
	ID3D12PipelineState* pipelineState_ = nullptr;
	
	DX12RootSignature rootSignature_ = {};

	ID3D12CommandSignature* commandSignature_ = nullptr;
	DX12Buffer indirectCommand_;

	DX12Shader vertexShader_ = {};
	DX12Shader fragmentShader_ = {};
	DX12Shader computeShader_ = {};

	CD3DX12_VIEWPORT viewport_ = {};
	CD3DX12_RECT scissor_ = {};
};

#endif