#include "AppSimple.h"
#include "DX12Exception.h"

AppSimple::AppSimple() : AppBase()
{
}

void AppSimple::OnInit()
{
	// Initialize Context
	context_.Init(width_, height_);

	// Camera
	camera_->SetPositionAndTarget(glm::vec3(1.0f, 3.0f, 5.0f), glm::vec3(0.0));

	// Scene
	scene_ = std::make_unique<Scene>();
	scene_->Init(context_);

	// Pipelines
	pip_ = std::make_unique<PipelineSimple>(context_, scene_.get(), camera_.get());

	// Close the command list and execute it to begin the initial GPU setup.
	ThrowIfFailed(context_.commandList_->Close());
	ID3D12CommandList* ppCommandLists[] = { context_.commandList_.Get() };
	context_.commandQueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	
	context_.CreateFence();
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
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { context_.commandList_.Get() };
	context_.commandQueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(context_.swapchain_->Present(1, 0));

	context_.MoveToNextFrame();
}

void AppSimple::OnDestroy()
{
	context_.WaitForGpu();

	CloseHandle(context_.fenceEvent_);
}

void AppSimple::PopulateCommandList()
{
	context_.ResetCommandAllocator();
	context_.ResetCommandList();

	pip_->PopulateCommandList(context_);
	
	ThrowIfFailed(context_.commandList_->Close());
}