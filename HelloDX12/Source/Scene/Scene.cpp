#include "Scene.h"
#include "DX12Exception.h"

#include "glm/glm.hpp"

#include <iostream>

void Scene::Destroy()
{
	model_.Destroy();
	for (auto& buff : modelConstBuffs_)
	{
		buff.Destroy();
	}
}

void Scene::Init(DX12Context& ctx)
{
	std::string modelFile = AppConfig::ModelFolder + "Zaku/Zaku.gltf";
	std::cout << "Load " << modelFile << '\n';
	model_.Load(ctx, modelFile, sceneData_);

	CreateModelBuffers(ctx);
	CreateBindlessResources(ctx);
}

void Scene::CreateBindlessResources(DX12Context& ctx)
{

}

DX12DescriptorArray Scene::GetImageDescriptors()
{
	DX12DescriptorArray dArray = {
		.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
		.shaderVisibility_ = D3D12_SHADER_VISIBILITY_PIXEL,
	};
	dArray.buffers_.resize(model_.textures_.size());
	dArray.srvDescriptions_.resize(model_.textures_.size());
	
	for (uint32_t i = 0; i < model_.textures_.size(); ++i)
	{
		dArray.buffers_[i] = &(model_.textures_[i].buffer_);
		dArray.srvDescriptions_[i] = model_.textures_[i].buffer_.srvDescription_;
	}

	return dArray;
}

void Scene::CreateModelBuffers(DX12Context& ctx)
{
	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		modelConstBuffs_[i].CreateConstantBuffer(ctx, sizeof(CCamera));
	}

	// TODO Identity matrix for now
	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		modelConsts_[i].modelMatrix = glm::transpose(glm::mat4(1.0f));
		modelConstBuffs_[i].UploadData(&(modelConsts_[i].modelMatrix));
	}
}