#include "AppSimple.h"
#include "DX12Exception.h"

AppSimple::AppSimple() : AppBase()
{
}

void AppSimple::OnInit()
{
	// Initialize Context
	context_.Init(width_, height_);
	context_.CreateFence();

	// Camera
	camera_->SetPositionAndTarget(glm::vec3(1.0f, 3.0f, 5.0f), glm::vec3(0.0));

	// Scene
	scene_ = std::make_unique<Scene>();
	scene_->Init(context_);

	// Render target and depth
	resourcesShared_ = std::make_unique<ResourcesShared>();

	// Lights
	resourcesLights_ = std::make_unique<ResourcesLights>();
	resourcesLights_->AddLights(context_,
	{
		{.position_ = glm::vec4(-1.5f, 0.7f,  1.5f, 1.f), .color_ = glm::vec4(1.f, 0.0f, 0.0f, 1.0f), .radius_ = 10.0f },
		{.position_ = glm::vec4(1.5f, 0.7f,  1.5f, 1.f), .color_ = glm::vec4(0.f, 1.0f, 0.0f, 1.0f), .radius_ = 10.0f },
		{.position_ = glm::vec4(-1.5f, 0.7f, -1.5f, 1.f), .color_ = glm::vec4(0.f, 0.0f, 1.0f, 1.0f), .radius_ = 10.0f },
		{.position_ = glm::vec4(1.5f, 0.7f, -1.5f, 1.f), .color_ = glm::vec4(1.f, 0.0f, 1.0f, 1.0f), .radius_ = 10.0f }
	});

	// Pipelines
	pip_ = std::make_unique<PipelineSimple>(
		context_, scene_.get(), 
		camera_.get(), 
		resourcesShared_.get(),
		resourcesLights_.get());
}

// Update frame-based values.
void AppSimple::OnUpdate()
{
	OnKeyboardInput();
	
	pip_->Update(context_);
}

// Render the scene.
void AppSimple::OnRender()
{
	PopulateCommandList();
	context_.SubmitCommandList();
	context_.PresentSwapchain();
	context_.MoveToNextFrame();
}

void AppSimple::PopulateCommandList()
{
	context_.ResetCommandAllocator();
	context_.ResetCommandList();

	pip_->PopulateCommandList(context_);
}

void AppSimple::OnDestroy()
{
	context_.WaitForGPU();
	pip_->Destroy();
	scene_->Destroy();
	resourcesLights_->Destroy();
	context_.Destroy();
}