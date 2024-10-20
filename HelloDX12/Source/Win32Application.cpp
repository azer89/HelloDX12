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

#include "Win32Application.h"

#include <iostream>
#include <WindowsX.h> // Mouse event

HWND Win32Application::m_hwnd = nullptr;

int Win32Application::Run(AppBase* pSample, HINSTANCE hInstance, int nCmdShow)
{
	// Parse the command line parameters
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	pSample->ParseCommandLineArgs(argv, argc);
	LocalFree(argv);

	// Initialize the window class.
	const WNDCLASSEX windowClass =
	{ 
		.cbSize = sizeof(WNDCLASSEX),
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = WindowProc,
		.hInstance = hInstance,
		.hCursor = LoadCursor(nullptr, IDC_ARROW),
		.lpszClassName = L"HelloDX12",
	};
	RegisterClassEx(&windowClass);

	RECT windowRect{ 0, 0, static_cast<LONG>(pSample->GetWidth()), static_cast<LONG>(pSample->GetHeight()) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	m_hwnd = CreateWindow(
		windowClass.lpszClassName,
		pSample->GetTitle(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,        // We have no parent window.
		nullptr,        // We aren't using menus.
		hInstance,
		pSample);

	// Initialize the sample. OnInit is defined in each child-implementation of DXSample.
	pSample->OnInit();

	ShowWindow(m_hwnd, nCmdShow);

	// Main sample loop.
	MSG msg{};
	while (msg.message != WM_QUIT)
	{
		// Process any messages in the queue.
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	pSample->OnDestroy();

	// Return this part of the WM_QUIT message to Windows.
	return static_cast<char>(msg.wParam);
}

// Main message handler for the sample.
LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	AppBase* app = reinterpret_cast<AppBase*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	// TODO ImGui
	if (app->WindowProc(hWnd, message, wParam, lParam))
	{
	}

	LPPOINT mousePos{};

	switch (message)
	{
	case WM_CREATE:
		{
			// Save the DXSample* passed in to CreateWindow.
			LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		}
		return 0;

	case WM_KEYDOWN:
		if (app)
		{
			app->OnKeyDown(static_cast<uint8_t>(wParam));
		}
		return 0;

	case WM_KEYUP:
		if (app)
		{
			app->OnKeyUp(static_cast<uint8_t>(wParam));
		}
		return 0;

	case WM_MOUSEMOVE:
		if (app)
		{
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			app->OnMouseMove(x, y);
		}
		return 0;

	case WM_LBUTTONUP:
		if (app)
		{
			app->OnMouseLeftRelease();
		}
		return 0;

	case WM_LBUTTONDOWN:
		if (app)
		{
			app->OnMouseLeftPressed();
		}
		return 0;

	case WM_PAINT:
		if (app)
		{
			app->OnUpdateInternal();
			app->OnUpdate();
			app->OnRender();
		}
		return 0;


	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			RECT rect;
			if (!::GetClientRect(hWnd, &rect))
			{
				throw std::runtime_error("Cannot get window rectangle");
			}

			app->OnWindowResize(rect.right, rect.bottom);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(hWnd, message, wParam, lParam);
}