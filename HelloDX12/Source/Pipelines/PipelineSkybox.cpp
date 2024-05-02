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
	CreateDescriptorHeap(ctx);
	GenerateShader(ctx);
	CreateRootSignature(ctx);
	CreatePipeline(ctx);
}

PipelineSkybox::~PipelineSkybox()
{
	Destroy();
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

void PipelineSkybox::CreateDescriptorHeap(DX12Context& ctx)
{
	descriptorManager_.CreateDescriptorHeap(ctx, 3);

	const uint32_t incrementSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	uint32_t descriptorOffset = 0;

	// Camera (CVB)
	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(descriptorManager_.descriptorHeap_->GetCPUDescriptorHandleForHeapStart(), descriptorOffset++, incrementSize);
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc =
		{
			.BufferLocation = constBuffCamera_[i].gpuAddress_,
			.SizeInBytes = static_cast<UINT>(constBuffCamera_[i].constantBufferSize_)
		};
		ctx.GetDevice()->CreateConstantBufferView(&cbvDesc, handle);
	}

	// Cubemap (SRV)
	auto imgSRVDesc = resourcesIBL_->environmentCubemap_.GetSRVDescription();
	auto imageResource = resourcesIBL_->environmentCubemap_.GetResource();
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(descriptorManager_.descriptorHeap_->GetCPUDescriptorHandleForHeapStart(), descriptorOffset++, incrementSize);
	ctx.GetDevice()->CreateShaderResourceView(imageResource, &imgSRVDesc, handle);
}

void PipelineSkybox::GenerateShader(DX12Context& ctx)
{
	vertexShader_.Create(ctx, AppConfig::ShaderFolder + "Skybox.hlsl", ShaderType::Vertex);
	fragmentShader_.Create(ctx, AppConfig::ShaderFolder + "Skybox.hlsl", ShaderType::Fragment);
}

void PipelineSkybox::CreateRootSignature(DX12Context& ctx)
{
	uint32_t cvbRegister = 0;
	uint32_t srvRegister = 0;
	std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges;
	ranges.emplace_back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, cvbRegister++, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges.emplace_back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, srvRegister++, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

	uint32_t paramOffset = 0;
	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
	rootParameters.emplace_back().InitAsDescriptorTable(1, ranges.data() + paramOffset++, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters.emplace_back().InitAsDescriptorTable(1, ranges.data() + paramOffset++, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_STATIC_SAMPLER_DESC sampler{ 0, D3D12_FILTER_ANISOTROPIC };
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// Allow input layout and deny unnecessary access to certain pipeline stages.
	constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	// Root signature
	descriptorManager_.CreateRootDescriptor(ctx, sampler, rootParameters, rootSignatureFlags);
}

void PipelineSkybox::CreatePipeline(DX12Context& ctx)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc =
	{
		.pRootSignature = descriptorManager_.rootSignature_,
		.VS = CD3DX12_SHADER_BYTECODE(vertexShader_.GetHandle()),
		.PS = CD3DX12_SHADER_BYTECODE(fragmentShader_.GetHandle()),
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
	ThrowIfFailed(ctx.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_)))
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
	commandList->SetGraphicsRootSignature(descriptorManager_.rootSignature_);

	// Descriptors
	uint32_t rootParamIndex = 0;
	const uint32_t incrementSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const CD3DX12_GPU_DESCRIPTOR_HANDLE handle1(descriptorManager_.descriptorHeap_->GetGPUDescriptorHandleForHeapStart(), ctx.GetFrameIndex(), incrementSize);
	const CD3DX12_GPU_DESCRIPTOR_HANDLE handle2(descriptorManager_.descriptorHeap_->GetGPUDescriptorHandleForHeapStart(), 2, incrementSize);
	ID3D12DescriptorHeap* ppHeaps[] = { descriptorManager_.descriptorHeap_ };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	commandList->SetGraphicsRootDescriptorTable(rootParamIndex++, handle1);
	commandList->SetGraphicsRootDescriptorTable(rootParamIndex++, handle2);

	const auto rtvHandle = resourcesShared_->GetMultiSampledRTVHandle();
	const auto dsvHandle = resourcesShared_->GetDSVHandle();
	constexpr uint32_t renderTargetCount = 1;
	commandList->OMSetRenderTargets(renderTargetCount, &rtvHandle, FALSE, &dsvHandle);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	constexpr uint32_t triangleCount = 36;
	commandList->DrawInstanced(triangleCount, 1, 0, 0);
}