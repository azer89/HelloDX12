#include "PipelineTonemap.h"
#include "DX12Exception.h"
#include <VertexData.h>

PipelineTonemap::PipelineTonemap(
	DX12Context& ctx,
	ResourcesShared* resourcesShared) :
	PipelineBase(ctx),
	resourcesShared_(resourcesShared)
{
	CreateDescriptorHeap(ctx);
	CreateRootSignature(ctx);
	CreateShaders(ctx);
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
	auto srcSRVDesc = resourcesShared_->GetOffscreenSRVDescription();
	auto srcResource = resourcesShared_->GetOffscreenResource();
	ctx.GetDevice()->CreateShaderResourceView(srcResource, &srcSRVDesc, handle1);
}

void PipelineTonemap::CreateRootSignature(DX12Context& ctx)
{
	CD3DX12_DESCRIPTOR_RANGE srvCbvRanges[1] = {};
	srvCbvRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[1] = {};
	rootParameters[0].InitAsDescriptorTable(1, &srvCbvRanges[0]);

	D3D12_STATIC_SAMPLER_DESC samplerDesc =
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

	// Create the root signature for the mipmap compute shader from the parameters and sampler above
	ID3DBlob* signature;
	ID3DBlob* error;
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	ctx.GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
}

void PipelineTonemap::CreateGraphicsPipeline(DX12Context& ctx)
{
	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc =
	{
		.pRootSignature = rootSignature_,
		.VS = CD3DX12_SHADER_BYTECODE(vertexShader_.GetHandle()),
		.PS = CD3DX12_SHADER_BYTECODE(fragmentShader_.GetHandle()),
		.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		.SampleMask = UINT_MAX,
		.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
		.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT),
		//.InputLayout = { inputElementDescs.data(), static_cast<uint32_t>(inputElementDescs.size()) },
		.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		.NumRenderTargets = 1,
		.DSVFormat = DXGI_FORMAT_D32_FLOAT,
	};
	psoDesc.RTVFormats[0] = ctx.GetSwapchainFormat();
	psoDesc.SampleDesc.Count = 1;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(ctx.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_)))
}

void PipelineTonemap::CreateShaders(DX12Context& ctx)
{
	vertexShader_.Create(ctx, AppConfig::ShaderFolder + "Tonemap.hlsl", ShaderType::Vertex);
	fragmentShader_.Create(ctx, AppConfig::ShaderFolder + "Tonemap.hlsl", ShaderType::Fragment);
}

void PipelineTonemap::PopulateCommandList(DX12Context& ctx)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc =
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = 2,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};
	ID3D12DescriptorHeap* descriptorHeap;
	ctx.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));

	ID3D12GraphicsCommandList* commandList = ctx.GetCommandList();

	const auto resourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(
			resourcesShared_->GetOffscreenRenderTarget(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &resourceBarrier);

	commandList->SetPipelineState(pipelineState_);
	commandList->RSSetViewports(1, &viewport_);
	commandList->RSSetScissorRects(1, &scissor_);
	commandList->SetGraphicsRootSignature(rootSignature_);

	// Descriptors
	const uint32_t incrementSize = ctx.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const CD3DX12_GPU_DESCRIPTOR_HANDLE handle1(descriptorHeap_->GetGPUDescriptorHandleForHeapStart(), 0, incrementSize);

	ID3D12DescriptorHeap* ppHeaps[] = { descriptorHeap_ };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	commandList->SetGraphicsRootDescriptorTable(0, handle1);

	const auto rtvHandle = resourcesShared_->GetSwapchainRTVHandle(ctx.GetFrameIndex());
	const auto dsvHandle = resourcesShared_->GetDSVHandle();
	constexpr uint32_t renderTargetCount = 1;
	commandList->OMSetRenderTargets(renderTargetCount, &rtvHandle, FALSE, &dsvHandle);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawIndexedInstanced(3, 1, 0, 0, 0);
}