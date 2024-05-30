#ifndef HELLO_DX12_PIPELINE_SPECULAR_MAP
#define HELLO_DX12_PIPELINE_SPECULAR_MAP

#include "DX12Context.h"
#include "PipelineBase.h"

class PipelineSpecularMap final : public PipelineBase
{
public:
	PipelineSpecularMap(
		DX12Context& ctx);
	~PipelineSpecularMap() = default;

	void Update(DX12Context& ctx, UIData& uiData) override {}
	void PopulateCommandList(DX12Context& ctx) override {};

private:
};

#endif