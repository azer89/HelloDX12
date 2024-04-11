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
	aspectRatio_(static_cast<float>(AppConfig::InitialScreenWidth) / static_cast<float>(AppConfig::InitialScreenHeight))
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
