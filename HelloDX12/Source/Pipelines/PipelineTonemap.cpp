#include "PipelineTonemap.h"
#include "DX12Exception.h"
#include "VertexData.h"

PipelineTonemap::PipelineTonemap(
	DX12Context& ctx,
	ResourcesShared* resourcesShared) :
	PipelineBase(ctx),
	resourcesShared_(resourcesShared)
{
	CreateDescriptors(ctx);
	CreateShaders(ctx);
	CreateGraphicsPipeline(ctx);
}

PipelineTonemap::~PipelineTonemap()
{
	Destroy();
}

void PipelineTonemap::Destroy()
{
	descriptorHeap_.Destroy();
}

void PipelineTonemap::CreateDescriptors(DX12Context& ctx)
{
	std::vector<DX12Descriptor> descriptors(1);
	descriptors[0] =
	{
		.type_ = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE,
		.shaderVisibility_ = D3D12_SHADER_VISIBILITY_PIXEL,
		.buffer_ = resourcesShared_->GetSingleSampledBuffer(),
		.srvDescription_ = resourcesShared_->GetSingleSampledSRVDescription()
	};

	descriptorHeap_.descriptors_ = descriptors;
	descriptorHeap_.Create(ctx);

	CD3DX12_STATIC_SAMPLER_DESC samplerDesc{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	constexpr D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	rootSignature_.Create(ctx, samplerDesc, descriptors, 0, rootSignatureFlags);
}

void PipelineTonemap::CreateShaders(DX12Context& ctx)
{
	vertexShader_.Create(ctx, AppConfig::ShaderFolder + "Tonemap.hlsl", ShaderType::Vertex);
	fragmentShader_.Create(ctx, AppConfig::ShaderFolder + "Tonemap.hlsl", ShaderType::Fragment);
}

void PipelineTonemap::CreateGraphicsPipeline(DX12Context& ctx)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc =
	{
		.pRootSignature = rootSignature_.rootSignature_,
		.VS = CD3DX12_SHADER_BYTECODE(vertexShader_.GetHandle()),
		.PS = CD3DX12_SHADER_BYTECODE(fragmentShader_.GetHandle()),
		.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		.SampleMask = UINT_MAX,
		.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
		.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		.NumRenderTargets = 1,
	};
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.RTVFormats[0] = ctx.GetSwapchainFormat();
	psoDesc.SampleDesc.Count = 1;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(ctx.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_)))
}

void PipelineTonemap::PopulateCommandList(DX12Context& ctx)
{
	ID3D12GraphicsCommandList* commandList = ctx.GetCommandList();

	commandList->SetPipelineState(pipelineState_);
	commandList->RSSetViewports(1, &viewport_);
	commandList->RSSetScissorRects(1, &scissor_);
	commandList->SetGraphicsRootSignature(rootSignature_.rootSignature_);

	// Descriptors
	const uint32_t incrementSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const CD3DX12_GPU_DESCRIPTOR_HANDLE handle1(descriptorHeap_.descriptorHeap_->GetGPUDescriptorHandleForHeapStart(), 0, incrementSize);

	descriptorHeap_.BindHeap(commandList);
	descriptorHeap_.BindDescriptorsGraphics(commandList, 0);

	const auto rtvHandle = resourcesShared_->GetSwapchainCPUHandle(ctx.GetFrameIndex());
	constexpr uint32_t renderTargetCount = 1;
	commandList->OMSetRenderTargets(renderTargetCount, &rtvHandle, FALSE, nullptr); // No depth attachment

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	constexpr uint32_t triangleCount = 3;
	commandList->DrawIndexedInstanced(triangleCount, 1, 0, 0, 0);
}