#ifndef HELLO_DX12_MESH
#define HELLO_DX12_MESH

#include "DX12Context.h"
#include "DX12Image.h"
#include "VertexData.h"

#include <vector>
#include <memory>

class Mesh
{
public:
	Mesh() = default;
	~Mesh() = default;

	void CreateCube(DX12Context& ctx);

public:
	std::unique_ptr<DX12Image> image_ = nullptr;
	uint32_t numVertices_ = 0;

	std::vector<VertexData> vertices_ = {};
	ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};

	std::vector<uint32_t> indices_ = {};
	ComPtr<ID3D12Resource> indexBuffer_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_ = {};

private:
	void CreateBuffers(DX12Context& ctx);
};

#endif