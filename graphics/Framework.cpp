#include "stdafx.h"
#include "Win32Application.h"

Framework::Framework(UINT width, UINT height)
    : m_width(width)
    , m_height(height)
    , m_aspectRatio(static_cast<float>(width) / static_cast<float>(height))
{
}

// Helper function for parsing any supplied command line args.
_Use_decl_annotations_
void Framework::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
}
