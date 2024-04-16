//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#include "AppBase.h"
#include "Configs.h"
#include "Utility.h"

using namespace Microsoft::WRL;

AppBase::AppBase() :
	width_(AppConfig::InitialScreenWidth),
	height_(AppConfig::InitialScreenHeight),
	title_(Utility::WStringConvert(AppConfig::ScreenTitle)),
	aspectRatio_(static_cast<float>(AppConfig::InitialScreenWidth) / static_cast<float>(AppConfig::InitialScreenHeight)),
	camera_(std::make_unique<Camera>(glm::vec3(0.0f)))
{
}

AppBase::~AppBase()
{
}

// Helper function for setting the window's title text.
void AppBase::SetCustomWindowText(LPCWSTR text)
{
	std::wstring windowText = title_ + L": " + text;
	SetWindowText(Win32Application::GetHwnd(), windowText.c_str());
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
		uiData_.mousePositionX = mousePositionX;
		uiData_.mousePositionY = mousePositionY;
		uiData_.mouseFirstUse_ = false;
		return;
	}

	if (uiData_.mouseLeftPressed_ || uiData_.mouseLeftHold_)
	{
		const float xOffset = mousePositionX - uiData_.mousePositionX;
		const float yOffset = uiData_.mousePositionY - mousePositionY; // Reversed since y-coordinates go from bottom to top
		camera_->ProcessMouseMovement(xOffset, yOffset);
	}

	uiData_.mousePositionX = mousePositionX;
	uiData_.mousePositionY = mousePositionY;
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
