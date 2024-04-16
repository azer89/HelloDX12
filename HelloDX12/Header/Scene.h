#ifndef HELLO_DX12_SCENE
#define HELLO_DX12_SCENE

#include "DX12Context.h"
#include "DX12Image.h"
#include "VertexData.h"

class Scene
{
public:
	Scene() = default;
	~Scene() = default;

	void Init(DX12Context& ctx);

public:
	std::unique_ptr<DX12Image> image_;
	uint32_t numVertices_;

	ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;

	ComPtr<ID3D12Resource> indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
};

#endif