#ifndef HELLO_DX12_PIPELINE_BRDF_LUT
#define HELLO_DX12_PIPELINE_BRDF_LUT

#include "DX12Context.h"
#include "PipelineBase.h"

class PipelineBRDFLUT final : public PipelineBase
{
public:
	PipelineBRDFLUT(
		DX12Context& ctx);
	~PipelineBRDFLUT() = default;

	void Update(DX12Context& ctx, UIData& uiData) override {}
	void PopulateCommandList(DX12Context& ctx) override {};

private:
};

#endif