#ifndef HELLO_DX12_SCENE
#define HELLO_DX12_SCENE

#include "Mesh.h"

class Scene
{
public:
	Scene() = default;
	~Scene() = default;

	void Init(DX12Context& ctx);

public:
	Mesh mesh_;
};

#endif