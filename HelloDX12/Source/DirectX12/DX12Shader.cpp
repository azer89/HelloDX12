#include "DX12Shader.h"
#include "DX12Exception.h"
#include "Utility.h"

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
	if (shaderType == ShaderType::Vertex) { return L"vs_6_6"; }
	if (shaderType == ShaderType::Fragment) { return L"ps_6_6"; }
	if (shaderType == ShaderType::Compute) { return L"cs_6_6"; }
	throw std::runtime_error("Shader type not recognized");
}

void DX12Shader::Destroy()
{
	if (handle_) { handle_->Release(); }
}

void DX12Shader::Create(DX12Context& ctx, const std::string& filename, ShaderType shaderType)
{
	shaderType_ = shaderType;

	const std::wstring assetPath = Utility::WStringConvert(filename);

	const LPCWSTR entryPoint = GetEntryPoint(shaderType);
	const LPCWSTR target = GetTarget(shaderType);

	IDxcIncludeHandler* pIncludeHandler{};
	ctx.GetDXCUtils()->CreateDefaultIncludeHandler(&pIncludeHandler);

	LPCWSTR argument[] =
	{
		assetPath.c_str(),
		L"-E", entryPoint,
		L"-T", target,
		L"-Zi", // Enable debug information
		//L"-D", L"MYDEFINE=1", // A single define.
		//L"-Fo", L"MyShader.bin", // Optional, stored in the pdb
		//L"-Fd", L"MyShader.pdb", // The file name of the pdb
		//L"-Qstrip_reflect", // Strip reflection
	};

	IDxcBlobEncoding* pSource{};
	ctx.GetDXCUtils()->LoadFile(assetPath.c_str(), nullptr, &pSource);
	const DxcBuffer sourceBuffer =
	{
		.Ptr = pSource->GetBufferPointer(),
		.Size = pSource->GetBufferSize(),
		.Encoding = DXC_CP_ACP // Assume BOM says UTF8 or UTF16 or this is ANSI text.
	};
	
	IDxcResult* pResults{};
	ctx.GetDXCCompiler()->Compile(
		&sourceBuffer,
		argument,
		_countof(argument),
		pIncludeHandler, // User-provided interface to handle #include directives (optional)
		IID_PPV_ARGS(&pResults) // Compiler output status, buffer, and errors
	);

	IDxcBlobUtf8* pErrors{};
	pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
	// Note that d3dcompiler would return null if no errors or warnings are present.
	// IDxcCompiler3::Compile will always return an error buffer, 
	// but its length will be zero if there are no warnings or errors.
	if (pErrors != nullptr && pErrors->GetStringLength() != 0)
	{
		wprintf(L"Warnings and errors:\n%S\n", pErrors->GetStringPointer());
	}

	HRESULT hr;
	pResults->GetStatus(&hr);
	if (FAILED(hr))
	{
		throw std::runtime_error("Shader error " + filename);
	}

	IDxcBlobUtf16* pOutputName{};
	pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&handle_), &pOutputName);
	if (handle_ == nullptr)
	{
		throw std::runtime_error("Shader error " + filename);
	}

	if (pOutputName != nullptr)
	{
		pOutputName->Release();
	}
	pErrors->Release();
	pResults->Release();
	pSource->Release();
	pIncludeHandler->Release();
}

void DX12Shader::AddShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc) const
{
	if (shaderType_ == ShaderType::Vertex)
	{
		psoDesc.VS.BytecodeLength = handle_->GetBufferSize();
		psoDesc.VS.pShaderBytecode = handle_->GetBufferPointer();
	}
	else if (shaderType_ == ShaderType::Fragment)
	{
		psoDesc.PS.BytecodeLength = handle_->GetBufferSize();
		psoDesc.PS.pShaderBytecode = handle_->GetBufferPointer();
	}
}

void DX12Shader::AddShader(D3D12_COMPUTE_PIPELINE_STATE_DESC& psoDesc) const
{
	psoDesc.CS.BytecodeLength = handle_->GetBufferSize();
	psoDesc.CS.pShaderBytecode = handle_->GetBufferPointer();
}