#include "PipelineSimple.h"
#include "DX12Exception.h"

PipelineSimple::PipelineSimple(DX12Context& ctx, Scene* scene) : scene_(scene)
{
	viewport_ = CD3DX12_VIEWPORT(
		0.0f, 
		0.0f, 
		static_cast<float>(ctx.swapchainWidth_), 
		static_cast<float>(ctx.swapchainHeight_));
	scissor_ = CD3DX12_RECT(
		0, 
		0, 
		static_cast<LONG>(ctx.swapchainWidth_),
		static_cast<LONG>(ctx.swapchainHeight_));

	CreateSRV(ctx);
	CreateRTV(ctx);
	CreateRootSignature(ctx);
	CreateShaders(ctx);
	CreateGraphicsPipeline(ctx);
}

void PipelineSimple::CreateSRV(DX12Context& ctx)
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = 1,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	};
	ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap_)));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
	{
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM, // Image format
		.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	};
	srvDesc.Texture2D.MipLevels = 1;
	ctx.GetDevice()->CreateShaderResourceView(scene_->image_->image_.Get(), &srvDesc, srvHeap_->GetCPUDescriptorHandleForHeapStart());
}

void PipelineSimple::CreateRTV(DX12Context& ctx)
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = AppConfig::FrameCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap_)));

	// Create a RTV for each frame
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap_->GetCPUDescriptorHandleForHeapStart());
	rtvDescriptorSize_ = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (uint32_t n = 0; n < AppConfig::FrameCount; n++)
	{
		ThrowIfFailed(ctx.swapchain_->GetBuffer(n, IID_PPV_ARGS(&renderTargets_[n])));
		ctx.GetDevice()->CreateRenderTargetView(renderTargets_[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, rtvDescriptorSize_);
	}
}

void PipelineSimple::CreateRootSignature(DX12Context& ctx)
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(ctx.GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
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
	ThrowIfFailed(ctx.GetDevice()->CreateRootSignature(
		0, 
		signature->GetBufferPointer(), 
		signature->GetBufferSize(), 
		IID_PPV_ARGS(&rootSignature_)));
}

void PipelineSimple::CreateShaders(DX12Context& ctx)
{
	vertexShader_.Create(ctx, AppConfig::ShaderFolder + "Shader.hlsl", ShaderType::Vertex);
	fragmentShader_.Create(ctx, AppConfig::ShaderFolder + "Shader.hlsl", ShaderType::Fragment);
}

void PipelineSimple::CreateGraphicsPipeline(DX12Context& ctx)
{
	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc =
	{
		.pRootSignature = rootSignature_.Get(),
		.VS = CD3DX12_SHADER_BYTECODE(vertexShader_.GetHandle()),
		.PS = CD3DX12_SHADER_BYTECODE(fragmentShader_.GetHandle()),
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
	ThrowIfFailed(ctx.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_)));
}

void PipelineSimple::PopulateCommandList(DX12Context& ctx)
{
	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(ctx.commandList_->Reset(ctx.commandAllocator_.Get(), pipelineState_.Get()));

	// Set necessary state.
	ctx.commandList_->SetGraphicsRootSignature(rootSignature_.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { srvHeap_.Get() };
	ctx.commandList_->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	ctx.commandList_->SetGraphicsRootDescriptorTable(0, srvHeap_->GetGPUDescriptorHandleForHeapStart());
	ctx.commandList_->RSSetViewports(1, &viewport_);
	ctx.commandList_->RSSetScissorRects(1, &scissor_);

	// Indicate that the back buffer will be used as a render target.
	{
		auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets_[ctx.frameIndex_].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		ctx.commandList_->ResourceBarrier(1, &resourceBarrier);
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap_->GetCPUDescriptorHandleForHeapStart(), ctx.frameIndex_, rtvDescriptorSize_);
	ctx.commandList_->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	ctx.commandList_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	ctx.commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx.commandList_->IASetVertexBuffers(0, 1, &(scene_->vertexBufferView_));
	ctx.commandList_->DrawInstanced(3, 1, 0, 0);

	// Indicate that the back buffer will now be used to present
	{
		auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets_[ctx.frameIndex_].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		ctx.commandList_->ResourceBarrier(1, &resourceBarrier);
	}
}