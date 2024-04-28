#ifndef HELLO_DX12_PIPELINE_EQUIRECT_TO_CUBE
#define HELLO_DX12_PIPELINE_EQUIRECT_TO_CUBE

#include "DX12Context.h"
#include "PipelineBase.h"
#include "ResourcesIBL.h"

class PipelineEquirect2Cube final : public PipelineBase
{
public:
	PipelineEquirect2Cube(
		DX12Context& ctx);
	~PipelineEquirect2Cube() = default;

	void Update(DX12Context& ctx) override {}
	void PopulateCommandList(DX12Context& ctx) override;

private:
	ResourcesIBL* resourcesIBL_;
};

#endif