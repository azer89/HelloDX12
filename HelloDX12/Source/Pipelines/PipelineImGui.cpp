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
	ImGui::StyleColorsDark();

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

void PipelineImGui::ImGuiShowFrameData(Timer* timer)
{
	ImVec2 wSize = ImGui::GetWindowSize();

	ImGui::Text("FPS: %.0f", timer->GetFPSDelayed());
	ImGui::Text("Delta: %.0f ms", timer->GetDeltaDelayed());
	ImGui::PlotLines("",
		timer->GetGraph(),
		timer->GetGraphLength(),
		0,
		nullptr,
		FLT_MAX,
		FLT_MAX,
		ImVec2(static_cast<float>(wSize.x - 15), 50));
}

void PipelineImGui::ImGuiShowPBRConfig(CPBR* constBuffPBR)
{
	ImGui::SliderFloat("Light Falloff", &(constBuffPBR->lightFalloff), 0.01f, 5.f);
	ImGui::SliderFloat("Light Intensity", &(constBuffPBR->lightIntensity), 0.1f, 20.f);
	ImGui::SliderFloat("Reflection LOD", &(constBuffPBR->maxReflectionLod), 0.1f, 5.0f);
	ImGui::SliderFloat("Albedo", &(constBuffPBR->albedoMultipler), 0.0f, 1.0f);
	ImGui::SliderFloat("Reflectivity", &(constBuffPBR->baseReflectivity), 0.01f, 1.f);
}