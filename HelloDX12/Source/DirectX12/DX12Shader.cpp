#include "DX12Shader.h"
#include "Utility.h"
#include "DX12Exception.h"

void DX12Shader::Create(DX12Context& ctx, const std::string& filename, ShaderType shaderType)
{
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	uint32_t compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	uint32_t compileFlags = 0;
#endif
	std::wstring assetPath = Utility::WStringConvert(filename);

	LPCSTR entryPoint = shaderType == ShaderType::Vertex ? "VSMain" : "PSMain";
	LPCSTR target = shaderType == ShaderType::Vertex ? "vs_5_0" : "ps_5_0";

	ThrowIfFailed(D3DCompileFromFile(assetPath.c_str(), nullptr, nullptr, entryPoint, target, compileFlags, 0, &handle_, nullptr));
}