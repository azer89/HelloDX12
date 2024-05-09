#ifndef HELLO_DX12_MESH
#define HELLO_DX12_MESH

#include "DX12Context.h"
#include "DX12Image.h"
#include "DX12Buffer.h"
#include "VertexData.h"

#include <vector>
#include <memory>

class Mesh
{
public:
	Mesh(
		DX12Context& ctx,
		const std::string& meshName,
		std::vector<VertexData>&& vertices,
		std::vector<uint32_t>&& indices);

	void Destroy();

	void CreateCube(DX12Context& ctx);

public:
	std::string meshName_ = {};

	std::unique_ptr<DX12Image> image_ = nullptr;
	uint32_t vertexCount_ = 0;

	std::vector<VertexData> vertices_ = {};
	DX12Buffer vertexBuffer_ = {};
	DX12Buffer vertexBuffer2_ = {};

	std::vector<uint32_t> indices_ = {};
	DX12Buffer indexBuffer_ = {};
	DX12Buffer indexBuffer2_ = {};

private:
	void CreateBuffers(DX12Context& ctx);
};

#endif