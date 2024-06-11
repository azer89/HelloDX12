#ifndef HELLO_DX12_PIPELINE_MIPMAP
#define HELLO_DX12_PIPELINE_MIPMAP

#include "DX12Context.h"
#include "DX12Image.h"
#include "PipelineBase.h"

class PipelineMipmap final : public PipelineBase
{
public:
	PipelineMipmap(
		DX12Context& ctx,
		bool textureArray);
	~PipelineMipmap() = default;

	void GenerateMipmap(DX12Context& ctx, DX12Image* image);

	void Update(DX12Context& ctx, UIData& uiData) override {}
	void PopulateCommandList(DX12Context& ctx) override {};

private:
	void GenerateShader(DX12Context& ctx);
	void CreatePipeline(DX12Context& ctx);

private:
	bool textureArray_ = false;
};

#endif