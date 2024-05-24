#include "PipelineImGui.h"
#include "Win32Application.h"
#include "Configs.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

PipelineImGui::PipelineImGui(
	DX12Context& ctx,
	ResourcesShared* resourcesShared) :
	PipelineBase(ctx),
	resourcesShared_(resourcesShared)
{
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = 
		{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			.NumDescriptors = 1,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		};
		ThrowIfFailed(ctx.GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap_)));
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(Win32Application::GetHwnd());
	ImGui_ImplDX12_Init(ctx.GetDevice(), AppConfig::FrameCount,
		DXGI_FORMAT_R8G8B8A8_UNORM, descriptorHeap_,
		descriptorHeap_->GetCPUDescriptorHandleForHeapStart(),
		descriptorHeap_->GetGPUDescriptorHandleForHeapStart());
}

PipelineImGui::~PipelineImGui()
{
	Destroy();
}

void PipelineImGui::Destroy()
{
	if (descriptorHeap_)
	{
		descriptorHeap_->Release();
	}

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void PipelineImGui::PopulateCommandList(DX12Context& ctx)
{
	ImDrawData* drawData = ImGui::GetDrawData();
	if (drawData)
	{
		ID3D12GraphicsCommandList* commandList = ctx.GetCommandList();
		commandList->SetDescriptorHeaps(1, &descriptorHeap_);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
	}
}

void PipelineImGui::ImGuiStart()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void PipelineImGui::ImGuiSetWindow(const char* title, int width, int height, float fontSize)
{
	ImGui::SetWindowSize(ImVec2(static_cast<float>(width), static_cast<float>(height)));
	ImGui::Begin(title);
	ImGui::SetWindowFontScale(fontSize);
}

void PipelineImGui::ImGuiEnd()
{
	ImGui::End();
	ImGui::Render();
}

void PipelineImGui::ImGuiDrawEmpty()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::Render();
}