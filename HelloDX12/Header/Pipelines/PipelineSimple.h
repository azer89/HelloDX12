#ifndef HELLO_DX12_PIPELINE_SIMPLE
#define HELLO_DX12_PIPELINE_SIMPLE

#include "DX12Context.h"
#include "DX12Shader.h"
#include "DX12Buffer.h"
#include "PipelineBase.h"
#include "ResourcesShared.h"
#include "ResourcesLights.h"
#include "Scene.h"
#include "Camera.h"
#include "Configs.h"

#include "glm/glm.hpp"

#include <array>

class PipelineSimple final : PipelineBase
{
public:
	PipelineSimple(
		DX12Context& ctx, 
		Scene* scene, 
		Camera* camera, 
		ResourcesShared* resourcesShared,
		ResourcesLights* resourcesLights);
	~PipelineSimple() = default;

	void Update(DX12Context& ctx) override;
	void PopulateCommandList(DX12Context& ctx) override;

	void Destroy() override;

private:
	void CreateSRV(DX12Context& ctx);
	void CreateConstantBuffer(DX12Context& ctx);
	void CreateRootSignature(DX12Context& ctx);
	void CreateShaders(DX12Context& ctx);
	void CreateGraphicsPipeline(DX12Context& ctx);

public:
	
	Scene* scene_;
	Camera* camera_;
	ResourcesShared* resourcesShared_;
	ResourcesLights* resourcesLights_;

private:
	std::array<DX12Buffer, AppConfig::FrameCount> constantBuffers_;
};

#endif