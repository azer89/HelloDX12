#ifndef HELLO_DX12_PIPELINE_TONEMAP
#define HELLO_DX12_PIPELINE_TONEMAP

#include "DX12Context.h"
#include "PipelineBase.h"
#include "ResourcesShared.h"

class PipelineTonemap final : public PipelineBase
{
public:
	PipelineTonemap(
		DX12Context& ctx,
		ResourcesShared* resourcesShared);
	~PipelineTonemap() = default;

	void Update(DX12Context& ctx) override {}
	void PopulateCommandList(DX12Context& ctx) override;

private:
	ResourcesShared* resourcesShared_;
};

#endif