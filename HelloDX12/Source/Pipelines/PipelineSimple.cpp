#include "PipelineSimple.h"
#include "DX12Exception.h"

#include "ConstantBufferStructs.h"

constexpr uint32_t ROOT_CONSTANT_COUNT = 1;

PipelineSimple::PipelineSimple(
	DX12Context& ctx, 
	Scene* scene, 
	Camera* camera,
	ResourcesShared* resourcesShared,
	ResourcesIBL* resourcesIBL,
	ResourcesLights* resourcesLights) :
	PipelineBase(ctx),
	scene_(scene),
	camera_(camera),
	resourcesShared_(resourcesShared),
	resourcesIBL_(resourcesIBL),
	resourcesLights_(resourcesLights)
{
	CreateIndirectCommand(ctx);
	CreateConstantBuffers(ctx);
	CreateShaders(ctx);
	CreateDescriptors(ctx);
	CreateCommandSignature(ctx);
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

	for (auto& buff : constBuffPBR_)
	{
		buff.Destroy();
	}

	for (auto& heap : descriptorHeaps_)
	{
		heap.Destroy();
	}
}

void PipelineSimple::CreateIndirectCommand(DX12Context& ctx)
{
	const uint32_t meshCount = scene_->GetMeshCount();
	std::vector<IndirectCommand> commandArray(meshCount);
	for (uint32_t i = 0; i < meshCount; ++i)
	{
		commandArray[i] =
		{
			.meshIndex = i,
			.drawArguments =
			{
				.VertexCountPerInstance = scene_->model_.meshes_[i].indexCount_,
				.InstanceCount = 1,
				.StartVertexLocation = 0,
				.StartInstanceLocation = 0
			}
		};
	}

	CreateIndirectCommandFromArray(ctx, commandArray);
}

void PipelineSimple::CreateConstantBuffers(DX12Context& ctx)
{
	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		constBuffCamera_[i].CreateConstantBuffer(ctx, sizeof(CCamera));
	}

	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		constBuffPBR_[i].CreateConstantBuffer(ctx, sizeof(CPBR));
	}
}

void PipelineSimple::CreateDescriptors(DX12Context& ctx)
{
	std::vector<DX12Descriptor> descriptors =
	{
		// b0 is root constants

		{ // b1
			.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
			.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
			.shaderVisibility_ = D3D12_SHADER_VISIBILITY_ALL,
			.buffer_ = nullptr
		},
		{ // b2

			.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
			.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
			.shaderVisibility_ = D3D12_SHADER_VISIBILITY_VERTEX,
			.buffer_ = nullptr
		},
		{ // b3

			.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
			.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
			.shaderVisibility_ = D3D12_SHADER_VISIBILITY_PIXEL,
			.buffer_ = nullptr
		},
		{ // t0
			.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
			.shaderVisibility_ = D3D12_SHADER_VISIBILITY_ALL,
			.buffer_ = &(scene_->vertexBuffer_),
			.srvDescription_ = scene_->vertexBuffer_.GetSRVDescription()
		},
		{ // t1
			.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
			.shaderVisibility_ = D3D12_SHADER_VISIBILITY_ALL,
			.buffer_ = &(scene_->indexBuffer_),
			.srvDescription_ = scene_->indexBuffer_.GetSRVDescription()
		},
		{ // t2
			.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
			.shaderVisibility_ = D3D12_SHADER_VISIBILITY_ALL,
			.buffer_ = &(scene_->meshDataBuffer_),
			.srvDescription_ = scene_->meshDataBuffer_.GetSRVDescription()
		},
		{ // t3
			.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
			.shaderVisibility_ = D3D12_SHADER_VISIBILITY_PIXEL,
			.buffer_ = &(resourcesLights_->buffer_),
			.srvDescription_ = resourcesLights_->buffer_.GetSRVDescription()
		},
		{ // t4
			.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
			.shaderVisibility_ = D3D12_SHADER_VISIBILITY_PIXEL,
			.buffer_ = &(resourcesIBL_->brdfLutImage_.buffer_),
			.srvDescription_ = resourcesIBL_->brdfLutImage_.buffer_.GetSRVDescription()
		},
	};

	// t4
	DX12DescriptorArray descriptorArray =  scene_->GetImageDescriptors(); 

	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		descriptors[0].buffer_ = &(constBuffCamera_[i]);
		descriptors[0].cbvDescription_ = constBuffCamera_[i].GetCBVDescription();

		descriptors[1].buffer_ = &(scene_->modelConstBuffs_[i]);
		descriptors[1].cbvDescription_ = scene_->modelConstBuffs_[i].GetCBVDescription();

		descriptors[2].buffer_ = &(constBuffPBR_[i]);
		descriptors[2].cbvDescription_ = constBuffPBR_[i].GetCBVDescription();

		descriptorHeaps_[i].descriptors_ = descriptors;
		descriptorHeaps_[i].descriptorArray_ = descriptorArray;
		descriptorHeaps_[i].Create(ctx);
	}

	D3D12_STATIC_SAMPLER_DESC sampler = DX12Image::GetDefaultSampler();
	
	constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	rootSignature_.Create(ctx, sampler, descriptors, descriptorArray, ROOT_CONSTANT_COUNT, rootSignatureFlags);
}

