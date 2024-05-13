#include "Mesh.h"
#include "ShapeGenerator.h"
#include "DX12Exception.h"
#include "PipelineMipmap.h"

Mesh::Mesh(
	DX12Context& ctx,
	const std::string& meshName,
	const uint32_t vertexOffset,
	const uint32_t indexOffset,
	const uint32_t vertexCount,
	const uint32_t indexCount,
	std::unordered_map<TextureType, uint32_t>&& textureIndices) :
	vertexOffset_(vertexOffset),
	indexOffset_(indexOffset),
	vertexCount_(vertexCount),
	indexCount_(indexCount),
	meshName_(meshName),
	textureIndices_(std::move(textureIndices))
{
	/*image_ = std::make_unique<DX12Image>();
	auto data = DX12Image::GenerateCheckerboard(100, 100, 4);
	image_->Load(ctx, data.data(), 100, 100);
	PipelineMipmap pip(ctx);
	pip.GenerateMipmap(ctx, image_.get());*/
}

void Mesh::Destroy()
{
}

void Mesh::CreateCube(DX12Context& ctx)
{
	/*ShapeGenerator::Cube(vertices_, indices_);
	vertexCount_ = static_cast<uint32_t>(indices_.size());
	CreateBuffers(ctx);*/
}