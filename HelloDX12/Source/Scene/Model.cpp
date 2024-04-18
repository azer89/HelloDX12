#include "Model.h"

#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <stdexcept>

inline glm::mat4 CastToGLMMat4(const aiMatrix4x4& m)
{
	return glm::transpose(glm::make_mat4(&m.a1));
}

void Model::Destroy()
{
	for (auto& m : meshes_)
	{
		m.Destroy();
	}
}

void Model::Load(DX12Context& ctx, const std::string& path)
{
	filepath_ = path;
	Assimp::Importer importer;
	scene_ = importer.ReadFile(
		filepath_,
		aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
	// Check for errors
	if (!scene_ || scene_->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene_->mRootNode) // if is Not Zero
	{
		throw std::runtime_error("Cannot load file " + path);
	}

	// Retrieve the directory path of the filepath
	directory_ = filepath_.substr(0, filepath_.find_last_of('/'));

	// Process assimp's root node recursively
	ProcessNode(
		ctx,
		scene_->mRootNode,
		glm::mat4(1.0));
}

void Model::ProcessNode(
	DX12Context& ctx,
	const aiNode* node,
	const glm::mat4& parentTransform)
{
	const glm::mat4 nodeTransform = CastToGLMMat4(node->mTransformation);
	const glm::mat4 totalTransform = parentTransform * nodeTransform;

	for (uint32_t i = 0; i < node->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene_->mMeshes[node->mMeshes[i]];
		ProcessMesh(
			ctx,
			mesh,
			totalTransform);
	}

	for (uint32_t i = 0; i < node->mNumChildren; ++i)
	{
		ProcessNode(
			ctx,
			node->mChildren[i],
			totalTransform);
	}
}

void Model::ProcessMesh(
	DX12Context& ctx,
	const aiMesh* mesh,
	const glm::mat4& transform)
{
	const std::string meshName = mesh->mName.C_Str();

	std::vector<VertexData> vertices = GetMeshVertices(mesh, transform);
	std::vector<uint32_t> indices = GetMeshIndices(mesh);

	meshes_.emplace_back(ctx, meshName, std::move(vertices), std::move(indices));
}

std::vector<VertexData> Model::GetMeshVertices(const aiMesh* mesh, const glm::mat4& transform)
{
	std::vector<VertexData> vertices(mesh->mNumVertices);
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		glm::vec3 position = 
			glm::vec3(transform *
				glm::vec4(
					mesh->mVertices[i].x,
					mesh->mVertices[i].y,
					mesh->mVertices[i].z,
					1));

		glm::vec3 normal = 
			glm::vec3(transform *
				glm::vec4(
					mesh->mNormals[i].x,
					mesh->mNormals[i].y,
					mesh->mNormals[i].z,
					0));

		glm::vec2 uv = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);

		vertices[i] = VertexData(position, normal, uv);
	}
	return vertices;
}

std::vector<uint32_t> Model::GetMeshIndices(const aiMesh* mesh)
{
	std::vector<uint32_t> indices;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		const aiFace& face = mesh->mFaces[i];
		// Retrieve all indices of the face and store them in the indices vector
		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			indices.push_back(static_cast<uint32_t>(face.mIndices[j]));
		}
	}
	return indices;
}