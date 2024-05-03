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
	~PipelineEquirect2Cube();

	void Destroy();

	void GenerateCubemapFromHDR(
		DX12Context& ctx, 
		DX12Image* hdrImage, 
		DX12Image* cubemapImage,
		const D3D12_UNORDERED_ACCESS_VIEW_DESC& cubemapUAVDesc);

	void Update(DX12Context& ctx) override {};
	void PopulateCommandList(DX12Context& ctx) override {};

private:
	void CreateDescriptors(DX12Context& ctx, 
		DX12Image* hdrImage,
		DX12Image* cubemapImage,
		const D3D12_UNORDERED_ACCESS_VIEW_DESC& cubemapUAVDesc);
	void GenerateShader(DX12Context& ctx);
	void CreatePipeline(DX12Context& ctx);
	void Execute(DX12Context& ctx,
		DX12Image* hdrImage,
		DX12Image* cubemapImage);

private:
	DX12DescriptorHeap descriptorHeap2_ = {}; // TODO Rename
};

#endif