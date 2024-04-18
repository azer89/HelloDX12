#ifndef HELLO_DX12_SCENE
#define HELLO_DX12_SCENE

#include "Model.h"

class Scene
{
public:
	Scene() = default;
	~Scene() = default;

	void Destroy()
	{
		model_.Destroy();
	}

	void Init(DX12Context& ctx);

public:
	Model model_;
};

#endif