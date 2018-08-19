#include "stdafx.h"
#include "Framework_DX12.h"
#include "Win32Application.h"

namespace
{
    
}

Framework_DX12::Framework_DX12(UINT width, UINT height, bool useWarpDevice)
    : Framework(width, height)
    , m_useWarpDevice(useWarpDevice)
{
}

Framework_DX12::~Framework_DX12()
{
}

void Framework_DX12::Init()
{
    Win32Application::SetCustomWindowText(_T("Framework_DX12"));
}

void Framework_DX12::Update()
{
    
}

void Framework_DX12::Render()
{
    
}

void Framework_DX12::Destroy()
{
    
}