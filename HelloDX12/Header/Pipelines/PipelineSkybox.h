#ifndef HELLO_DX12_PIPELINE_SKYBOX
#define HELLO_DX12_PIPELINE_SKYBOX

#include "DX12Context.h"
#include "PipelineBase.h"
#include "ResourcesIBL.h"

class PipelineSkybox final : public PipelineBase
{
public:
	PipelineSkybox(
		DX12Context& ctx,
		ResourcesIBL* resourcesIBL);
	~PipelineSkybox() = default;

	void Update(DX12Context& ctx) override;
	void PopulateCommandList(DX12Context& ctx) override;

private:
	void GenerateShader(DX12Context& ctx);
	void CreatePipeline(DX12Context& ctx);

private:
	ResourcesIBL* resourcesIBL_;
};

#endif