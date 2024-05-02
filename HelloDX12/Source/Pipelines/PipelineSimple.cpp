#include "PipelineSimple.h"
#include "DX12Exception.h"

#include "ConstantDefinitions.h"

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
	CreateConstantBuffer(ctx);
	CreateDescriptorHeap(ctx);
	CreateShaders(ctx);
	CreateRootSignature(ctx);
	CreateGraphicsPipeline(ctx);
}

PipelineSimple::~PipelineSimple()
{
	Destroy();
}

void PipelineSimple::Destroy()
{
	for (auto& buff : constBuffCamera_)
	{
		buff.Destroy();
	}
}

void PipelineSimple::CreateConstantBuffer(DX12Context& ctx)
{
	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		constBuffCamera_[i].CreateConstantBuffer(ctx, sizeof(CCamera));
	}
}

void PipelineSimple::CreateDescriptorHeap(DX12Context& ctx)
{
	constexpr uint32_t descriptorCount = 6;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = descriptorCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	};
	ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap_)))

	uint32_t incrementSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	uint32_t descriptorOffset = 0;
	
	// Camera
	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(descriptorHeap_->GetCPUDescriptorHandleForHeapStart(), descriptorOffset++, incrementSize);
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc =
		{
			.BufferLocation = constBuffCamera_[i].gpuAddress_,
			.SizeInBytes = static_cast<UINT>(constBuffCamera_[i].constantBufferSize_)
		};
		ctx.GetDevice()->CreateConstantBufferView(&cbvDesc, handle);
	}
	
	// Model
	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(descriptorHeap_->GetCPUDescriptorHandleForHeapStart(), descriptorOffset++, incrementSize);
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc =
		{
			.BufferLocation = scene_->modelConstBuffs_[i].gpuAddress_,
			.SizeInBytes = static_cast<UINT>(scene_->modelConstBuffs_[i].constantBufferSize_)
		};
		ctx.GetDevice()->CreateConstantBufferView(&cbvDesc, handle);
	}

	// Texture
	CD3DX12_CPU_DESCRIPTOR_HANDLE texHandle(descriptorHeap_->GetCPUDescriptorHandleForHeapStart(), descriptorOffset++, incrementSize);
	auto imgSRVDesc = scene_->model_.meshes_[0].image_->GetSRVDescription();
	auto imageResource = scene_->model_.meshes_[0].image_->GetResource();
	ctx.GetDevice()->CreateShaderResourceView(imageResource, &imgSRVDesc, texHandle);

	// Lights
	CD3DX12_CPU_DESCRIPTOR_HANDLE lightHandle(descriptorHeap_->GetCPUDescriptorHandleForHeapStart(), descriptorOffset++, incrementSize);
	auto lightSRVDesc = resourcesLights_->GetSRVDescription();
	ctx.GetDevice()->CreateShaderResourceView(
		resourcesLights_->GetResource(),
		&lightSRVDesc,
		lightHandle);
}

void PipelineSimple::CreateShaders(DX12Context& ctx)
{
	vertexShader_.Create(ctx, AppConfig::ShaderFolder + "BlinnPhong.hlsl", ShaderType::Vertex);
	fragmentShader_.Create(ctx, AppConfig::ShaderFolder + "BlinnPhong.hlsl", ShaderType::Fragment);
}

