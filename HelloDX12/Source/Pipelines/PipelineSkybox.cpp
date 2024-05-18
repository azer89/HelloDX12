#include "PipelineSkybox.h"

#include "ConstantDefinitions.h"

PipelineSkybox::PipelineSkybox(
	DX12Context& ctx,
	ResourcesIBL* resourcesIBL,
	ResourcesShared* resourcesShared,
	Camera* camera) :
	PipelineBase(ctx),
	resourcesIBL_(resourcesIBL),
	resourcesShared_(resourcesShared),
	camera_(camera)
{
	CreateConstantBuffer(ctx);
	CreateDescriptors(ctx);
	GenerateShader(ctx);
	CreatePipeline(ctx);
}

PipelineSkybox::~PipelineSkybox()
{
	Destroy();
	for (auto& heap : descriptorHeaps_)
	{
		heap.Destroy();
	}
}

void PipelineSkybox::Destroy()
{
	for (auto& buff : constBuffCamera_)
	{
		buff.Destroy();
	}
}

void PipelineSkybox::CreateConstantBuffer(DX12Context& ctx)
{
	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		constBuffCamera_[i].CreateConstantBuffer(ctx, sizeof(CCamera));
	}
}

void PipelineSkybox::CreateDescriptors(DX12Context& ctx)
{
	std::vector<DX12Descriptor> descriptors(2);
	descriptors[0] =
	{
		.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
		.shaderVisibility_ = D3D12_SHADER_VISIBILITY_VERTEX,
		.buffer_ = nullptr
	};

	descriptors[1] =
	{
		.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
		.shaderVisibility_ = D3D12_SHADER_VISIBILITY_PIXEL,
		.buffer_ = &(resourcesIBL_->environmentCubemap_.buffer_),
		.srvDescription_ = resourcesIBL_->environmentCubemap_.buffer_.srvDescription_
	};

	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		descriptors[0].buffer_ = &(constBuffCamera_[i]);
		descriptors[0].cbvDescription_ = constBuffCamera_[i].GetCBVDescription();

		descriptorHeaps_[i].descriptors_ = descriptors;
		descriptorHeaps_[i].Create(ctx);
	}

	CD3DX12_STATIC_SAMPLER_DESC sampler{ 0, D3D12_FILTER_ANISOTROPIC };
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// Allow input layout and deny unnecessary access to certain pipeline stages.
	constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	// Root signature
	rootSignature_.Create(ctx, sampler, descriptors, {}, 0, rootSignatureFlags);
}

void PipelineSkybox::GenerateShader(DX12Context& ctx)
{
	vertexShader_.Create(ctx, AppConfig::ShaderFolder + "Skybox.hlsl", ShaderType::Vertex);
	fragmentShader_.Create(ctx, AppConfig::ShaderFolder + "Skybox.hlsl", ShaderType::Fragment);
}

void PipelineSkybox::CreatePipeline(DX12Context& ctx)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc =
	{
		.pRootSignature = rootSignature_.rootSignature_,
		.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		.SampleMask = UINT_MAX,
		.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
		.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT),
		.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		.NumRenderTargets = 1,
		.DSVFormat = DXGI_FORMAT_D32_FLOAT,
	};
	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // Do not write depth
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // Specific to skybox
	psoDesc.RTVFormats[0] = ctx.GetSwapchainFormat();
	psoDesc.SampleDesc.Count = AppConfig::MSAACount;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	vertexShader_.AddShader(psoDesc);
	fragmentShader_.AddShader(psoDesc);

	ThrowIfFailed(ctx.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_)));
}

void PipelineSkybox::Update(DX12Context& ctx)
{
	CCamera cb = {
		.viewMatrix = glm::transpose(glm::mat4(glm::mat3(camera_->GetViewMatrix()))),
		.projectionMatrix = glm::transpose(camera_->GetProjectionMatrix()),
		.cameraPosition = camera_->Position()
	};
	constBuffCamera_[ctx.GetFrameIndex()].UploadData(&cb);
}

void PipelineSkybox::PopulateCommandList(DX12Context& ctx)
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

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	constexpr uint32_t triangleCount = 36;
	commandList->DrawInstanced(triangleCount, 1, 0, 0);
}