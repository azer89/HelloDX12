#ifndef HELLO_DX12_PIPELINE_RESOLVE
#define HELLO_DX12_PIPELINE_RESOLVE

#include "DX12Context.h"
#include "PipelineBase.h"
#include "ResourcesShared.h"

class PipelineResolve final : public PipelineBase
{
public:
	PipelineResolve(
		DX12Context& ctx,
		ResourcesShared* resourcesShared);
	~PipelineResolve() = default;

	void Update(DX12Context& ctx, UIData& uiData) override {}
	void PopulateCommandList(DX12Context& ctx) override;

private:
	ResourcesShared* resourcesShared_{};
};

#endif