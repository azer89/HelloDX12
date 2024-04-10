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

using namespace Microsoft::WRL;

AppBase::AppBase(UINT width, UINT height, std::wstring name) :
	width_(width),
	height_(height),
	title_(name)
{
	aspectRatio_ = static_cast<float>(width) / static_cast<float>(height);
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
