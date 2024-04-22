#include "DX12Shader.h"
#include "Utility.h"
#include "DX12Exception.h"

#include <stdexcept>
#include <iostream>

LPCSTR GetEntryPoint(ShaderType shaderType)
{
	if (shaderType == ShaderType::Vertex) { return "VSMain"; }
	else if (shaderType == ShaderType::Fragment) { return "PSMain"; } 
	else if (shaderType == ShaderType::Compute) { return "CSMain"; }
	throw std::runtime_error("Shader type not recognized");
}

LPCSTR GetTarget(ShaderType shaderType)
{
	if (shaderType == ShaderType::Vertex) { return "vs_5_1"; }
	else if (shaderType == ShaderType::Fragment) { return "ps_5_1"; }
	else if (shaderType == ShaderType::Compute) { return "cs_5_1"; }
	throw std::runtime_error("Shader type not recognized");
}

void DX12Shader::Create(DX12Context& ctx, const std::string& filename, ShaderType shaderType)
{
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	uint32_t compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	uint32_t compileFlags = 0;
#endif
	std::wstring assetPath = Utility::WStringConvert(filename);

	LPCSTR entryPoint = GetEntryPoint(shaderType);
	LPCSTR target = GetTarget(shaderType);

	ID3DBlob* errorBuff; // Buffer holding the error data if any
	HRESULT hr = D3DCompileFromFile(
		assetPath.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint,
		target,
		compileFlags,
		0,
		&handle_,
		&errorBuff);
	if (FAILED(hr))
	{
		char* errorMessage = (char*)errorBuff->GetBufferPointer();
		OutputDebugStringA(errorMessage); // Print to Output window on Visual Studio
		std::cerr << errorMessage << '\n'; // Print to console
		throw std::runtime_error("Shader error");
	}
}