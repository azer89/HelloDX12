#ifndef HELLO_DX12_RESOURCES_IBL
#define HELLO_DX12_RESOURCES_IBL

#include "ResourcesBase.h"
#include "DX12Context.h"
#include "DX12Image.h"

#include "glm/glm.hpp"

#include <string>

class ResourcesIBL final : public ResourcesBase
{
public:
	ResourcesIBL(DX12Context& ctx, const std::string& hdrFile);
	~ResourcesIBL();

	void Destroy() override;

public:
	DX12Image hdrImage_ = {};
};

#endif