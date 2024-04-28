#ifndef HELLO_DX12_PIPELINE_SKYBOX
#define HELLO_DX12_PIPELINE_SKYBOX

#include "DX12Context.h"
#include "PipelineBase.h"
#include "ResourcesIBL.h"

class PipelineSkybox final : public PipelineBase
{
public:
	PipelineSkybox(
		DX12Context& ctx);
	~PipelineSkybox() = default;

	void Update(DX12Context& ctx) override {}
	void PopulateCommandList(DX12Context& ctx) override;

private:
	ResourcesIBL* resourcesIBL_;
};

#endif