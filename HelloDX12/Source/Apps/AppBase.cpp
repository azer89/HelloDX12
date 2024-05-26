#include "AppBase.h"
#include "Configs.h"
#include "Utility.h"

#include "imgui.h"
#include "imgui_impl_win32.h"

#include <windows.h>
#include <fcntl.h>
#include <iostream>

using namespace Microsoft::WRL;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

AppBase::AppBase() :
	windowWidth_(AppConfig::InitialScreenWidth),
	windowHeight_(AppConfig::InitialScreenHeight),
	windowAspectRatio_(static_cast<float>(AppConfig::InitialScreenWidth) / static_cast<float>(AppConfig::InitialScreenHeight)),
	camera_(std::make_unique<Camera>(glm::vec3(0.0f))),
	title_(Utility::WStringConvert(AppConfig::ScreenTitle))
{
	ConsoleShow();
}

AppBase::~AppBase()
{
}

// Helper function for setting the window's title text.
void AppBase::SetCustomWindowText(LPCWSTR text) const
{
	std::wstring windowText = title_ + L": " + text;
	SetWindowText(Win32Application::GetHwnd(), windowText.c_str());
}

LRESULT AppBase::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
}

void AppBase::OnWindowResize(uint32_t width, uint32_t height)
{
	if (width == windowWidth_ && height == windowHeight_)
	{
		return;
	}

	windowResize_ = true;
	targetWindowWidth_ = width;
	targetWindowHeight_ = height;
}

// Helper function for parsing any supplied command line args.
_Use_decl_annotations_
void AppBase::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
	for (int i = 1; i < argc; ++i)
	{
		// Empty
	}
}

void AppBase::OnKeyDown(uint8_t key)
{
}

void AppBase::OnKeyUp(uint8_t key)
{
}

void AppBase::OnMouseMove(int mousePositionX, int mousePositionY)
{
	if (uiData_.mouseFirstUse_)
	{
		uiData_.mousePositionX_ = static_cast<float>(mousePositionX);
		uiData_.mousePositionY_ = static_cast<float>(mousePositionY);
		uiData_.mouseFirstUse_ = false;
		return;
	}

	if (!ImGui::GetIO().WantCaptureMouse && (uiData_.mouseLeftPressed_ || uiData_.mouseLeftHold_))
	{
		const float xOffset = static_cast<float>(mousePositionX) - uiData_.mousePositionX_;
		const float yOffset = uiData_.mousePositionY_ - static_cast<float>(mousePositionY); // Reversed since y-coordinates go from bottom to top
		camera_->ProcessMouseMovement(xOffset, yOffset);
	}

	uiData_.mousePositionX_ = static_cast<float>(mousePositionX);
	uiData_.mousePositionY_ = static_cast<float>(mousePositionY);
}

void AppBase::OnMouseLeftPressed()
{
	uiData_.mouseLeftPressed_ = true;
}

void AppBase::OnMouseLeftRelease()
{
	uiData_.mouseLeftPressed_ = false;
}

void AppBase::OnKeyboardInput()
{
	// TODO
	float fpsTemp = 0.01f;

	if (GetAsyncKeyState('W') & 0x8000) { camera_->ProcessKeyboard(CameraMovement::Forward, fpsTemp); }
	if (GetAsyncKeyState('S') & 0x8000) { camera_->ProcessKeyboard(CameraMovement::Backward, fpsTemp); }
	if (GetAsyncKeyState('A') & 0x8000) { camera_->ProcessKeyboard(CameraMovement::Left, fpsTemp); }
	if (GetAsyncKeyState('D') & 0x8000) { camera_->ProcessKeyboard(CameraMovement::Right, fpsTemp); }
}

void AppBase::ConsoleShow()
{
	if (AllocConsole())
	{
		FILE* fp;
		if(freopen_s(&fp, "CONOUT$", "w", stdout) != 0)
		{
			std::cerr << "Cannot open stdout\n";
		}
		if (freopen_s(&fp, "CONOUT$", "w", stderr) != 0)
		{
			std::cerr << "Cannot open stderr\n";
		}
	}
}

void AppBase::BeginRender()
{
	context_.ResetCommandAllocator();
	context_.ResetCommandList();
}

void AppBase::EndRender()
{
	context_.SubmitCommandList();
	context_.PresentSwapchain();
	
	if (windowResize_)
	{
		context_.WaitForAllFrames();
		context_.ResizeSwapchain(targetWindowWidth_, targetWindowHeight_);
		camera_->SetScreenSize(static_cast<float>(targetWindowWidth_), static_cast<float>(targetWindowHeight_));

		for (auto& res : resources_)
		{
			res->OnWindowResize(context_, targetWindowWidth_, targetWindowHeight_);
		}

		for (auto& pip : pipelines_)
		{
			pip->SetupViewportAndScissor(context_);
			pip->OnWindowResize(context_, targetWindowWidth_, targetWindowHeight_);
		}

		windowWidth_ = targetWindowWidth_;
		windowHeight_ = targetWindowHeight_;
		windowResize_ = false;
	}
	else
	{
		context_.MoveToNextFrame();
	}
}
