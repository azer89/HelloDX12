#ifndef HELLO_DX12_PIPELINE_SPECULAR_MAP
#define HELLO_DX12_PIPELINE_SPECULAR_MAP

#include "DX12Context.h"
#include "DX12Image.h"
#include "PipelineBase.h"

class PipelineSpecularMap final : public PipelineBase
{
public:
	PipelineSpecularMap(
		DX12Context& ctx);
	~PipelineSpecularMap();

	void Destroy() override;

	void Update(DX12Context& ctx, UIData& uiData) override {}
	void PopulateCommandList(DX12Context& ctx) override {};

	void Execute(DX12Context& ctx,
		DX12Image* environmentMap,
		DX12Image* specularMap);

private:
	void CreateDescriptors(DX12Context& ctx,
		DX12Image* environmentMap,
		DX12Image* specularMap);

	void GenerateShader(DX12Context& ctx);

	void CreatePipeline(DX12Context& ctx);

private:
	DX12DescriptorHeap descriptorHeap_{};
};

#endif