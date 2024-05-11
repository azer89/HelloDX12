#ifndef HELLO_DX12_MODEL
#define HELLO_DX12_MODEL

#include "DX12Context.h"
#include "VertexData.h"
#include "Mesh.h"

#include <string>
#include <vector>

#include "assimp/scene.h"

class Model
{
public:
	Model() = default;
	~Model() = default;

	void Destroy();
	void Load(DX12Context& ctx, const std::string& path);

private:
	void ProcessNode(
		DX12Context& ctx,
		const aiNode* node,
		const glm::mat4& parentTransform);

	void ProcessMesh(
		DX12Context& ctx,
		const aiMesh* mesh,
		const glm::mat4& transform);

	[[nodiscard]] std::vector<VertexData> GetMeshVertices(const aiMesh* mesh, const glm::mat4& transform);
	[[nodiscard]] std::vector<uint32_t> GetMeshIndices(const aiMesh* mesh);

	void CreateDefaultTextures(DX12Context& ctx);
	void AddTexture(DX12Context& ctx, const std::string& textureFilename);
	void AddTexture(DX12Context& ctx, const std::string& textureName, void* data, int width, int height);
		
public:
	std::string filepath_ = {};
	std::string directory_ = {};
	std::vector<Mesh> meshes_ = {};

	std::vector<DX12Image> textures_ = {};
	std::unordered_map<std::string, uint32_t> textureMap_ = {};

private:
	const aiScene* scene_ = nullptr;
};

#endif