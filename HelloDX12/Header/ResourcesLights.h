#ifndef HELLO_DX12_RESOURCES_LIGHTS
#define HELLO_DX12_RESOURCES_LIGHTS

#include "glm/glm.hpp"

#include <vector>

struct LightData
{
	glm::vec4 position_;
	glm::vec4 color_;
};

struct ResourcesLights
{
public:
	ResourcesLights() = default;
	~ResourcesLights() = default;

public:
	std::vector<LightData> lights_ = {};
};

#endif