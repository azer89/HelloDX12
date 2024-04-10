#include "AppSimple.h"
#include "DX12Exception.h"

AppSimple::AppSimple(UINT width, UINT height, std::wstring name) :
	AppBase(width, height, name)
{
}

void AppSimple::OnInit()
{
	LoadPipeline();
	LoadAssets();
}

// Load the rendering pipeline dependencies.
void AppSimple::LoadPipeline()
{
	// Initialize Context
	context_.Init(width_, height_);
}

// Load the sample assets.
void AppSimple::LoadAssets()
{
	// Create the command list.
	ThrowIfFailed(context_.device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, context_.commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&context_.commandList_)));

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
		context_.fenceValue_ = 1;

		// Create an event handle to use for frame synchronization.
		context_.fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (context_.fenceEvent_ == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		context_.WaitForPreviousFrame();
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

	context_.WaitForPreviousFrame();
}

void AppSimple::OnDestroy()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	context_.WaitForPreviousFrame();

	CloseHandle(context_.fenceEvent_);
}

void AppSimple::PopulateCommandList()
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(context_.commandAllocator_->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(context_.commandList_->Reset(context_.commandAllocator_.Get(), pip_->pipelineState_.Get()));

	// Set necessary state.
	context_.commandList_->SetGraphicsRootSignature(pip_->rootSignature_.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { pip_->srvHeap_.Get() };
	context_.commandList_->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	context_.commandList_->SetGraphicsRootDescriptorTable(0, pip_->srvHeap_->GetGPUDescriptorHandleForHeapStart());
	context_.commandList_->RSSetViewports(1, &pip_->viewport_);
	context_.commandList_->RSSetScissorRects(1, &pip_->scissor_);

	// Indicate that the back buffer will be used as a render target.
	{
		auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pip_->renderTargets_[context_.frameIndex_].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		context_.commandList_->ResourceBarrier(1, &resourceBarrier);
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(pip_->rtvHeap_->GetCPUDescriptorHandleForHeapStart(), context_.frameIndex_, pip_->rtvDescriptorSize_);
	context_.commandList_->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	context_.commandList_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	context_.commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context_.commandList_->IASetVertexBuffers(0, 1, &(scene_->vertexBufferView_));
	context_.commandList_->DrawInstanced(3, 1, 0, 0);

	// Indicate that the back buffer will now be used to present
	{
		auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pip_->renderTargets_[context_.frameIndex_].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		context_.commandList_->ResourceBarrier(1, &resourceBarrier);
	}
	
	ThrowIfFailed(context_.commandList_->Close());
}