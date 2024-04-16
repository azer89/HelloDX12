#ifndef HELLO_DX12_VERTEX_DATA
#define HELLO_DX12_VERTEX_DATA

#include "glm/glm.hpp"

#include <vector>

struct VertexData
{
	glm::vec3 position_;
	glm::vec3 normal_;
	glm::vec2 uv_;

	VertexData() = default;

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

	static std::vector<D3D12_INPUT_ELEMENT_DESC> GetInputElementDescriptions()
	{
		return
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
	}
};

#endif