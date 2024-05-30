#ifndef HELLO_DX12_PIPELINE_DIFFUSE_MAP
#define HELLO_DX12_PIPELINE_DIFFUSE_MAP

#include "DX12Context.h"
#include "PipelineBase.h"

class PipelineDiffuseMap final : public PipelineBase
{
public:
	PipelineDiffuseMap(
		DX12Context& ctx);
	~PipelineDiffuseMap() = default;

	void Update(DX12Context& ctx, UIData& uiData) override {}
	void PopulateCommandList(DX12Context& ctx) override {};

private:
};

#endif