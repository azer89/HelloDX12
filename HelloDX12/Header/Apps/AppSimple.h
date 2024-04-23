#ifndef HELLO_DX12_APP_SIMPLE
#define HELLO_DX12_APP_SIMPLE

#include "AppBase.h"
#include "DX12Context.h"
#include "PipelineSimple.h"
#include "ResourcesShared.h"
#include "ResourcesLights.h"
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
	DX12Context context_;
	std::unique_ptr<Scene> scene_ = nullptr;
	PipelineSimple* pipSimple_ = nullptr;
	ResourcesShared* resourcesShared_ = nullptr;
	ResourcesLights* resourcesLights_ = nullptr;
};

#endif