#include "Model.h"

#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "PipelineMipmap.h"

#include <stdexcept>

static const std::string DEFAULT_BLACK_TEXTURE = "DefaultBlackTexture";
static const std::string DEFAULT_NORMAL_TEXTURE = "DefaultNormalTexture";

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

	for (auto & t : textures_)
	{
		t.Destroy();
	}
}

void Model::Load(
	DX12Context& ctx,
	const std::string& path,
	SceneData& sceneData)
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

	CreateDefaultTextures(ctx);

	// Retrieve the directory path of the filepath
	directory_ = filepath_.substr(0, filepath_.find_last_of('/'));

	// Process assimp's root node recursively
	ProcessNode(
		ctx,
		scene_->mRootNode,
		glm::mat4(1.0),
		sceneData);
}

void Model::ProcessNode(
	DX12Context& ctx,
	const aiNode* node,
	const glm::mat4& parentTransform,
	SceneData& sceneData)
{
	const glm::mat4 nodeTransform = CastToGLMMat4(node->mTransformation);
	const glm::mat4 totalTransform = parentTransform * nodeTransform;

	for (uint32_t i = 0; i < node->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene_->mMeshes[node->mMeshes[i]];
		ProcessMesh(
			ctx,
			mesh,
			totalTransform,
			sceneData);
	}

	for (uint32_t i = 0; i < node->mNumChildren; ++i)
	{
		ProcessNode(
			ctx,
			node->mChildren[i],
			totalTransform,
			sceneData);
	}
}

void Model::ProcessMesh(
	DX12Context& ctx,
	const aiMesh* mesh,
	const glm::mat4& transform,
	SceneData& sceneData)
{
	const std::string meshName = mesh->mName.C_Str();

	std::vector<VertexData> vertices = GetMeshVertices(mesh, transform);
	std::vector<uint32_t> indices = GetMeshIndices(mesh);
	std::unordered_map<TextureType, uint32_t> textureIndices = GetTextureIndices(ctx, mesh);

	const uint32_t prevVertexOffset = sceneData.GetCurrentVertexOffset();
	const uint32_t prevIndexOffset = sceneData.GetCurrentIndexOffset();

	sceneData.vertices_.insert(std::end(sceneData.vertices_), std::begin(vertices), std::end(vertices));
	sceneData.indices_.insert(std::end(sceneData.indices_), std::begin(indices), std::end(indices));

	const uint32_t currVertexOffset = static_cast<uint32_t>(vertices.size());
	const uint32_t currIndexOffset = static_cast<uint32_t>(indices.size());

	// Update offsets
	sceneData.vertexOffsets_.emplace_back(currVertexOffset + prevVertexOffset);
	sceneData.indexOffsets_.emplace_back(currIndexOffset + prevIndexOffset);

	meshes_.emplace_back(
		ctx,
		meshName,
		prevVertexOffset,
		prevIndexOffset,
		vertices.size(),
		indices.size(),
		std::move(vertices),
		std::move(indices),
		std::move(textureIndices));
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

void Model::CreateDefaultTextures(DX12Context& ctx)
{
	uint32_t black = 0xff000000;
	AddTexture(ctx, DEFAULT_BLACK_TEXTURE, (void*)&black, 1, 1);

	// TODO Investigate a correct value for normal vector
	uint32_t normal = 0xffff8888;
	AddTexture(ctx, DEFAULT_NORMAL_TEXTURE, (void*)&normal, 1, 1);
}

void Model::AddTexture(DX12Context& ctx, const std::string& textureFilename)
{
	const std::string fullFilePath = this->directory_ + '/' + textureFilename;
	textures_.emplace_back().Load(
		ctx,
		fullFilePath);
	textureMap_[textureFilename] = static_cast<uint32_t>(textures_.size() - 1);
}

void Model::AddTexture(DX12Context& ctx, const std::string& textureName, void* data, int width, int height)
{
	textures_.emplace_back().Load(
		ctx,
		data,
		1,
		1);
	textureMap_[textureName] = static_cast<uint32_t>(textures_.size() - 1);

	if (width > 1 && height > 1)
	{
		PipelineMipmap pip(ctx);
		pip.GenerateMipmap(ctx, &(textures_.back()));
	}
}

std::unordered_map<TextureType, uint32_t> Model::GetTextureIndices(DX12Context& ctx, const aiMesh* mesh)
{
	// PBR textures
	std::unordered_map<TextureType, uint32_t> textures;
	const aiMaterial* material = scene_->mMaterials[mesh->mMaterialIndex];
	for (const auto& aiTType : TextureMapper::aiTTypeSearchOrder)
	{
		const auto count = material->GetTextureCount(aiTType);
		for (unsigned int i = 0; i < count; ++i)
		{
			aiString str;
			material->GetTexture(aiTType, i, &str);
			std::string filename = str.C_Str();
			TextureType tType = TextureMapper::GetTextureType(aiTType);

			// Make sure each texture is loaded once
			if (!textureMap_.contains(filename))
			{
				AddTexture(ctx, filename);
			}

			// Only support one image per texture type, if we happen to load 
			// multiple textures of the same type, we only use one.
			if (!textures.contains(tType))
			{
				textures[tType] = textureMap_[filename];
			}
		}
	}

	// Replace missing PBR textures with a black 1x1 texture
	if (!textures.contains(TextureType::Albedo))
		{ textures[TextureType::Albedo] = textureMap_[DEFAULT_BLACK_TEXTURE]; }
	if (!textures.contains(TextureType::Normal))
		{ textures[TextureType::Normal] = textureMap_[DEFAULT_NORMAL_TEXTURE]; }
	if (!textures.contains(TextureType::Metalness))
		{ textures[TextureType::Metalness] = textureMap_[DEFAULT_BLACK_TEXTURE]; }
	if (!textures.contains(TextureType::Roughness))
		{ textures[TextureType::Roughness] = textureMap_[DEFAULT_BLACK_TEXTURE]; }
	if (!textures.contains(TextureType::AmbientOcclusion))
		{ textures[TextureType::AmbientOcclusion] = textureMap_[DEFAULT_BLACK_TEXTURE]; }
	if (!textures.contains(TextureType::Emissive))
		{ textures[TextureType::Emissive] = textureMap_[DEFAULT_BLACK_TEXTURE]; }

	return textures;
}