#include "AppSimple.h"
#include "DX12Exception.h"

AppSimple::AppSimple(UINT width, UINT height, std::wstring name) :
	AppBase(width, height, name)
{
	context_.frameIndex_ = 0;
	context_.viewport_ = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
	context_.scissor_ = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));
	context_.rtvDescriptorSize_ = 0;
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

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc =
		{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			.NumDescriptors = context_.FrameCount,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
		};
		ThrowIfFailed(context_.device_->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&context_.rtvHeap_)));

		// Describe and create a shader resource view (SRV) heap for the texture.
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc =
		{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			.NumDescriptors = 1,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		};
		ThrowIfFailed(context_.device_->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&context_.srvHeap_)));

		context_.rtvDescriptorSize_ = context_.device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(context_.rtvHeap_->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < context_.FrameCount; n++)
		{
			ThrowIfFailed(context_.swapchain_->GetBuffer(n, IID_PPV_ARGS(&context_.renderTargets_[n])));
			context_.device_->CreateRenderTargetView(context_.renderTargets_[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, context_.rtvDescriptorSize_);
		}
	}

	ThrowIfFailed(context_.device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&context_.commandAllocator_)));
}

// Load the sample assets.
void AppSimple::LoadAssets()
{
	// Create the root signature.
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(context_.device_->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		CD3DX12_ROOT_PARAMETER1 rootParameters[1];
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

		D3D12_STATIC_SAMPLER_DESC sampler = 
		{
			.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT,
			.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
			.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
			.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
			.MipLODBias = 0,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
			.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
			.MinLOD = 0.0f,
			.MaxLOD = D3D12_FLOAT32_MAX,
			.ShaderRegister = 0,
			.RegisterSpace = 0,
			.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
		};

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(context_.device_->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&context_.rootSignature_)));
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		std::wstring assetPath = L"C:/Users/azer/workspace/HelloDX12/HelloDX12/Shader/Shader.hlsl";

		ThrowIfFailed(D3DCompileFromFile(assetPath.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(assetPath.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = 
		{
			.pRootSignature = context_.rootSignature_.Get(),
			.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get()),
			.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get()),
			.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT),
			.SampleMask = UINT_MAX,
			.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
			.InputLayout = {inputElementDescs, _countof(inputElementDescs)},
			.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			.NumRenderTargets = 1,
		};
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(context_.device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&context_.pipelineState_)));
	}

	// Create the command list.
	ThrowIfFailed(context_.device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, context_.commandAllocator_.Get(), context_.pipelineState_.Get(), IID_PPV_ARGS(&context_.commandList_)));

	// Scene
	scene_ = std::make_unique<Scene>();
	scene_->Init(context_);
	
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
		WaitForPreviousFrame();
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

	WaitForPreviousFrame();
}

void AppSimple::OnDestroy()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForPreviousFrame();

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
	ThrowIfFailed(context_.commandList_->Reset(context_.commandAllocator_.Get(), context_.pipelineState_.Get()));

	// Set necessary state.
	context_.commandList_->SetGraphicsRootSignature(context_.rootSignature_.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { context_.srvHeap_.Get() };
	context_.commandList_->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	context_.commandList_->SetGraphicsRootDescriptorTable(0, context_.srvHeap_->GetGPUDescriptorHandleForHeapStart());
	context_.commandList_->RSSetViewports(1, &context_.viewport_);
	context_.commandList_->RSSetScissorRects(1, &context_.scissor_);

	// Indicate that the back buffer will be used as a render target.
	{
		auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(context_.renderTargets_[context_.frameIndex_].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		context_.commandList_->ResourceBarrier(1, &resourceBarrier);
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(context_.rtvHeap_->GetCPUDescriptorHandleForHeapStart(), context_.frameIndex_, context_.rtvDescriptorSize_);
	context_.commandList_->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	context_.commandList_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	context_.commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context_.commandList_->IASetVertexBuffers(0, 1, &(scene_->vertexBufferView_));
	context_.commandList_->DrawInstanced(3, 1, 0, 0);

	// Indicate that the back buffer will now be used to present
	{
		auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(context_.renderTargets_[context_.frameIndex_].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		context_.commandList_->ResourceBarrier(1, &resourceBarrier);
	}
	
	ThrowIfFailed(context_.commandList_->Close());
}

void AppSimple::WaitForPreviousFrame()
{
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.

	// Signal and increment the fence value.
	const UINT64 fence = context_.fenceValue_;
	ThrowIfFailed(context_.commandQueue_->Signal(context_.fence_.Get(), fence));
	context_.fenceValue_++;

	// Wait until the previous frame is finished.
	if (context_.fence_->GetCompletedValue() < fence)
	{
		ThrowIfFailed(context_.fence_->SetEventOnCompletion(fence, context_.fenceEvent_));
		WaitForSingleObject(context_.fenceEvent_, INFINITE);
	}

	context_.frameIndex_ = context_.swapchain_->GetCurrentBackBufferIndex();
}