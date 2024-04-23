#ifndef HELLO_DX12_SHADER
#define HELLO_DX12_SHADER

#include "DX12Context.h"

#include <string>

enum class ShaderType
{
	Vertex,
	Fragment,
	Compute,
};

class DX12Shader
{
public:
	DX12Shader() = default;
	~DX12Shader() = default;

	void Destroy();
	ID3DBlob* GetHandle() const { return handle_; }
	void Create(DX12Context& ctx, const std::string& filename, ShaderType shaderType);

private:
	ID3DBlob* handle_ = nullptr;
};

#endif