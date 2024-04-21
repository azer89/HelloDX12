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
	camera_->SetPositionAndTarget(glm::vec3(1.0f, 3.0f, 5.0f), glm::vec3(0.0, 1.5, 0.0));

	// Scene
	scene_ = std::make_unique<Scene>();
	scene_->Init(context_);

	// Render target and depth
	resourcesShared_ = std::make_unique<ResourcesShared>();
	resourcesShared_->Init(context_);

	// Lights
	resourcesLights_ = std::make_unique<ResourcesLights>();
	resourcesLights_->AddLights(context_,
	{
		{.position_ = glm::vec4(-1.5f, 3.5f,  1.5f, 1.f), .color_ = glm::vec4(1.f, 1.f, 1.f, 1.f), .radius_ = 10.0f },
		{.position_ = glm::vec4(1.5f, 3.5f,  1.5f, 1.f), .color_ = glm::vec4(1.f, 1.f, 1.f, 1.f), .radius_ = 10.0f },
		{.position_ = glm::vec4(-1.5f, 3.5f, -1.5f, 1.f), .color_ = glm::vec4(1.f, 1.f, 1.f, 1.f), .radius_ = 10.0f },
		{.position_ = glm::vec4(1.5f, 3.5f, -1.5f, 1.f), .color_ = glm::vec4(1.f, 1.f, 1.f, 1.f), .radius_ = 10.0f }
	});

	// Pipelines
	pipClear_ = std::make_unique<PipelineClear>(context_, resourcesShared_.get());
	pipSimple_ = std::make_unique<PipelineSimple>(
		context_, 
		scene_.get(), 
		camera_.get(), 
		resourcesShared_.get(),
		resourcesLights_.get());
	pipPresent_ = std::make_unique<PipelinePresent>(context_, resourcesShared_.get());
}

// Update frame-based values.
void AppSimple::OnUpdate()
{
	OnKeyboardInput();
	
	pipSimple_->Update(context_);
}

// Render the scene.
void AppSimple::OnRender()
{
	context_.ResetCommandAllocator();
	context_.ResetCommandList();

	pipClear_->PopulateCommandList(context_);
	pipSimple_->PopulateCommandList(context_);
	pipPresent_->PopulateCommandList(context_);

	context_.SubmitCommandList();
	context_.PresentSwapchain();
	context_.MoveToNextFrame();
}

void AppSimple::OnDestroy()
{
	context_.WaitForGPU();
	pipSimple_->Destroy();
	scene_->Destroy();
	resourcesLights_->Destroy();
	context_.Destroy();
}