#ifndef GRAPHICS_DX11_H
#define GRAPHICS_DX11_H

struct OxEd_Gfx_dx11
{
    static const char* Name() { return "GfxDX11"; }
    static void FrameBegin();
    static void FrameEnd();
    static bool Init();
    static bool Term();
    static void ImGui_Init();
    static void ImGui_Term();
    static void ImGui_NewFrame();
    static void ImGui_RenderDrawData(ImDrawData*);
};

using OxEd_GfxT = OxEd_Gfx_dx11;

// DirectX11:
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
// Dear ImGui:
#include "imgui/backends/imgui_impl_dx11.h"

#endif // GRAPHICS_DX11_H

