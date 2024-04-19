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

	// Pipelines
	pip_ = std::make_unique<PipelineSimple>(context_, scene_.get(), camera_.get());
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
	context_.Destroy();
}