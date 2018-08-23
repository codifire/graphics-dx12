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

#include "stdafx.h"
#include "Win32Application.h"
#include "Resource.h"

// In order to define a function called CreateWindow, the Windows macro needs to be undefined.
#if defined(CreateWindow)
#undef CreateWindow
#endif

HWND Win32Application::m_hwnd = nullptr;
std::wstring Win32Application::m_title = _T("graphics");

RECT Win32Application::m_windowRect = {}; // Window rectangle (used to toggle fullscreen state).
bool Win32Application::m_useFullScreen = false;

Framework* Win32Application::m_framework = nullptr;


void Win32Application::RegisterWindowClass(HINSTANCE hInst, const wchar_t* windowClassName)
{
    // Register a window class for creating our render window with.
    WNDCLASSEXW windowClass = {0};

    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &WindowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInst;
    windowClass.hIcon = ::LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMALL)); //  MAKEINTRESOURCE(APPLICATION_ICON));
    windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = windowClassName;
    windowClass.hIconSm = ::LoadIcon(hInst, MAKEINTRESOURCE(IDI_GRAPHICS)); //  MAKEINTRESOURCE(APPLICATION_ICON));

    static ATOM atom = ::RegisterClassExW(&windowClass);
    assert(atom > 0);
}

HWND Win32Application::CreateWindow(const wchar_t* windowClassName, HINSTANCE hInst, const wchar_t* windowTitle,
                                    uint32_t width, uint32_t height, void* param)
{
    int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

    RECT windowRect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    ::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    // Center the window within the screen. Clamp to 0, 0 for the top-left corner.
    int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
    int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

    HWND hWnd = ::CreateWindowExW(
        NULL,
        windowClassName,
        windowTitle,
        WS_OVERLAPPEDWINDOW,
        windowX,
        windowY,
        windowWidth,
        windowHeight,
        nullptr,
        nullptr,
        hInst,
        param
    );

    assert(hWnd && "Failed to create window");

    return hWnd;
}

int Win32Application::Run(Framework* frameworkPtr, HINSTANCE hInstance, int nCmdShow)
{
    m_framework = frameworkPtr;
    // Parse the command line parameters
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    frameworkPtr->ParseCommandLineArgs(argv, argc);
    LocalFree(argv);

    // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
    // Using this awareness context allows the client area of the window 
    // to achieve 100% scaling while still allowing non-client window content to 
    // be rendered in a DPI sensitive fashion.
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    const wchar_t* windowClassName = L"Graphics-DX12";

    // Initialize the window class.
    RegisterWindowClass(hInstance, windowClassName);

    m_hwnd = CreateWindow(windowClassName, hInstance, L"Rendering", frameworkPtr->GetWidth(), frameworkPtr->GetHeight(),
                          frameworkPtr);

    ::GetWindowRect(GetHwnd(), &m_windowRect); // cache settings

    frameworkPtr->Init();

    ShowWindow(m_hwnd, nCmdShow);

    // Main sample loop.
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    frameworkPtr->Release();

    // Return this part of the WM_QUIT message to Windows.
    return static_cast<char>(msg.wParam);
}

void Win32Application::SetCustomWindowText(LPCWSTR text)
{
    std::wstring windowText = m_title + L": " + text;
    SetWindowText(Win32Application::GetHwnd(), windowText.c_str());
}

void Win32Application::SetFullScreen(bool goFullScreen)
{
    if (m_useFullScreen != goFullScreen)
    {
        m_useFullScreen = goFullScreen;

        if (m_useFullScreen)
        {
            ::GetWindowRect(GetHwnd(), &m_windowRect); // cache older settings

            // Set the window style to a borderless window so the client area fills the entire screen.
            const auto NewWindowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX
                | WS_MAXIMIZEBOX);
            ::SetWindowLongW(GetHwnd(), GWL_STYLE, NewWindowStyle);

            // Query the name of the nearest display device for the window. 
            // This is required to set the fullscreen dimensions of the window when using a multi-monitor setup.
            auto hMonitor = ::MonitorFromWindow(GetHwnd(), MONITOR_DEFAULTTONEAREST);

            MONITORINFOEX monitorInfo = {};
            monitorInfo.cbSize = sizeof(MONITORINFOEX);
            ::GetMonitorInfo(hMonitor, &monitorInfo);

            ::SetWindowPos(GetHwnd(), HWND_TOP,
                           monitorInfo.rcMonitor.left,
                           monitorInfo.rcMonitor.top,
                           monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                           monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                           SWP_FRAMECHANGED | SWP_NOACTIVATE);

            ::ShowWindow(GetHwnd(), SW_MAXIMIZE);
        }
        else
        {
            ::SetWindowLongW(GetHwnd(), GWL_STYLE, WS_OVERLAPPEDWINDOW);
            ::SetWindowPos(GetHwnd(), HWND_NOTOPMOST,
                           m_windowRect.left,
                           m_windowRect.top,
                           m_windowRect.right - m_windowRect.left,
                           m_windowRect.bottom - m_windowRect.top,
                           SWP_FRAMECHANGED | SWP_NOACTIVATE);

            ::ShowWindow(GetHwnd(), SW_NORMAL);
        }
    }
}

// Main message handler for the sample.
LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Framework* frameworkPtr = reinterpret_cast<Framework*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
        {
            // Save the Framework* passed in to CreateWindow.
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
        return 0;
    case WM_PAINT:
        if (frameworkPtr && frameworkPtr->HasInitialized())
        {
            frameworkPtr->Update();
            frameworkPtr->Render();
        }
        return 0;
    case WM_SIZE:
        {
            if (frameworkPtr)
            {
                RECT clientRect = {};
                ::GetClientRect(GetHwnd(), &clientRect);

                const auto width = clientRect.right - clientRect.left;
                const auto height = clientRect.bottom - clientRect.top;

                frameworkPtr->Resize(width, height);
            }
        }
        return 0;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
        {
            bool propagateKeyPress = true;
            bool altPressed = (::GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

            switch(wParam)
            {
            case VK_ESCAPE:
                propagateKeyPress = false;
                ::PostQuitMessage(0);
                break;
            case VK_RETURN:
                if (altPressed)
                {
            case VK_F11:
                    propagateKeyPress = false;
                    SetFullScreen(!m_useFullScreen);
                }
                break;
            }

            if (frameworkPtr && propagateKeyPress)
            {
                frameworkPtr->KeyDown(static_cast<UINT8>(wParam));
            }
        }
        return 0;
    case WM_KEYUP:
        if (frameworkPtr)
        {
            frameworkPtr->KeyUp(static_cast<UINT8>(wParam));
        }
        return 0;
    case WM_SYSCHAR:
        {
            // The default window procedure will play a system notification sound when pressing the 
            // Alt+Enter keyboard combination if this message is not handled.
        }
        return 0;
    case WM_DESTROY:
        {
            PostQuitMessage(0);
        }
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}
