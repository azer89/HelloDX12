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
	~PipelineTonemap();
	void Destroy();

	void Update(DX12Context& ctx, UIData& uiData) override {}
	void PopulateCommandList(DX12Context& ctx) override;

	void OnWindowResize(DX12Context& ctx, uint32_t width, uint32_t height) override
	{
		descriptorHeap_.SetupHandles(ctx);
	}

private:
	void CreateDescriptors(DX12Context& ctx);
	void CreateGraphicsPipeline(DX12Context& ctx);
	void CreateShaders(DX12Context& ctx);

private:
	ResourcesShared* resourcesShared_ = nullptr;
	DX12DescriptorHeap descriptorHeap_ = {};
};

#endif