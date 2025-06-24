#ifndef GRAPHICS_DX11_H
#define GRAPHICS_DX11_H

// DirectX11:
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

// Dear ImGui:
#include "imgui/backends/imgui_impl_dx11.h"

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

    static IDXGISwapChain* DX_SwapChain;
    static ID3D11Device* DX_Device;
    static D3D_FEATURE_LEVEL UsedFeatureLevel;
    static ID3D11DeviceContext* DX_ImmediateContext;

    static ID3D11Texture2D* DX_BackBuffer;
    static ID3D11RenderTargetView* DX_RenderTargetView;

    static IDXGIFactory1* DX_Factory;

    static ID3D11RasterizerState* DX_RasterizerState;
    static ID3D11Texture2D* DX_DepthStencil;
    static ID3D11DepthStencilView* DX_DepthStencilView;
};

using OxEd_GfxT = OxEd_Gfx_dx11;


#endif // GRAPHICS_DX11_H

