#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <shellapi.h> // contains the definition for the  CommandLineToArgvW function

// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#include <wrl.h> // used mostly for ComPtr
using namespace Microsoft::WRL;
// Since all DirectX 12 objects are COM objects, the ComPtr template class is used to track the COM object lifetimes.

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h> // used to manage the low-level tasks such as enumerating GPU adapters, presenting the rendered image to the screen, and handling full-screen 
// transitions, that are not necessarily part of the DirectX rendering API. DXGI 1.6 adds functionality in order to detect HDR displays
#include <d3dcompiler.h> // link against the D3Dcompiler_47.lib library and copy the D3dcompiler_47.dll to the same folder as the binary executable
#include <DirectXMath.h> // provides SIMD-friendly C++ types and functions for commonly used for graphics related programming

#include <cstdio>
#include <string>

#include "helper/d3dx12.h" // provides helper functions and structs to make certain common operations less verbose in your code.
#include "helper/dx12_utility.h"





#define CONF_BOOL_ENABLE_D3D_DEBUG_LAYER     (true)
#define CONF_BOOL_USE_WARP_DEVICE_IF_NO_HARDWARE_AVAILABLE      (true)

#define LOG printf