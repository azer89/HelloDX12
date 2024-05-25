#ifndef HELLO_DX12_APP_SIMPLE
#define HELLO_DX12_APP_SIMPLE

#include "AppBase.h"
#include "PipelineSimple.h"
#include "PipelineImGui.h"
#include "ResourcesShared.h"
#include "ResourcesLights.h"
#include "ResourcesIBL.h"
#include "Scene.h"

#include <memory>

class AppSimple : public AppBase
{
public:
	AppSimple();

	void OnInit() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnDestroy() override;

private:
	std::unique_ptr<Scene> scene_ = nullptr;
	PipelineSimple* pipSimple_ = nullptr;
	PipelineImGui* imguiPtr_ = nullptr;
	ResourcesShared* resourcesShared_ = nullptr;
	ResourcesLights* resourcesLights_ = nullptr;
	ResourcesIBL* resourcesIBL = nullptr;
};

#endif