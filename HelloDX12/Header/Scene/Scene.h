#ifndef HELLO_DX12_SCENE
#define HELLO_DX12_SCENE

#include "Model.h"
#include "Configs.h"
#include "ConstantDefinitions.h"
#include "ScenePODs.h"
#include "DX12Descriptor.h"
#include "DX12Buffer.h"

#include <array>

class Scene
{
public:
	Scene() = default;
	~Scene() = default;

	void Destroy();
	void Init(DX12Context& ctx);

	// TODO
	DX12DescriptorArray GetImageDescriptors();

private:
	void CreateModelBuffers(DX12Context& ctx);
	void CreateBindlessResources(DX12Context& ctx);

public:
	Model model_ = {};
	std::array<ModelMatrix, AppConfig::FrameCount> modelConsts_ = {};
	std::array<DX12Buffer, AppConfig::FrameCount> modelConstBuffs_ = {};

	SceneData sceneData_ = {}; // Containing vertices and indices
	std::vector<MeshData> meshDataArray_ = {};
	DX12Buffer vertexBuffer_ = {};
	DX12Buffer indexBuffer_ = {};
	DX12Buffer meshDataBuffer_ = {};
};

#endif