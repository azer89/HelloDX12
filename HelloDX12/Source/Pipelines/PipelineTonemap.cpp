#include "PipelineTonemap.h"
#include "DX12Exception.h"
#include "VertexData.h"

PipelineTonemap::PipelineTonemap(
	DX12Context& ctx,
	ResourcesShared* resourcesShared) :
	PipelineBase(ctx),
	resourcesShared_(resourcesShared)
{
	CreateDescriptorHeap(ctx);
	CreateShaders(ctx);
	CreateRootSignature(ctx);
	CreateGraphicsPipeline(ctx);
}

void PipelineTonemap::CreateDescriptorHeap(DX12Context& ctx)
{
	constexpr uint32_t descriptorCount = 1;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = descriptorCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	};
	ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap_)))

	// TODO handles can be precomputed
	uint32_t incrementSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle1(descriptorHeap_->GetCPUDescriptorHandleForHeapStart(), 0, incrementSize); 

	// Source image SRV
	auto srcSRVDesc = resourcesShared_->GetSingleSampledSRVDescription();
	auto srcResource = resourcesShared_->GetSingleSampledRenderTarget();
	ctx.GetDevice()->CreateShaderResourceView(srcResource, &srcSRVDesc, handle1);
}

void PipelineTonemap::CreateShaders(DX12Context& ctx)
{
	vertexShader_.Create(ctx, AppConfig::ShaderFolder + "Tonemap.hlsl", ShaderType::Vertex);
	fragmentShader_.Create(ctx, AppConfig::ShaderFolder + "Tonemap.hlsl", ShaderType::Fragment);
}

void PipelineTonemap::CreateRootSignature(DX12Context& ctx)
{
	std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges = {};
	ranges.emplace_back().Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters = {};
	rootParameters.emplace_back().InitAsDescriptorTable(1, ranges.data(), D3D12_SHADER_VISIBILITY_PIXEL);

	constexpr D3D12_STATIC_SAMPLER_DESC samplerDesc =
	{
		.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		.MipLODBias = 0.0f,
		.MaxAnisotropy = 0,
		.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
		.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK,
		.MinLOD = 0.0f,
		.MaxLOD = D3D12_FLOAT32_MAX,
		.ShaderRegister = 0,
		.RegisterSpace = 0,
		.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
	};

	// Root signature
	descriptor_.CreateRootDescriptor(
		ctx, 
		samplerDesc, 
		rootParameters, 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
}

void PipelineTonemap::CreateGraphicsPipeline(DX12Context& ctx)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc =
	{
		.pRootSignature = descriptor_.rootSignature_,
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
	commandList->SetGraphicsRootSignature(descriptor_.rootSignature_);

	// Descriptors
	const uint32_t incrementSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const CD3DX12_GPU_DESCRIPTOR_HANDLE handle1(descriptorHeap_->GetGPUDescriptorHandleForHeapStart(), 0, incrementSize);

	ID3D12DescriptorHeap* ppHeaps[] = { descriptorHeap_ };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	commandList->SetGraphicsRootDescriptorTable(0, handle1);

	const auto rtvHandle = resourcesShared_->GetSwapchainRTVHandle(ctx.GetFrameIndex());
	constexpr uint32_t renderTargetCount = 1;
	commandList->OMSetRenderTargets(renderTargetCount, &rtvHandle, FALSE, nullptr); // No depth attachment

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	constexpr uint32_t triangleCount = 3;
	commandList->DrawIndexedInstanced(triangleCount, 1, 0, 0, 0);
}