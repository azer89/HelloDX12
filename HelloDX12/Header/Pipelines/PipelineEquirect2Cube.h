#ifndef HELLO_DX12_PIPELINE_EQUIRECT_TO_CUBE
#define HELLO_DX12_PIPELINE_EQUIRECT_TO_CUBE

#include "DX12Context.h"
#include "PipelineBase.h"
#include "DX12Image.h"

class PipelineEquirect2Cube final : public PipelineBase
{
public:
	PipelineEquirect2Cube(
		DX12Context& ctx);
	~PipelineEquirect2Cube() = default;

	void GenerateCubemapFromHDR(
		DX12Context& ctx, 
		DX12Image* hdrImage, 
		DX12Image* cubemapImage,
		const D3D12_UNORDERED_ACCESS_VIEW_DESC& cubemapUAVDesc);

	void Update(DX12Context& ctx) override {};
	void PopulateCommandList(DX12Context& ctx) override {};

private:
	void GenerateShader(DX12Context& ctx);
	void CreateRootSignature(DX12Context& ctx);
	void CreateDescriptorHeap(
		DX12Context& ctx,
		DX12Image* hdrImage,
		DX12Image* cubemapImage,
		const D3D12_UNORDERED_ACCESS_VIEW_DESC& cubemapUAVDesc);
	void CreatePipeline(DX12Context& ctx);
	void Execute(DX12Context& ctx);
};

#endif