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

	void OnKeyDown(uint8_t key);
	void OnKeyUp(uint8_t key);
	void OnMouseMove(int mousePositionX, int mousePositionY);
	void OnMouseLeftPressed();
	void OnMouseLeftRelease();
	void OnKeyboardInput();

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