void PipelineSimple::CreateRootSignature(DX12Context& ctx)
{
	uint32_t cvbRegister = 0;
	uint32_t srvRegister = 0;
	std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges;
	ranges.emplace_back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, cvbRegister++, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges.emplace_back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, cvbRegister++, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges.emplace_back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, srvRegister++, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges.emplace_back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, srvRegister++, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

	uint32_t paramOffset = 0;
	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
	rootParameters.emplace_back().InitAsDescriptorTable(1, ranges.data() + paramOffset++, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters.emplace_back().InitAsDescriptorTable(1, ranges.data() + paramOffset++, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters.emplace_back().InitAsDescriptorTable(1, ranges.data() + paramOffset++, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters.emplace_back().InitAsDescriptorTable(1, ranges.data() + paramOffset++, D3D12_SHADER_VISIBILITY_PIXEL);

	// Image
	D3D12_STATIC_SAMPLER_DESC sampler = scene_->model_.meshes_[0].image_->GetSampler();

	// Allow input layout and deny unnecessary access to certain pipeline stages.
	constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	// Root signature
	descriptor_.CreateRootDescriptor(ctx, sampler, rootParameters, rootSignatureFlags);
}

void PipelineSimple::CreateGraphicsPipeline(DX12Context& ctx)
{
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs = VertexData::GetInputElementDescriptions();

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc =
	{
		.pRootSignature = descriptor_.rootSignature_,
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
	psoDesc.RTVFormats[0] = ctx.GetSwapchainFormat();
	psoDesc.SampleDesc.Count = AppConfig::MSAACount;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(ctx.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_)))
}

void PipelineSimple::Update(DX12Context& ctx)
{
	CCamera cb = {
		.viewMatrix = glm::transpose(camera_->GetViewMatrix()),
		.projectionMatrix = glm::transpose(camera_->GetProjectionMatrix()),
		.cameraPosition = camera_->Position()
	};
	constBuffCamera_[ctx.GetFrameIndex()].UploadData(&cb);
}

void PipelineSimple::PopulateCommandList(DX12Context& ctx)
{
	ID3D12GraphicsCommandList* commandList = ctx.GetCommandList();

	commandList->SetPipelineState(pipelineState_);
	commandList->RSSetViewports(1, &viewport_);
	commandList->RSSetScissorRects(1, &scissor_);
	commandList->SetGraphicsRootSignature(descriptor_.rootSignature_);

	// Descriptors
	uint32_t rootParamIndex = 0;
	// TODO handles can be precomputed
	const uint32_t incrementSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	const CD3DX12_GPU_DESCRIPTOR_HANDLE camHandle(descriptorHeap_->GetGPUDescriptorHandleForHeapStart(), ctx.GetFrameIndex(), incrementSize);
	const CD3DX12_GPU_DESCRIPTOR_HANDLE modelHandle(descriptorHeap_->GetGPUDescriptorHandleForHeapStart(), ctx.GetFrameIndex() + 2, incrementSize);
	const CD3DX12_GPU_DESCRIPTOR_HANDLE texHandle(descriptorHeap_->GetGPUDescriptorHandleForHeapStart(), 4, incrementSize);
	const CD3DX12_GPU_DESCRIPTOR_HANDLE lightHandle(descriptorHeap_->GetGPUDescriptorHandleForHeapStart(), 5, incrementSize);
	
	ID3D12DescriptorHeap* ppHeaps[] = { descriptorHeap_};
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	commandList->SetGraphicsRootDescriptorTable(rootParamIndex++, camHandle);
	commandList->SetGraphicsRootDescriptorTable(rootParamIndex++, modelHandle);
	commandList->SetGraphicsRootDescriptorTable(rootParamIndex++, texHandle);
	commandList->SetGraphicsRootDescriptorTable(rootParamIndex++, lightHandle);

	const auto rtvHandle = resourcesShared_->GetMultiSampledRTVHandle();
	const auto dsvHandle = resourcesShared_->GetDSVHandle();
	constexpr uint32_t renderTargetCount = 1;
	commandList->OMSetRenderTargets(renderTargetCount, &rtvHandle, FALSE, &dsvHandle);

	// TODO Only one mesh for now
	const Mesh& mesh = scene_->model_.meshes_[0];

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &(mesh.vertexBuffer_.vertexBufferView_));
	commandList->IASetIndexBuffer(&mesh.indexBuffer_.indexBufferView_);
	commandList->DrawIndexedInstanced(mesh.vertexCount_, 1, 0, 0, 0);
}