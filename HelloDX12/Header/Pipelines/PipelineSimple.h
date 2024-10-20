#ifndef HELLO_DX12_PIPELINE_SIMPLE
#define HELLO_DX12_PIPELINE_SIMPLE

#include "DX12Context.h"
#include "DX12Buffer.h"
#include "PipelineBase.h"
#include "ResourcesShared.h"
#include "ResourcesLights.h"
#include "ResourcesIBL.h"
#include "Scene.h"
#include "Camera.h"
#include "Configs.h"

#include <array>

class PipelineSimple final : public PipelineBase
{
public:
	PipelineSimple(
		DX12Context& ctx, 
		Scene* scene, 
		Camera* camera, 
		ResourcesShared* resourcesShared,
		ResourcesIBL* resourcesIBL,
		ResourcesLights* resourcesLights);
	~PipelineSimple();

	void Update(DX12Context& ctx, UIData& uiData) override;
	void PopulateCommandList(DX12Context& ctx) override;

	void Destroy() override;

	void OnWindowResize(DX12Context& ctx, uint32_t width, uint32_t height) override
	{
		for (auto& dHeap : descriptorHeaps_)
		{
			dHeap.SetupHandles(ctx);
		}
	}

private:
	void CreateIndirectCommand(DX12Context& ctx);
	void CreateConstantBuffers(DX12Context& ctx);
	void CreateDescriptors(DX12Context& ctx);
	void CreateShaders(DX12Context& ctx);
	void CreateGraphicsPipeline(DX12Context& ctx);

public:
	
	Scene* scene_{};
	Camera* camera_{};
	ResourcesShared* resourcesShared_{};
	ResourcesIBL* resourcesIBL_{};
	ResourcesLights* resourcesLights_{};

private:
	std::array<DX12DescriptorHeap, AppConfig::FrameCount> descriptorHeaps_{};
	std::array<DX12Buffer, AppConfig::FrameCount> constBuffCamera_{};
	std::array<DX12Buffer, AppConfig::FrameCount> constBuffPBR_{};
};

#endif