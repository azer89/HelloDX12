#ifndef HELLO_DX12_PIPELINE_PRESENT
#define HELLO_DX12_PIPELINE_PRESENT

#include "DX12Context.h"
#include "PipelineBase.h"
#include "ResourcesShared.h"

class PipelinePresent final : public PipelineBase
{
public:
	PipelinePresent(
		DX12Context& ctx,
		ResourcesShared* resourcesShared);
	~PipelinePresent() = default;

	void Update(DX12Context& ctx) override {}
	void PopulateCommandList(DX12Context& ctx) override;

private:
	ResourcesShared* resourcesShared_;
};

#endif