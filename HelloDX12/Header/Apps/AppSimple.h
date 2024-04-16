#ifndef HELLO_DX12_APP_SIMPLE
#define HELLO_DX12_APP_SIMPLE

#include "AppBase.h"
#include "DX12Context.h"
#include "PipelineSimple.h"
#include "Scene.h"

#include <memory>

using Microsoft::WRL::ComPtr;

class AppSimple : public AppBase
{
public:
	AppSimple();

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();

private:
	DX12Context context_;
	std::unique_ptr<Scene> scene_ = nullptr;
	std::unique_ptr<PipelineSimple> pip_ = nullptr;

private:
	void PopulateCommandList();
};

#endif