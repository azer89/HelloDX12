#ifndef HELLO_SCENE_PODS
#define HELLO_SCENE_PODS

#include <cstdint>

struct ModelMatrix
{
	glm::mat4 modelMatrix;
};

struct MeshData
{
	uint32_t vertexOffset_ = 0;
	uint32_t indexOffset_ = 0;

	uint32_t modelMatrixIndex_ = 0;

	// PBR Texture IDs
	uint32_t albedo_ = 0;
	uint32_t normal_ = 0;
	uint32_t metalness_ = 0;
	uint32_t roughness_ = 0;
	uint32_t ao_ = 0;
	uint32_t emissive_ = 0;
};

struct SceneData
{
	std::vector<VertexData> vertices_ = {};
	std::vector<uint32_t> indices_ = {};
	std::vector<uint32_t> vertexOffsets_ = {};
	std::vector<uint32_t> indexOffsets_ = {};

	uint32_t GetCurrentVertexOffset() const
	{
		if (vertexOffsets_.empty()) { return 0u; }
		return vertexOffsets_.back();
	}

	uint32_t GetCurrentIndexOffset() const
	{
		if (indexOffsets_.empty()) { return 0u; }
		return indexOffsets_.back();
	}
};

#endif