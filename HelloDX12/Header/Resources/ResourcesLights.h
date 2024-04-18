#ifndef HELLO_DX12_RESOURCES_LIGHTS
#define HELLO_DX12_RESOURCES_LIGHTS

#include "ResourcesBase.h"

#include "glm/glm.hpp"

#include <vector>

struct LightData
{
	glm::vec4 position_;
	glm::vec4 color_;
};

class ResourcesLights final : ResourcesBase
{
public:
	ResourcesLights() = default;
	~ResourcesLights() = default;

public:
	std::vector<LightData> lights_ = {};
};

#endif