#ifndef HELLO_DX12_RESOURCES_IBL
#define HELLO_DX12_RESOURCES_IBL

#include "ResourcesBase.h"
#include "DX12Context.h"
#include "DX12Image.h"

#include <string>

class ResourcesIBL final : public ResourcesBase
{
public:
	ResourcesIBL(DX12Context& ctx, const std::string& hdrFile);
	~ResourcesIBL();

	void Destroy() override;

public:
	DX12Image hdrImage_{};
	DX12Image environmentCubemap_{};
};

#endif