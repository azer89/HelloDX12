#include "AppSimple.h"
#include "DX12Exception.h"

AppSimple::AppSimple() : AppBase()
{
}

void AppSimple::OnInit()
{
	// Initialize Context
	context_.Init(width_, height_);

	// Scene
	scene_ = std::make_unique<Scene>();
	scene_->Init(context_);

	pip_ = std::make_unique<PipelineSimple>(context_, scene_.get());

	// Close the command list and execute it to begin the initial GPU setup.
	ThrowIfFailed(context_.commandList_->Close());
	ID3D12CommandList* ppCommandLists[] = { context_.commandList_.Get() };
	context_.commandQueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		ThrowIfFailed(context_.device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&context_.fence_)));
		context_.fenceValues_[context_.frameIndex_]++;

		// Create an event handle to use for frame synchronization.
		context_.fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (context_.fenceEvent_ == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		context_.WaitForGpu();
	}
}

// Update frame-based values.
void AppSimple::OnUpdate()
{
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
	ThrowIfFailed(context_.commandAllocators_[context_.frameIndex_]->Reset());

	pip_->PopulateCommandList(context_);
	
	ThrowIfFailed(context_.commandList_->Close());
}