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
	IDxcBlob* GetHandle() const { return handle_; }
	void Create(DX12Context& ctx, const std::string& filename, ShaderType shaderType);

private:
	IDxcBlob* handle_ = nullptr;
};

#endif