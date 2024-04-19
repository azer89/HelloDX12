#ifndef HELLO_DX12_RESOURCES_LIGHTS
#define HELLO_DX12_RESOURCES_LIGHTS

#include "ResourcesBase.h"
#include "DX12Context.h"
#include "DX12Buffer.h"

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

	void Destroy();

	void AddLights(DX12Context& ctx, const std::vector<LightData>& lights);

public:
	std::vector<LightData> lights_ = {};
	DX12Buffer buffer_;
};

#endif