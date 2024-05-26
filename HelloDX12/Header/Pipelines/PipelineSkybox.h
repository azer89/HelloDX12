#ifndef HELLO_DX12_PIPELINE_SKYBOX
#define HELLO_DX12_PIPELINE_SKYBOX

#include "DX12Context.h"
#include "PipelineBase.h"
#include "ResourcesIBL.h"
#include "ResourcesShared.h"
#include "Camera.h"

#include <array>

class PipelineSkybox final : public PipelineBase
{
public:
	PipelineSkybox(
		DX12Context& ctx,
		ResourcesIBL* resourcesIBL,
		ResourcesShared* resourcesShared,
		Camera* camera);
	~PipelineSkybox();

	void Destroy() override;
	void Update(DX12Context& ctx) override;
	void PopulateCommandList(DX12Context& ctx) override;

	void OnWindowResize(DX12Context& ctx, uint32_t width, uint32_t height) override
	{
		for (auto& dHeap : descriptorHeaps_)
		{
			dHeap.SetupHandles(ctx);
		}
	}

private:
	void CreateDescriptors(DX12Context& ctx);
	void GenerateShader(DX12Context& ctx);
	void CreatePipeline(DX12Context& ctx);
	void CreateConstantBuffer(DX12Context& ctx);

private:
	ResourcesIBL* resourcesIBL_ = nullptr;
	ResourcesShared* resourcesShared_ = nullptr;
	Camera* camera_ = nullptr;

	std::array<DX12DescriptorHeap, AppConfig::FrameCount> descriptorHeaps_ = {};
	std::array<DX12Buffer, AppConfig::FrameCount> constBuffCamera_ = {};
};

#endif