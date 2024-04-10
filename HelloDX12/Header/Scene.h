#ifndef HELLO_DX12_SCENE
#define HELLO_DX12_SCENE

#include "DX12Context.h"

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 uv;
};

class Scene
{
public:
	Scene() = default;
	~Scene() = default;

	void Init(const DX12Context& ctx);

private:
	std::vector<UINT8> GenerateTextureData(const DX12Context& ctx);

public:
	static const UINT TextureWidth = 256;
	static const UINT TextureHeight = 256;
	static const UINT TexturePixelSize = 4;

	ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	ComPtr<ID3D12Resource> texture_;

private:
	// Note: ComPtr's are CPU objects but this resource needs to stay in scope until
	// the command list that references it has finished executing on the GPU.
	// We will flush the GPU at the end of this method to ensure the resource is not
	// prematurely destroyed.
	ComPtr<ID3D12Resource> textureUploadHeap_;
};

#endif