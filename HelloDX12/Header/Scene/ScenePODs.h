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

#endif