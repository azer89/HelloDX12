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
	CreateShaders(ctx);
	CreateDescriptors(ctx);
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
	for (auto& heap : descriptorHeaps_)
	{
		heap.Destroy();
	}
}

void PipelineSimple::CreateConstantBuffer(DX12Context& ctx)
{
	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		constBuffCamera_[i].CreateConstantBuffer(ctx, sizeof(CCamera));
	}
}

void PipelineSimple::CreateDescriptors(DX12Context& ctx)
{
	std::vector<DX12Descriptor> descriptors(4);
	descriptors[0] =
	{
		.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
		.shaderVisibility_ = D3D12_SHADER_VISIBILITY_ALL,
		.buffer_ = nullptr
	};

	descriptors[1] =
	{

		.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
		.shaderVisibility_ = D3D12_SHADER_VISIBILITY_VERTEX,
		.buffer_ = nullptr
	};

	descriptors[2] =
	{
		.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
		.shaderVisibility_ = D3D12_SHADER_VISIBILITY_PIXEL,
		.buffer_ = &(scene_->model_.meshes_[0].image_->buffer_),
		.srvDescription_ = scene_->model_.meshes_[0].image_->buffer_.srvDesccription_
	};

	descriptors[3] =
	{
		.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
		.shaderVisibility_ = D3D12_SHADER_VISIBILITY_PIXEL,
		.buffer_ = &(resourcesLights_->buffer_),
		.srvDescription_ = resourcesLights_->buffer_.srvDesccription_
	};

	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		descriptors[0].buffer_ = &(constBuffCamera_[i]);
		descriptors[0].cbvDescription_ = constBuffCamera_[i].GetCBVDescription();

		descriptors[1].buffer_ = &(scene_->modelConstBuffs_[i]);
		descriptors[1].cbvDescription_ = scene_->modelConstBuffs_[i].GetCBVDescription();

		descriptorHeaps_[i].descriptors_ = descriptors;
		descriptorHeaps_[i].Create(ctx);
	}

	D3D12_STATIC_SAMPLER_DESC sampler = scene_->model_.meshes_[0].image_->GetSampler();
	constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	rootSignature_.Create(ctx, sampler, descriptors, 0, rootSignatureFlags);
}

void PipelineSimple::CreateShaders(DX12Context& ctx)
{
	vertexShader_.Create(ctx, AppConfig::ShaderFolder + "BlinnPhong.hlsl", ShaderType::Vertex);
	fragmentShader_.Create(ctx, AppConfig::ShaderFolder + "BlinnPhong.hlsl", ShaderType::Fragment);
}

void PipelineSimple::CreateGraphicsPipeline(DX12Context& ctx)
{
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs = VertexData::GetInputElementDescriptions();

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc =
	{
		.pRootSignature = rootSignature_.rootSignature_,
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

	psoDesc.VS.BytecodeLength = vertexShader_.GetHandle()->GetBufferSize();
	psoDesc.VS.pShaderBytecode = vertexShader_.GetHandle()->GetBufferPointer();
	psoDesc.PS.BytecodeLength = fragmentShader_.GetHandle()->GetBufferSize();
	psoDesc.PS.pShaderBytecode = fragmentShader_.GetHandle()->GetBufferPointer();

	ThrowIfFailed(ctx.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_)))
}

void PipelineSimple::Update(DX12Context& ctx)
{
	// TODO find a way without transpose
	CCamera* ptr = constBuffCamera_[ctx.GetFrameIndex()].As<CCamera>();
	ptr->viewMatrix = glm::transpose(camera_->GetViewMatrix());
	ptr->projectionMatrix = glm::transpose(camera_->GetProjectionMatrix());
	ptr->cameraPosition = camera_->Position();
}

void PipelineSimple::PopulateCommandList(DX12Context& ctx)
{
	ID3D12GraphicsCommandList* commandList = ctx.GetCommandList();

	commandList->SetPipelineState(pipelineState_);
	commandList->RSSetViewports(1, &viewport_);
	commandList->RSSetScissorRects(1, &scissor_);
	commandList->SetGraphicsRootSignature(rootSignature_.rootSignature_);

	// Descriptors
	descriptorHeaps_[ctx.GetFrameIndex()].BindHeap(commandList);
	descriptorHeaps_[ctx.GetFrameIndex()].BindDescriptorsGraphics(commandList, 0);

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