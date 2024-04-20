#include "PipelineSimple.h"
#include "DX12Exception.h"

#include "ConstantBuffers.h"

PipelineSimple::PipelineSimple(
	DX12Context& ctx, 
	Scene* scene, 
	Camera* camera,
	ResourcesShared* resourcesShared,
	ResourcesLights* resourcesLights) :
	PipelineBase(ctx),
	scene_(scene),
	camera_(camera),
	resourcesShared_(resourcesShared),
	resourcesLights_(resourcesLights)
{
	CreateSRV(ctx);
	CreateRootSignature(ctx);
	CreateConstantBuffer(ctx);
	CreateShaders(ctx);
	CreateGraphicsPipeline(ctx);
}

void PipelineSimple::Destroy()
{
	for (auto& buff : constantBuffers_)
	{
		buff.Destroy();
	}
}

void PipelineSimple::CreateSRV(DX12Context& ctx)
{
	constexpr uint32_t srvCount = 2;

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = srvCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	};
	ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap_)))

	uint32_t incrementSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle1(srvHeap_->GetCPUDescriptorHandleForHeapStart(), 0, incrementSize); // Texture
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle2(srvHeap_->GetCPUDescriptorHandleForHeapStart(), 1, incrementSize); // Lights

	// Texture
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc1 =
	{
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM, // Image format
		.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	};
	srvDesc1.Texture2D.MipLevels = 1;
	auto imageResource = scene_->model_.meshes_[0].image_->buffer_.resource_.Get();
	ctx.GetDevice()->CreateShaderResourceView(imageResource, &srvDesc1, handle1);

	// Lights
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2 = 
	{
		.Format = DXGI_FORMAT_UNKNOWN,
		.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	};
	srvDesc2.Buffer.FirstElement = 0;
	srvDesc2.Buffer.NumElements = resourcesLights_->lights_.size();
	srvDesc2.Buffer.StructureByteStride = sizeof(LightData);
	srvDesc2.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	ctx.GetDevice()->CreateShaderResourceView(
		resourcesLights_->buffer_.resource_.Get(),
		&srvDesc2,
		handle2);
}

void PipelineSimple::CreateConstantBuffer(DX12Context& ctx)
{
	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		constantBuffers_[i].CreateConstantBuffer(ctx, sizeof(CBMVP));
	}
}

void PipelineSimple::CreateRootSignature(DX12Context& ctx)
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, 
	// the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(ctx.GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

	CD3DX12_ROOT_PARAMETER1 rootParameters[3];
	rootParameters[0].InitAsConstantBufferView(0, 0);
	rootParameters[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[2].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);

	// Image
	D3D12_STATIC_SAMPLER_DESC sampler = scene_->model_.meshes_[0].image_->GetSampler();

	// Allow input layout and deny uneccessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error))
	ThrowIfFailed(ctx.GetDevice()->CreateRootSignature(
		0, 
		signature->GetBufferPointer(), 
		signature->GetBufferSize(), 
		IID_PPV_ARGS(&rootSignature_)))
}

void PipelineSimple::CreateShaders(DX12Context& ctx)
{
	vertexShader_.Create(ctx, AppConfig::ShaderFolder + "Shader.hlsl", ShaderType::Vertex);
	fragmentShader_.Create(ctx, AppConfig::ShaderFolder + "Shader.hlsl", ShaderType::Fragment);
}

void PipelineSimple::CreateGraphicsPipeline(DX12Context& ctx)
{
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs = VertexData::GetInputElementDescriptions();

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc =
	{
		.pRootSignature = rootSignature_.Get(),
		.VS = CD3DX12_SHADER_BYTECODE(vertexShader_.GetHandle()),
		.PS = CD3DX12_SHADER_BYTECODE(fragmentShader_.GetHandle()),
		.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		.SampleMask = UINT_MAX,
		.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
		.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT),
		.InputLayout = { inputElementDescs.data(), static_cast<uint32_t>(inputElementDescs.size()) },
		.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		.NumRenderTargets = 1,
		.DSVFormat = DXGI_FORMAT_D32_FLOAT,
	};
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(ctx.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_)))
}

void PipelineSimple::Update(DX12Context& ctx)
{
	CBMVP cb = {
		.worldMatrix = glm::transpose(glm::mat4(1.0)),
		.viewMatrix = glm::transpose(camera_->GetViewMatrix()),
		.projectionMatrix = glm::transpose(camera_->GetProjectionMatrix())
	};
	constantBuffers_[ctx.GetFrameIndex()].UploadData(&cb);
}

void PipelineSimple::PopulateCommandList(DX12Context& ctx)
{
	ID3D12GraphicsCommandList* commandList = ctx.GetCommandList();

	commandList->SetPipelineState(pipelineState_.Get());
	commandList->RSSetViewports(1, &viewport_);
	commandList->RSSetScissorRects(1, &scissor_);
	commandList->SetGraphicsRootSignature(rootSignature_.Get());

	// Descriptors
	uint32_t incrementSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_GPU_DESCRIPTOR_HANDLE handle1(srvHeap_->GetGPUDescriptorHandleForHeapStart(), 0, incrementSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE handle2(srvHeap_->GetGPUDescriptorHandleForHeapStart(), 1, incrementSize);
	ID3D12DescriptorHeap* ppHeaps[] = { srvHeap_.Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	commandList->SetGraphicsRootConstantBufferView(0, constantBuffers_[ctx.GetFrameIndex()].gpuAddress_);
	commandList->SetGraphicsRootDescriptorTable(1, handle1);
	commandList->SetGraphicsRootDescriptorTable(2, handle2);
	
	// Indicate that the back buffer will be used as a render target.
	{
		auto resourceBarrier = 
			CD3DX12_RESOURCE_BARRIER::Transition(
				//renderTargets_[ctx.GetFrameIndex()].Get(), 
				resourcesShared_->GetRenderTarget(ctx.GetFrameIndex()),
				D3D12_RESOURCE_STATE_PRESENT, 
				D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &resourceBarrier);
	}

	auto rtvHandle = resourcesShared_->GetRTVHandle(ctx.GetFrameIndex());
	auto dsvHandle = resourcesShared_->GetDSVHandle();
	constexpr uint32_t renderTargetCount = 1;
	commandList->OMSetRenderTargets(renderTargetCount, &rtvHandle, FALSE, &dsvHandle);

	// TODO Only one mesh for now
	Mesh& mesh = scene_->model_.meshes_[0];

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &(mesh.vertexBuffer_.vertexBufferView_));
	commandList->IASetIndexBuffer(&mesh.indexBuffer_.indexBufferView_);
	commandList->DrawIndexedInstanced(mesh.vertexCount_, 1, 0, 0, 0);

	// Indicate that the back buffer will now be used to present
	{
		auto resourceBarrier = 
			CD3DX12_RESOURCE_BARRIER::Transition(
				resourcesShared_->GetRenderTarget(ctx.GetFrameIndex()),
				D3D12_RESOURCE_STATE_RENDER_TARGET, 
				D3D12_RESOURCE_STATE_PRESENT);
		commandList->ResourceBarrier(1, &resourceBarrier);
	}
}