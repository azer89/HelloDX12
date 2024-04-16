#include "Scene.h"
#include "DX12Exception.h"

#include "glm/glm.hpp"

void Scene::Init(DX12Context& ctx)
{
	// Create the vertex buffer.
	{
		// Define the geometry for a triangle.
		VertexData triangleVertices[] =
		{
			{ glm::vec3(0.0f,  0.5f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.5f, 0.0f) },
			{ glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f) },

			{ glm::vec3(0.0f,  1.0f, -2.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.5f, 0.0f) },
			{ glm::vec3(1.0f, -1.0f, -2.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(-1.0f, -1.0f, -2.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f) },
		};

		const uint32_t vertexBufferSize = sizeof(triangleVertices);

		// Note: using upload heaps to transfer static data like vert buffers is not 
		// recommended. Every time the GPU needs it, the upload heap will be marshalled 
		// over. Please read up on Default Heap usage. An upload heap is used here for 
		// code simplicity and because there are very few verts to actually transfer
		{
			auto vHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto vResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
			ThrowIfFailed(ctx.GetDevice()->CreateCommittedResource(
				&vHeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&vResourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&vertexBuffer_)));
		}

		// Copy the triangle data to the vertex buffer
		uint8_t* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU
		ThrowIfFailed(vertexBuffer_->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		vertexBuffer_->Unmap(0, nullptr);

		// Initialize the vertex buffer view
		vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
		vertexBufferView_.StrideInBytes = sizeof(VertexData);
		vertexBufferView_.SizeInBytes = vertexBufferSize;

		// Create index buffer
		static const uint16_t indices[] =
		{
			0, 1, 2,
			3, 4, 5
		};

		const UINT indexBufferSize = sizeof(indices);

		auto iHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto iResourceDescription = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
		ThrowIfFailed(ctx.GetDevice()->CreateCommittedResource(
			&iHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&iResourceDescription,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&indexBuffer_)));

		// Copy the cube data to the vertex buffer.
		ThrowIfFailed(indexBuffer_->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, indices, sizeof(indices));
		indexBuffer_->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		indexBufferView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
		indexBufferView_.Format = DXGI_FORMAT_R16_UINT;
		indexBufferView_.SizeInBytes = indexBufferSize;
	}

	// Create the texture.
	image_ = std::make_unique<DX12Image>(ctx);
}