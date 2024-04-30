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
	~PipelineSkybox() = default;

	void Update(DX12Context& ctx) override;
	void PopulateCommandList(DX12Context& ctx) override;

private:
	void GenerateShader(DX12Context& ctx);
	void CreateRootSignature(DX12Context& ctx);
	void CreateDescriptorHeap(DX12Context& ctx);
	void CreatePipeline(DX12Context& ctx);
	void CreateConstantBuffer(DX12Context& ctx);

private:
	ResourcesIBL* resourcesIBL_;
	ResourcesShared* resourcesShared_;
	Camera* camera_;

	std::array<DX12Buffer, AppConfig::FrameCount> constBuffCamera_ = {};
};

#endif