#ifndef HELLO_DX12_APP_SIMPLE
#define HELLO_DX12_APP_SIMPLE

#include "stdafx.h"
#include "DXSample.h"
#include "DX12Context.h"

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

class AppSimple : public DXSample
{
public:
	AppSimple(UINT width, UINT height, std::wstring name);

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();

private:
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT2 uv;
	};

	DX12Context context_ = {};

private:
	void LoadPipeline();
	void LoadAssets();
	std::vector<UINT8> GenerateTextureData();
	void PopulateCommandList();
	void WaitForPreviousFrame();
};

#endif