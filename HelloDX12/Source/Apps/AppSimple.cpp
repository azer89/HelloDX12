#include "AppSimple.h"
#include "PipelineClear.h"
#include "PipelinePresent.h"
#include "PipelineTonemap.h"
#include "PipelineResolve.h"
#include "PipelineSkybox.h"

#include <iostream>

AppSimple::AppSimple() : AppBase()
{
}

void AppSimple::OnInit()
{
	// Initialize Context
	context_.Init(windowWidth_, windowHeight_);
	context_.CreateFence();

	// Camera
	camera_->SetPositionAndTarget(glm::vec3(1.0f, 3.0f, 5.0f), glm::vec3(0.0, 1.5, 0.0));

	// Scene
	scene_ = std::make_unique<Scene>();
	scene_->Init(context_, AppConfig::ModelFolder + "DamagedHelmet/DamagedHelmet.gltf");

	// IBL
	resourcesIBL = AddResources<ResourcesIBL>(context_, AppConfig::TextureFolder + "piazza_bologni_1k.hdr");

	// Render target and depth
	resourcesShared_ = AddResources<ResourcesShared>();
	resourcesShared_->Init(context_);

	// Lights
	resourcesLights_ = AddResources<ResourcesLights>();
	resourcesLights_->AddLights(context_,
	{
		{.position_ = glm::vec4(-1.5f, 3.5f,  1.5f, 1.f), .color_ = glm::vec4(1.f, 1.f, 1.f, 1.f), .radius_ = 10.0f },
		{.position_ = glm::vec4(1.5f, 3.5f,  1.5f, 1.f), .color_ = glm::vec4(1.f, 1.f, 1.f, 1.f), .radius_ = 10.0f },
		{.position_ = glm::vec4(-1.5f, 3.5f, -1.5f, 1.f), .color_ = glm::vec4(1.f, 1.f, 1.f, 1.f), .radius_ = 10.0f },
		{.position_ = glm::vec4(1.5f, 3.5f, -1.5f, 1.f), .color_ = glm::vec4(1.f, 1.f, 1.f, 1.f), .radius_ = 10.0f }
	});

	// Pipelines
	AddPipeline<PipelineClear>(context_, resourcesShared_);
	AddPipeline<PipelineSkybox>(context_, resourcesIBL, resourcesShared_, camera_.get());
	pipSimple_ = AddPipeline<PipelineSimple>(
		context_, 
		scene_.get(), 
		camera_.get(), 
		resourcesShared_,
		resourcesIBL,
		resourcesLights_);
	AddPipeline<PipelineResolve>(context_, resourcesShared_);
	AddPipeline<PipelineTonemap>(context_, resourcesShared_);
	imguiPtr_ = AddPipeline<PipelineImGui>(context_, resourcesShared_);
	AddPipeline<PipelinePresent>(context_, resourcesShared_);
}

void AppSimple::OnUpdate()
{
	OnKeyboardInput();

	if (!uiData_.imguiShow_)
	{
		imguiPtr_->ImGuiDrawEmpty();
		return;
	}

	imguiPtr_->ImGuiStart();
	imguiPtr_->ImGuiSetWindow("Simple Renderer", 450, 750);
	imguiPtr_->ImGuiShowFrameData(&timer_);
	imguiPtr_->ImGuiShowPBRConfig(&(uiData_.constBufferPBR_));
	imguiPtr_->ImGuiEnd();

	for (auto& pip : pipelines_)
	{
		pip->Update(context_, uiData_);
	}
}

// Render the scene.
void AppSimple::OnRender()
{
	BeginRender();

	for (auto& pip : pipelines_)
	{
		pip->PopulateCommandList(context_);
	}

	EndRender();
}

void AppSimple::OnDestroy()
{
	context_.WaitForGPU();
	scene_->Destroy();
	for (auto& pip : pipelines_) { pip.reset(); }
	for (auto& res : resources_) { res.reset(); }
	context_.Destroy();
}