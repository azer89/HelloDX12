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
private:
	Scene() = default;
	~Scene() = default;
};

#endif