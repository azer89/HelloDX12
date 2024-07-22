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
	meshName_(meshName),
	vertexCount_(vertexCount),
	indexCount_(indexCount),
	vertexOffset_(vertexOffset),
	indexOffset_(indexOffset),
	textureIndices_(std::move(textureIndices))
{
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