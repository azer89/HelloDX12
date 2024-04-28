#ifndef HELLO_DX12_RESOURCES_IBL
#define HELLO_DX12_RESOURCES_IBL

#include "ResourcesBase.h"
#include "DX12Context.h"
#include "DX12Image.h"

#include "glm/glm.hpp"

#include <vector>

class ResourcesIBL final : public ResourcesBase
{
public:
	ResourcesIBL() = default;
	~ResourcesIBL();

	void Destroy() override;

public:
};

#endif