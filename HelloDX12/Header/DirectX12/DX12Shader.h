#ifndef HELLO_DX12_SHADER
#define HELLO_DX12_SHADER

#include "DX12Context.h"

#include <string>

enum class ShaderType
{
	Vertex,
	Fragment,
	Compute,
	Invalid,
};

class DX12Shader
{
public:
	DX12Shader() = default;
	~DX12Shader() = default;

	void Destroy();
	IDxcBlob* GetHandle() const { return handle_; }
	void Create(DX12Context& ctx, const std::string& filename, ShaderType shaderType);

	void AddShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc) const;
	void AddShader(D3D12_COMPUTE_PIPELINE_STATE_DESC& psoDesc) const;

private:
	IDxcBlob* handle_{};
	ShaderType shaderType_ = ShaderType::Invalid;
};

#endif