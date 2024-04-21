#ifndef HELLO_DX12_SCENE
#define HELLO_DX12_SCENE

#include "Model.h"
#include "Configs.h"
#include "ConstantDefinitions.h"

#include <array>

class Scene
{
public:
	Scene() = default;
	~Scene() = default;

	void Destroy();

	void Init(DX12Context& ctx);

private:
	void CreateModelBuffers(DX12Context& ctx);

public:
	Model model_;
	std::array<CModel, AppConfig::FrameCount> modelConsts_;
	std::array<DX12Buffer, AppConfig::FrameCount> modelConstBuffs_;
};

#endif