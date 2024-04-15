#ifndef HELLO_DX12_SCENE
#define HELLO_DX12_SCENE

#include "DX12Context.h"
#include "DX12Image.h"

#include "glm/glm.hpp"

struct VertexData
{
	glm::vec3 position;
	glm::vec2 uv;
};

class Scene
{
public:
	Scene() = default;
	~Scene() = default;

	void Init(DX12Context& ctx);

public:
	std::unique_ptr<DX12Image> image_;
	ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
};

#endif