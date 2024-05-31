#ifndef HELLO_DX12_PIPELINE_BRDF_LUT
#define HELLO_DX12_PIPELINE_BRDF_LUT

#include "DX12Context.h"
#include "DX12Image.h"
#include "PipelineBase.h"

class PipelineBRDFLUT final : public PipelineBase
{
public:
	PipelineBRDFLUT(
		DX12Context& ctx);
	~PipelineBRDFLUT() = default;

	void Update(DX12Context& ctx, UIData& uiData) override {}
	void PopulateCommandList(DX12Context& ctx) override {};

	void Execute(DX12Context& ctx,
		DX12Image* lut);

private:
	void CreateDescriptors(DX12Context& ctx,
		DX12Image* lut);

	void GenerateShader(DX12Context& ctx);

	void CreatePipeline(DX12Context& ctx);
};

#endif