#ifndef HELLO_DX12_SCENE
#define HELLO_DX12_SCENE

#include "Model.h"
#include "Configs.h"
#include "ConstantBufferStructs.h"
#include "ScenePODs.h"
#include "DX12Descriptor.h"
#include "DX12Buffer.h"

#include <array>
#include <string>

class Scene
{
public:
	Scene() = default;
	~Scene() = default;

	void Destroy();
	void Init(DX12Context& ctx, const std::string& modelFileName);

	// TODO
	DX12DescriptorArray GetImageDescriptors();

	uint32_t GetMeshCount() const { return static_cast<uint32_t>(model_.meshes_.size()); }

private:
	void CreateModelBuffers(DX12Context& ctx);
	void CreateBindlessResources(DX12Context& ctx);

public:
	Model model_{};
	std::array<ModelMatrix, AppConfig::FrameCount> modelConsts_{};
	std::array<DX12Buffer, AppConfig::FrameCount> modelConstBuffs_{};

	SceneData sceneData_{}; // Containing vertices and indices
	std::vector<MeshData> meshDataArray_{};
	DX12Buffer vertexBuffer_{};
	DX12Buffer indexBuffer_{};
	DX12Buffer meshDataBuffer_{};
};

#endif