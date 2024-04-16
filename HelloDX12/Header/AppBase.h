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

#pragma once

#include "DX12Helper.h"
#include "Win32Application.h"
#include "Camera.h"
#include "UIData.h"

#include <memory>
#include <iostream>

class AppBase
{
public:
	AppBase();
	virtual ~AppBase();

	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnDestroy() = 0;

	// Samples override the event handlers to handle specific messages.
	void OnKeyDown(uint8_t key) 
	{
		// TODO
		float fpsTemp = 0.1;

		if (key == 'W')
		{
			camera_->ProcessKeyboard(CameraMovement::Forward, fpsTemp);
		}
		else if (key == 'A')
		{
			camera_->ProcessKeyboard(CameraMovement::Left, fpsTemp);
		}
		else if (key == 'S')
		{
			camera_->ProcessKeyboard(CameraMovement::Backward, fpsTemp);
		}
		else if (key == 'D')
		{
			camera_->ProcessKeyboard(CameraMovement::Right, fpsTemp);
		}
	}

	void OnKeyUp(uint8_t key) 
	{
	}

	void OnMouseMove(int mousePositionX, int mousePositionY)
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

	void OnMouseLeftPressed()
	{
		uiData_.mouseLeftPressed_ = true;
	}

	void OnMouseLeftRelease()
	{
		uiData_.mouseLeftPressed_ = false;
	}

	// Accessors.
	uint32_t GetWidth() const { return width_; }
	uint32_t GetHeight() const { return height_; }
	const WCHAR* GetTitle() const { return title_.c_str(); }

	void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

protected:
	void SetCustomWindowText(LPCWSTR text);

protected:
	uint32_t width_;
	uint32_t height_;
	float aspectRatio_;

	UIData uiData_;

	std::unique_ptr<Camera> camera_;

private:
	// Window title.
	std::wstring title_;
};