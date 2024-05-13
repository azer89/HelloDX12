#ifndef HELLO_DX12_MESH
#define HELLO_DX12_MESH

#include "DX12Context.h"
#include "DX12Image.h"
#include "DX12Buffer.h"
#include "TextureMapper.h"
#include "VertexData.h"
#include "ScenePODs.h"

#include <vector>
#include <memory>

class Mesh
{
public:
	Mesh(
		DX12Context& ctx,
		const std::string& meshName,
		const uint32_t vertexOffset,
		const uint32_t indexOffset,
		const uint32_t vertexCount,
		const uint32_t indexCount,
		std::unordered_map<TextureType, uint32_t>&& textureIndices);

	void Destroy();

	void CreateCube(DX12Context& ctx);

	[[nodiscard]] MeshData GetMeshData(uint32_t textureIndexOffset, uint32_t modelMatrixIndex)
	{
		return
		{
			.vertexOffset_ = vertexOffset_,
			.indexOffset_ = indexOffset_,
			.modelMatrixIndex_ = modelMatrixIndex,
			.albedo_ = textureIndices_[TextureType::Albedo] + textureIndexOffset,
			.normal_ = textureIndices_[TextureType::Normal] + textureIndexOffset,
			.metalness_ = textureIndices_[TextureType::Metalness] + textureIndexOffset,
			.roughness_ = textureIndices_[TextureType::Roughness] + textureIndexOffset,
			.ao_ = textureIndices_[TextureType::AmbientOcclusion] + textureIndexOffset,
			.emissive_ = textureIndices_[TextureType::Emissive] + textureIndexOffset,
		};
	}

public:
	std::string meshName_ = {};

	uint32_t vertexCount_ = 0;
	uint32_t indexCount_ = 0;
	uint32_t vertexOffset_ = 0;
	uint32_t indexOffset_ = 0;

	std::unordered_map<TextureType, uint32_t> textureIndices_;
};

#endif