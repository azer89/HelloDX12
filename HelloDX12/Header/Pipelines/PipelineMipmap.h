#ifndef HELLO_DX12_PIPELINE_MIPMAP
#define HELLO_DX12_PIPELINE_MIPMAP

#include "DX12Context.h"
#include "DX12Image.h"
#include "PipelineBase.h"

class PipelineMipmap final : public PipelineBase
{
public:
	PipelineMipmap(
		DX12Context& ctx);
	~PipelineMipmap() = default;

	void Update(DX12Context& ctx) override {}
	void PopulateCommandList(DX12Context& ctx) override {};
};

#endif