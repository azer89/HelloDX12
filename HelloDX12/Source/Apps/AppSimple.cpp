#include "AppSimple.h"
#include "PipelineClear.h"
#include "PipelinePresent.h"

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
	scene_->Init(context_);

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
	pipSimple_ = AddPipeline<PipelineSimple>(
		context_, 
		scene_.get(), 
		camera_.get(), 
		resourcesShared_,
		resourcesLights_);
	AddPipeline<PipelinePresent>(context_, resourcesShared_);
}

// Update frame-based values.
void AppSimple::OnUpdate()
{
	OnKeyboardInput();

	for (auto& pip : pipelines_)
	{
		pip->Update(context_);
	}
}

// Render the scene.
void AppSimple::OnRender()
{
	context_.ResetCommandAllocator();
	context_.ResetCommandList();

	for (auto& pip : pipelines_)
	{
		pip->PopulateCommandList(context_);
	}

	context_.SubmitCommandList();
	context_.PresentSwapchain();
	context_.MoveToNextFrame();
}

void AppSimple::OnDestroy()
{
	context_.WaitForGPU();
	scene_->Destroy();

	for (auto& pip : pipelines_)
	{
		pip.reset();
	}
	for (auto& res : resources_)
	{
		res.reset();
	}

	context_.Destroy();
}