#include "Scene.h"
#include "DX12Exception.h"

#include "glm/glm.hpp"

void Scene::Init(DX12Context& ctx)
{
	mesh_.CreateCube(ctx);
}