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
	std::string modelFile = AppConfig::ModelFolder + "HorseStatue/HorseStatue.gltf";
	std::cout << "Load " << modelFile << '\n';
	model_.Load(ctx, modelFile);

	CreateModelBuffers(ctx);
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