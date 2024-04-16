#include "PipelineSimple.h"
#include "DX12Exception.h"

PipelineSimple::PipelineSimple(DX12Context& ctx, Scene* scene, Camera* camera) : 
	scene_(scene),
	camera_(camera),
	viewport_(ctx.GetViewport()),
	scissor_(ctx.GetScissor())
{
	CreateSRV(ctx);
	CreateRTV(ctx);
	CreateDSV(ctx);
	CreateRootSignature(ctx);
	CreateConstantBuffer(ctx);
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

void PipelineSimple::CreateDSV(DX12Context& ctx)
{
	// Describe and create a depth stencil view (DSV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = 
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		.NumDescriptors = 1,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap_)));

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = 
	{
		.Format = DXGI_FORMAT_D32_FLOAT,
		.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
		.Flags = D3D12_DSV_FLAG_NONE
	};

	CD3DX12_HEAP_PROPERTIES heapProperties = 
		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	// Performance tip: Deny shader resource access to resources that don't need shader resource views.
	CD3DX12_RESOURCE_DESC resourceDescription = 
		CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, 
			ctx.swapchainWidth_, 
			ctx.swapchainHeight_, 
			1, 
			0, 
			1, 
			0, 
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);
	// Performance tip: Tell the runtime at resource creation the desired clear value.
	CD3DX12_CLEAR_VALUE clearValue = 
		CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0); 

	ThrowIfFailed(ctx.GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDescription,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&depthStencil_)
	));

	ctx.GetDevice()->CreateDepthStencilView(
		depthStencil_.Get(), 
		&depthStencilDesc, 
		dsvHeap_->GetCPUDescriptorHandleForHeapStart());
}

void PipelineSimple::CreateConstantBuffer(DX12Context& ctx)
{
	const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	size_t cbSize = AppConfig::FrameCount * sizeof(PaddedConstantBuffer);

	const D3D12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(cbSize);
	ThrowIfFailed(ctx.GetDevice()->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&constantBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(perFrameConstants_.ReleaseAndGetAddressOf())));

	ThrowIfFailed(perFrameConstants_->Map(0, nullptr, reinterpret_cast<void**>(&mappedConstantData_)));

	// GPU virtual address of the resource
	constantDataGpuAddr_ = perFrameConstants_->GetGPUVirtualAddress();
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

	CD3DX12_ROOT_PARAMETER1 rootParameters[2];
	rootParameters[0].InitAsConstantBufferView(0, 0);
	rootParameters[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

	// Image
	D3D12_STATIC_SAMPLER_DESC sampler = scene_->image_->GetSampler();

	// Allow input layout and deny uneccessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	//rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

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
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
	ctx.SetPipelineState(pipelineState_.Get());

	// Set necessary state.
	ctx.commandList_->SetGraphicsRootSignature(rootSignature_.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { srvHeap_.Get() };
	ctx.commandList_->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// Push constant
	ConstantBuffer cb = {
		.worldMatrix = glm::transpose(glm::mat4(1.0)),
		.viewMatrix = glm::transpose(camera_->GetViewMatrix()),
		.projectionMatrix = glm::transpose(camera_->GetProjectionMatrix())
	};
	memcpy(&mappedConstantData_[ctx.frameIndex_], &cb, sizeof(ConstantBuffer));
	auto baseGpuAddress = constantDataGpuAddr_ + sizeof(PaddedConstantBuffer) * ctx.frameIndex_;
	ctx.commandList_->SetGraphicsRootConstantBufferView(0, baseGpuAddress);

	ctx.commandList_->SetGraphicsRootDescriptorTable(1, srvHeap_->GetGPUDescriptorHandleForHeapStart());
	ctx.commandList_->RSSetViewports(1, &viewport_);
	ctx.commandList_->RSSetScissorRects(1, &scissor_);

	// Indicate that the back buffer will be used as a render target.
	{
		auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets_[ctx.frameIndex_].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		ctx.commandList_->ResourceBarrier(1, &resourceBarrier);
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap_->GetCPUDescriptorHandleForHeapStart(), ctx.frameIndex_, rtvDescriptorSize_);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap_->GetCPUDescriptorHandleForHeapStart());
	ctx.commandList_->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	ctx.commandList_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	ctx.commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	ctx.commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx.commandList_->IASetVertexBuffers(0, 1, &(scene_->vertexBufferView_));
	//ctx.commandList_->DrawInstanced(3, 1, 0, 0);
	ctx.commandList_->IASetIndexBuffer(&scene_->indexBufferView_);
	ctx.commandList_->DrawIndexedInstanced(6, 1, 0, 0, 0);

	// Indicate that the back buffer will now be used to present
	{
		auto resourceBarrier = 
			CD3DX12_RESOURCE_BARRIER::Transition(
				renderTargets_[ctx.frameIndex_].Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, 
				D3D12_RESOURCE_STATE_PRESENT);
		ctx.commandList_->ResourceBarrier(1, &resourceBarrier);
	}
}