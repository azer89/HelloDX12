#include "Mesh.h"
#include "ShapeGenerator.h"
#include "DX12Exception.h"

void Mesh::CreateCube(DX12Context& ctx)
{
	ShapeGenerator::Cube(vertices_, indices_);
	numVertices_ = static_cast<uint32_t>(indices_.size());

	CreateBuffers(ctx);

	image_ = std::make_unique<DX12Image>(ctx);
}

void Mesh::CreateBuffers(DX12Context& ctx)
{
	const uint32_t vertexBufferSize = static_cast<uint32_t>(sizeof(VertexData) * vertices_.size());
	vertexBuffer_.CreateVertexBuffer(ctx, vertices_.data(), vertexBufferSize, sizeof(VertexData));

	const uint32_t indexBufferSize = static_cast<uint32_t>(sizeof(uint32_t) * indices_.size());
	indexBuffer_.CreateIndexBuffer(ctx, indices_.data(), indexBufferSize, DXGI_FORMAT_R32_UINT);
}