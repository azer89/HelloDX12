#ifndef HELLO_SCENE_PODS
#define HELLO_SCENE_PODS

#include <cstdint>
#include <vector>

#include "VertexData.h"

struct ModelMatrix
{
	glm::mat4 modelMatrix;
};

struct MeshData
{
	uint32_t vertexOffset_{};
	uint32_t indexOffset_{};

	uint32_t modelMatrixIndex_{};

	// PBR Texture IDs
	uint32_t albedo_{};
	uint32_t normal_{};
	uint32_t metalness_{};
	uint32_t roughness_{};
	uint32_t ao_{};
	uint32_t emissive_{};
};

struct SceneData
{
	std::vector<VertexData> vertices_{};
	std::vector<uint32_t> indices_{};
	std::vector<uint32_t> vertexOffsets_{};
	std::vector<uint32_t> indexOffsets_{};

	uint32_t GetCurrentVertexOffset() const
	{
		return vertexOffsets_.empty() ? 0u : vertexOffsets_.back();
	}

	uint32_t GetCurrentIndexOffset() const
	{
		return indexOffsets_.empty() ? 0u : indexOffsets_.back();
	}
};

#endif