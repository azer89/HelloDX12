#ifndef HELLO_DX12_PIPELINE_DIFFUSE_MAP
#define HELLO_DX12_PIPELINE_DIFFUSE_MAP

#include "DX12Context.h"
#include "DX12Image.h"
#include "PipelineBase.h"

class PipelineDiffuseMap final : public PipelineBase
{
public:
	PipelineDiffuseMap(
		DX12Context& ctx);
	~PipelineDiffuseMap() = default;

	void Update(DX12Context& ctx, UIData& uiData) override {}
	void PopulateCommandList(DX12Context& ctx) override {};

	void Execute(DX12Context& ctx,
		DX12Image* environmentMap,
		DX12Image* diffuseMap);

private:
	void CreateDescriptors(DX12Context& ctx,
		DX12Image* environmentMap,
		DX12Image* diffuseMap);

	void GenerateShader(DX12Context& ctx);

	void CreatePipeline(DX12Context& ctx);
};

#endif