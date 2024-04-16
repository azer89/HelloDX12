#ifndef HELLO_DX12_VERTEX_DATA
#define HELLO_DX12_VERTEX_DATA

#include "glm/glm.hpp"

struct VertexData
{
	glm::vec3 position_;
	glm::vec3 normal_;
	glm::vec2 uv_;

	VertexData()
	{
	}

	VertexData(
		glm::vec3 position,
		glm::vec3 normal,
		glm::vec2 uv) :
		position_(position),
		normal_(normal),
		uv_(uv)
	{
	}

	VertexData(
		float px, float py, float pz,
		float nx, float ny, float nz,
		float u, float v) :
		position_(glm::vec3(px, py, pz)),
		normal_(glm::vec3(nx, ny, nz)),
		uv_(glm::vec2(u, v))
	{
	}
};

#endif