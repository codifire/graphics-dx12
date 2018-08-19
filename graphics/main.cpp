#include "stdafx.h"
#include "Win32Application.h"
#include "Framework_DX12.h"

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
    Framework_DX12 basic((UINT)1280, (UINT)720);
    return Win32Application::Run(&basic, hInstance, nCmdShow);
}