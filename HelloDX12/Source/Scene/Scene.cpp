#include "Scene.h"
#include "DX12Exception.h"

#include "glm/glm.hpp"

void Scene::Init(DX12Context& ctx)
{
	model_.Load(ctx, AppConfig::ModelFolder + "HorseStatue/HorseStatue.gltf");
}