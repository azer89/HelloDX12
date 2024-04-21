#ifndef HELLO_DX12_APP_SIMPLE
#define HELLO_DX12_APP_SIMPLE

#include "AppBase.h"
#include "DX12Context.h"
#include "PipelineSimple.h"
#include "PipelineClear.h"
#include "PipelinePresent.h"
#include "ResourcesShared.h"
#include "ResourcesLights.h"
#include "Scene.h"

#include <memory>

using Microsoft::WRL::ComPtr;

class AppSimple : public AppBase
{
public:
	AppSimple();

	void OnInit() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnDestroy() override;

private:
	DX12Context context_;
	std::unique_ptr<Scene> scene_ = nullptr;
	std::unique_ptr<PipelineClear> pipClear_ = nullptr;
	std::unique_ptr<PipelineSimple> pipSimple_ = nullptr;
	std::unique_ptr<PipelinePresent> pipPresent_ = nullptr;
	std::unique_ptr<ResourcesShared> resourcesShared_ = nullptr;
	std::unique_ptr<ResourcesLights> resourcesLights_ = nullptr;

private:
	//void PopulateCommandList();
};

#endif