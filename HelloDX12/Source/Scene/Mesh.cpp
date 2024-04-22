#include "Mesh.h"
#include "ShapeGenerator.h"
#include "DX12Exception.h"
#include "PipelineMipmap.h"

Mesh::Mesh(
	DX12Context& ctx,
	const std::string& meshName,
	std::vector<VertexData>&& vertices,
	std::vector<uint32_t>&& indices) :
	meshName_(meshName),
	vertices_(std::move(vertices)),
	indices_(indices)
{
	vertexCount_ = static_cast<uint32_t>(indices_.size());

	CreateBuffers(ctx);

	image_ = std::make_unique<DX12Image>();
	image_->Load(ctx, AppConfig::ModelFolder + "Zaku/Textures/m_14_baseColor.png");

	PipelineMipmap pip(ctx);
	pip.GenerateMipmap(ctx, image_.get());
}

void Mesh::CreateCube(DX12Context& ctx)
{
	ShapeGenerator::Cube(vertices_, indices_);
	vertexCount_ = static_cast<uint32_t>(indices_.size());

	CreateBuffers(ctx);
}

void Mesh::CreateBuffers(DX12Context& ctx)
{
	const uint32_t vertexBufferSize = static_cast<uint32_t>(sizeof(VertexData) * vertices_.size());
	vertexBuffer_.CreateVertexBuffer(ctx, vertices_.data(), vertexBufferSize, sizeof(VertexData));

	const uint32_t indexBufferSize = static_cast<uint32_t>(sizeof(uint32_t) * indices_.size());
	indexBuffer_.CreateIndexBuffer(ctx, indices_.data(), indexBufferSize, DXGI_FORMAT_R32_UINT);
}