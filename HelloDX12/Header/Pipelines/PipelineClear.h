#ifndef HELLO_DX12_PIPELINE_CLEAR
#define HELLO_DX12_PIPELINE_CLEAR

#include "DX12Context.h"
#include "PipelineBase.h"
#include "ResourcesShared.h"

class PipelineClear final : public PipelineBase
{
public:
	PipelineClear(
		DX12Context& ctx,
		ResourcesShared* resourcesShared);
	~PipelineClear() = default;

	void Update(DX12Context& ctx, UIData& uiData) override {}
	void PopulateCommandList(DX12Context& ctx) override;

private:
	ResourcesShared* resourcesShared_{};
};

#endif