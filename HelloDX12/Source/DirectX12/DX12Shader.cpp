#include "DX12Shader.h"
#include "Utility.h"
#include "DX12Exception.h"

#include <stdexcept>
#include <iostream>

LPCWSTR GetEntryPoint(ShaderType shaderType)
{
	if (shaderType == ShaderType::Vertex) { return L"VSMain"; }
	if (shaderType == ShaderType::Fragment) { return L"PSMain"; }
	if (shaderType == ShaderType::Compute) { return L"CSMain"; }
	throw std::runtime_error("Shader type not recognized");
}

LPCWSTR GetTarget(ShaderType shaderType)
{
	if (shaderType == ShaderType::Vertex) { return L"vs_6_0"; }
	if (shaderType == ShaderType::Fragment) { return L"ps_6_0"; }
	if (shaderType == ShaderType::Compute) { return L"cs_6_0"; }
	throw std::runtime_error("Shader type not recognized");
}

void DX12Shader::Destroy()
{
	if (handle_) { handle_->Release(); }
}

void DX12Shader::Create(DX12Context& ctx, const std::string& filename, ShaderType shaderType)
{
	const std::wstring assetPath = Utility::WStringConvert(filename);

	const LPCWSTR entryPoint = GetEntryPoint(shaderType);
	const LPCWSTR target = GetTarget(shaderType);

	ComPtr<IDxcIncludeHandler> pIncludeHandler = nullptr;
	ctx.GetDXCUtils()->CreateDefaultIncludeHandler(&pIncludeHandler);

	LPCWSTR argument[] =
	{
		assetPath.c_str(),
		L"-E", entryPoint,
		L"-T", target,
		L"-Zs", // Enable debug information (slim format)
		//L"-D", L"MYDEFINE=1", // A single define.
		//L"-Fo", L"MyShader.bin", // Optional, stored in the pdb
		//L"-Fd", L"MyShader.pdb", // The file name of the pdb
		L"-Qstrip_reflect", // Strip reflection into a separate blob
	};

	ComPtr<IDxcBlobEncoding> pSource = nullptr;
	ctx.GetDXCUtils()->LoadFile(assetPath.c_str(), nullptr, &pSource);
	const DxcBuffer sourceBuffer =
	{
		.Ptr = pSource->GetBufferPointer(),
		.Size = pSource->GetBufferSize(),
		.Encoding = DXC_CP_ACP // Assume BOM says UTF8 or UTF16 or this is ANSI text.
	};
	
	ComPtr<IDxcResult> pResults;
	ctx.GetDXCCompiler()->Compile(
		&sourceBuffer,
		argument,
		_countof(argument),
		pIncludeHandler.Get(), // User-provided interface to handle #include directives (optional)
		IID_PPV_ARGS(&pResults) // Compiler output status, buffer, and errors
	);

	ComPtr<IDxcBlobUtf8> pErrors = nullptr;
	pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
	// Note that d3dcompiler would return null if no errors or warnings are present.
	// IDxcCompiler3::Compile will always return an error buffer, 
	// but its length will be zero if there are no warnings or errors.
	if (pErrors != nullptr && pErrors->GetStringLength() != 0)
	{
		wprintf(L"Warnings and Errors:\n%S\n", pErrors->GetStringPointer());
	}

	HRESULT hr;
	pResults->GetStatus(&hr);
	if (FAILED(hr))
	{
		throw std::runtime_error("Shader error");
	}

	ComPtr<IDxcBlobUtf16> pShaderName = nullptr;
	pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&handle_), &pShaderName);
	if (handle_ != nullptr)
	{
		std::cout << "Shader compiled " << filename << '\n';
	}
}