void PipelineSimple::CreateShaders(DX12Context& ctx)
{
	vertexShader_.Create(ctx, AppConfig::ShaderFolder + "PBR/PBR.hlsl", ShaderType::Vertex);
	fragmentShader_.Create(ctx, AppConfig::ShaderFolder + "PBR/PBR.hlsl", ShaderType::Fragment);
}

void PipelineSimple::CreateGraphicsPipeline(DX12Context& ctx)
{
	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc =
	{
		.pRootSignature = rootSignature_.handle_,
		.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		.SampleMask = UINT_MAX,
		.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
		.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT),
		.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		.NumRenderTargets = 1,
		.DSVFormat = DXGI_FORMAT_D32_FLOAT,
	};
	psoDesc.RTVFormats[0] = ctx.GetSwapchainFormat();
	psoDesc.SampleDesc.Count = AppConfig::MSAACount;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	// Set shader
	vertexShader_.AddShader(psoDesc);
	fragmentShader_.AddShader(psoDesc);

	ThrowIfFailed(ctx.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_)));
}

void PipelineSimple::Update(DX12Context& ctx, UIData& uiData)
{
	uint32_t frameIndex = ctx.GetFrameIndex();

	// TODO find a way without transpose
	CCamera* pCamera = constBuffCamera_[frameIndex].As<CCamera>();
	pCamera->viewMatrix = glm::transpose(camera_->GetViewMatrix());
	pCamera->projectionMatrix = glm::transpose(camera_->GetProjectionMatrix());
	pCamera->cameraPosition = camera_->Position();

	CPBR* pPBR = constBuffPBR_[frameIndex].As<CPBR>();
	pPBR->albedoMultipler = uiData.constBufferPBR_.albedoMultipler;
	pPBR->baseReflectivity = uiData.constBufferPBR_.baseReflectivity;
	pPBR->lightFalloff = uiData.constBufferPBR_.lightFalloff;
	pPBR->lightIntensity = uiData.constBufferPBR_.lightIntensity;
}

void PipelineSimple::PopulateCommandList(DX12Context& ctx)
{
	ID3D12GraphicsCommandList* commandList = ctx.GetCommandList();

	commandList->SetPipelineState(pipelineState_);
	commandList->RSSetViewports(1, &viewport_);
	commandList->RSSetScissorRects(1, &scissor_);
	commandList->SetGraphicsRootSignature(rootSignature_.handle_);

	// Descriptors
	descriptorHeaps_[ctx.GetFrameIndex()].BindHeap(commandList);
	descriptorHeaps_[ctx.GetFrameIndex()].BindDescriptorsGraphics(commandList, ROOT_CONSTANT_COUNT);

	const auto rtvHandle = resourcesShared_->GetMultiSampledRTVHandle();
	const auto dsvHandle = resourcesShared_->GetDSVHandle();
	constexpr uint32_t renderTargetCount = 1;
	commandList->OMSetRenderTargets(renderTargetCount, &rtvHandle, FALSE, &dsvHandle);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	const uint32_t meshCount = scene_->GetMeshCount();
	commandList->ExecuteIndirect(
		commandSignature_, // pCommandSignature
		meshCount, // MaxCommandCount
		indirectCommand_.GetResource(), // pArgumentBuffer
		0, // ArgumentBufferOffset
		nullptr, // pCountBuffer
		0); // CountBufferOffset
}