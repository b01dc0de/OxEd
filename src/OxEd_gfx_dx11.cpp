#include "OxEd.h"

IDXGISwapChain* OxEd_Gfx_dx11::DX_SwapChain = nullptr;
ID3D11Device* OxEd_Gfx_dx11::DX_Device = nullptr;
D3D_FEATURE_LEVEL OxEd_Gfx_dx11::UsedFeatureLevel;
ID3D11DeviceContext* OxEd_Gfx_dx11::DX_ImmediateContext = nullptr;
ID3D11Texture2D* OxEd_Gfx_dx11::DX_BackBuffer = nullptr;
ID3D11RenderTargetView* OxEd_Gfx_dx11::DX_RenderTargetView = nullptr;
ID3D11RasterizerState* OxEd_Gfx_dx11::DX_RasterizerState = nullptr;
ID3D11Texture2D* OxEd_Gfx_dx11::DX_DepthStencil = nullptr;
ID3D11DepthStencilView* OxEd_Gfx_dx11::DX_DepthStencilView = nullptr;

void OxEd_Gfx_dx11::FrameBegin()
{
    DX_ImmediateContext->OMSetRenderTargets(1, &DX_RenderTargetView, DX_DepthStencilView);
    constexpr float ClearColor[] = RGB_TO_FLOAT4(30, 30, 46);
    constexpr float fDepth = 1.0f;
    DX_ImmediateContext->ClearRenderTargetView(DX_RenderTargetView, ClearColor);
    DX_ImmediateContext->ClearDepthStencilView(DX_DepthStencilView, D3D11_CLEAR_DEPTH, fDepth, 0);
}

void OxEd_Gfx_dx11::FrameEnd()
{
    DX_SwapChain->Present(0, 0);
}

bool OxEd_Gfx_dx11::Init()
{
#define DXCHECK(Result) if (FAILED(Result)) { return false; }
#define DXCHECKMSG(Result, Msg) if (FAILED(Result)) { OutputDebugStringA((Msg)); return false; }

    HRESULT Result = S_OK;

    D3D_FEATURE_LEVEL SupportedFeatureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    UINT NumSupportedFeatureLevels = ARRAYSIZE(SupportedFeatureLevels);
    D3D_FEATURE_LEVEL D3DFeatureLevel = D3D_FEATURE_LEVEL_11_0;
    (void)D3DFeatureLevel;

    DXGI_SAMPLE_DESC SharedSampleDesc = {};
    SharedSampleDesc.Count = 1;
    SharedSampleDesc.Quality = 0;

    UINT FrameRefreshRate = 60;
    DXGI_SWAP_CHAIN_DESC swapchain_desc = {};
    swapchain_desc.BufferCount = 2;
    swapchain_desc.BufferDesc.Width = WinResX;
    swapchain_desc.BufferDesc.Height = WinResY;
    swapchain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchain_desc.BufferDesc.RefreshRate.Numerator = FrameRefreshRate;
    swapchain_desc.BufferDesc.RefreshRate.Denominator = 1;
    swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchain_desc.OutputWindow = OxEd_PlatformT::hWindow; // NOTE: must be Platform_win32
    swapchain_desc.SampleDesc = SharedSampleDesc;
    swapchain_desc.Windowed = true;
    swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    UINT CreateDeviceFlags = 0;
#if OXED_CONFIG_DEBUG()
    CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // OXED_CONFIG_DEBUG()

    Result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        CreateDeviceFlags,
        SupportedFeatureLevels,	
        NumSupportedFeatureLevels,
        D3D11_SDK_VERSION,
        &swapchain_desc,
        &DX_SwapChain,
        &DX_Device,
        &UsedFeatureLevel,
        &DX_ImmediateContext
    );
    DXCHECK(Result);

    Result = DX_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&DX_BackBuffer);
    DXCHECK(Result);

    Result = DX_Device->CreateRenderTargetView(DX_BackBuffer, nullptr, &DX_RenderTargetView);
    DXCHECK(Result);

    D3D11_RASTERIZER_DESC RasterDesc = {};
    RasterDesc.FillMode = D3D11_FILL_SOLID;
    RasterDesc.CullMode = D3D11_CULL_BACK;
    RasterDesc.FrontCounterClockwise = true;
    RasterDesc.DepthClipEnable = true;
    RasterDesc.ScissorEnable = false;
    RasterDesc.MultisampleEnable = true;
    RasterDesc.AntialiasedLineEnable = true;

    Result = DX_Device->CreateRasterizerState(&RasterDesc, &DX_RasterizerState);
    DXCHECK(Result);

    DX_ImmediateContext->RSSetState(DX_RasterizerState);

    D3D11_TEXTURE2D_DESC DepthDesc = {};
    DepthDesc.Width = WinResX;
    DepthDesc.Height = WinResY;
    DepthDesc.MipLevels = 1;
    DepthDesc.ArraySize = 1;
    DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthDesc.SampleDesc = SharedSampleDesc;
    DepthDesc.Usage = D3D11_USAGE_DEFAULT;
    DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    DepthDesc.CPUAccessFlags = 0;
    DepthDesc.MiscFlags = 0;

    Result = DX_Device->CreateTexture2D(&DepthDesc, nullptr, &DX_DepthStencil);
    DXCHECK(Result);

    D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {};
    DepthStencilViewDesc.Format = DepthStencilViewDesc.Format;
    DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    DepthStencilViewDesc.Texture2D.MipSlice = 0;

    Result = DX_Device->CreateDepthStencilView(DX_DepthStencil, &DepthStencilViewDesc, &DX_DepthStencilView);
    DXCHECK(Result);

    DX_ImmediateContext->OMSetRenderTargets(1, &DX_RenderTargetView, DX_DepthStencilView);

    D3D11_VIEWPORT Viewport_Desc = {};
    Viewport_Desc.Width = (FLOAT)WinResX;
    Viewport_Desc.Height = (FLOAT)WinResY;
    Viewport_Desc.MinDepth = 0.0f;
    Viewport_Desc.MaxDepth = 1.0f;
    Viewport_Desc.TopLeftX = 0;
    Viewport_Desc.TopLeftY = 0;
    DX_ImmediateContext->RSSetViewports(1, &Viewport_Desc);

    return !FAILED(Result);
}

bool OxEd_Gfx_dx11::Term()
{
#define SAFE_RELEASE(Ptr) if (Ptr) {Ptr->Release();}

    SAFE_RELEASE(DX_BackBuffer);
    SAFE_RELEASE(DX_RenderTargetView);
    SAFE_RELEASE(DX_RasterizerState);
    SAFE_RELEASE(DX_DepthStencil);
    SAFE_RELEASE(DX_DepthStencilView);


    SAFE_RELEASE(DX_SwapChain);
    SAFE_RELEASE(DX_ImmediateContext);
    SAFE_RELEASE(DX_Device);

    return true;
}

void OxEd_Gfx_dx11::ImGui_Init()
{
    ImGui_ImplDX11_Init(DX_Device, DX_ImmediateContext);
}

void OxEd_Gfx_dx11::ImGui_Term()
{
    ImGui_ImplDX11_Shutdown();
}

void OxEd_Gfx_dx11::ImGui_NewFrame()
{
    ImGui_ImplDX11_NewFrame();
}

void OxEd_Gfx_dx11::ImGui_RenderDrawData(ImDrawData* _ImDrawData)
{
    ImGui_ImplDX11_RenderDrawData(_ImDrawData);
}
