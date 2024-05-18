#include "Scene.h"
#include "DX12Exception.h"

#include "glm/glm.hpp"

#include <iostream>

void Scene::Destroy()
{
	model_.Destroy();
	for (auto& buff : modelConstBuffs_)
	{
		buff.Destroy();
	}
	vertexBuffer_.Destroy();
	indexBuffer_.Destroy();
	meshDataBuffer_.Destroy();
}

void Scene::Init(DX12Context& ctx, const std::string& modelFileName)
{
	std::cout << "Load " << modelFileName << '\n';
	model_.Load(ctx, modelFileName, sceneData_);

	CreateModelBuffers(ctx);
	CreateBindlessResources(ctx);
}

void Scene::CreateBindlessResources(DX12Context& ctx)
{
	meshDataArray_.resize(model_.meshes_.size());
	for (uint32_t i = 0; i < model_.meshes_.size(); ++i)
	{
		meshDataArray_[i] = model_.meshes_[i].GetMeshData(0, 0);
	}

	const uint32_t vertexBufferSize = static_cast<uint32_t>(sizeof(VertexData) * sceneData_.vertices_.size());
	const uint32_t indexBufferSize = static_cast<uint32_t>(sizeof(uint32_t) * sceneData_.indices_.size());
	const uint32_t meshDataBufferSize = static_cast<uint32_t>(sizeof(MeshData) * meshDataArray_.size());

	vertexBuffer_.CreateDeviceOnlyBuffer(
		ctx,
		sceneData_.vertices_.data(),
		static_cast<uint32_t>(sceneData_.vertices_.size()),
		vertexBufferSize,
		sizeof(VertexData));

	indexBuffer_.CreateDeviceOnlyBuffer(
		ctx,
		sceneData_.indices_.data(),
		static_cast<uint32_t>(sceneData_.indices_.size()),
		indexBufferSize,
		sizeof(uint32_t));

	meshDataBuffer_.CreateDeviceOnlyBuffer(
		ctx,
		meshDataArray_.data(),
		static_cast<uint32_t>(meshDataArray_.size()),
		meshDataBufferSize,
		sizeof(MeshData));
}

DX12DescriptorArray Scene::GetImageDescriptors()
{
	DX12DescriptorArray dArray = {
		.rangeFlags_ = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
		.shaderVisibility_ = D3D12_SHADER_VISIBILITY_PIXEL,
	};
	dArray.buffers_.resize(model_.textures_.size());
	dArray.srvDescriptions_.resize(model_.textures_.size());
	
	for (uint32_t i = 0; i < model_.textures_.size(); ++i)
	{
		dArray.buffers_[i] = &(model_.textures_[i].buffer_);
		dArray.srvDescriptions_[i] = model_.textures_[i].buffer_.srvDescription_;
	}

	return dArray;
}

void Scene::CreateModelBuffers(DX12Context& ctx)
{
	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		modelConstBuffs_[i].CreateConstantBuffer(ctx, sizeof(CCamera));
	}

	// TODO Identity matrix for now
	for (uint32_t i = 0; i < AppConfig::FrameCount; ++i)
	{
		modelConsts_[i].modelMatrix = glm::transpose(glm::mat4(1.0f));
		modelConstBuffs_[i].UploadData(&(modelConsts_[i].modelMatrix));
	}
}