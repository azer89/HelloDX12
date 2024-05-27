#ifndef HELLO_DX12_PIPELINE_IMGUI
#define HELLO_DX12_PIPELINE_IMGUI

#include "DX12Context.h"
#include "PipelineBase.h"
#include "ResourcesShared.h"
#include "Timer.h"

class PipelineImGui final : public PipelineBase
{
public:
	PipelineImGui(
		DX12Context& ctx,
		ResourcesShared* resourcesShared);
	~PipelineImGui();

	void Destroy();

	void Update(DX12Context& ctx, UIData& uiData) override {}
	void PopulateCommandList(DX12Context& ctx) override;

	void ImGuiStart();
	void ImGuiSetWindow(const char* title, int width, int height, float fontSize = 1.0f);
	void ImGuiEnd();
	void ImGuiDrawEmpty();

	void ImGuiShowFrameData(Timer* timer);

private:
	ResourcesShared* resourcesShared_ = nullptr;

	// TODO Use DX12DescriptorHeap
	ID3D12DescriptorHeap* descriptorHeap_ = nullptr;
};

#endif