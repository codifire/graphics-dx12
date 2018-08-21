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

#include "Framework.h"

class Win32Application
{
public:
    static int Run(Framework* frameworkPtr, HINSTANCE hInstance, int nCmdShow);
    static HWND GetHwnd() { return m_hwnd; }
    static void SetCustomWindowText(LPCWSTR text);
    static const WCHAR* GetTitle() { return m_title.c_str(); }

protected:
    static void RegisterWindowClass(HINSTANCE hInst, const wchar_t* windowClassName);
    static HWND CreateWindow(const wchar_t* windowClassName, HINSTANCE hInst, const wchar_t* windowTitle, uint32_t width, uint32_t height, void* param);

    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static void SetFullScreen(bool goFullScreen);

private:
    static HWND m_hwnd;
    static std::wstring m_title;

    static RECT m_windowRect; // Window rectangle (used to toggle fullscreen state).
    static bool m_useFullScreen;

    static Framework* m_framework;
};
