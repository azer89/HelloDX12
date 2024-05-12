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
	std::vector<VertexData>&& vertices,
	std::vector<uint32_t>&& indices,
	std::unordered_map<TextureType, uint32_t>&& textureIndices) :
	vertexOffset_(vertexOffset),
	indexOffset_(indexOffset),
	vertexCount_(vertexCount),
	indexCount_(indexCount),
	meshName_(meshName),
	vertices_(std::move(vertices)),
	indices_(std::move(indices)),
	textureIndices_(std::move(textureIndices))
{
	//vertexCount_ = static_cast<uint32_t>(indices_.size());

	CreateBuffers(ctx);

	image_ = std::make_unique<DX12Image>();
	//image_->Load(ctx, AppConfig::ModelFolder + "Zaku/Textures/m_14_baseColor.png");
	auto data = DX12Image::GenerateCheckerboard(100, 100, 4);
	image_->Load(ctx, data.data(), 100, 100);


	PipelineMipmap pip(ctx);
	pip.GenerateMipmap(ctx, image_.get());
}

void Mesh::Destroy()
{
	vertexBuffer_.Destroy();
	indexBuffer_.Destroy();

	image_->Destroy();
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
	const uint32_t indexBufferSize = static_cast<uint32_t>(sizeof(uint32_t) * indices_.size());

	vertexBuffer_.CreateDeviceOnlyBuffer(
		ctx,
		vertices_.data(), 
		vertices_.size(), 
		vertexBufferSize, 
		sizeof(VertexData));

	indexBuffer_.CreateDeviceOnlyBuffer(
		ctx,
		indices_.data(),
		indices_.size(),
		indexBufferSize,
		sizeof(uint32_t));
}