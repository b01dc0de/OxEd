#include "Graphics_DX11.h"
#include "Utils.h"

#define DXCHECK(Result) if (FAILED(Result)) { return -1; }
#define DXCHECKMSG(Result, Msg) if (FAILED(Result)) { OutputDebugStringA((Msg)); return -1; }

IDXGISwapChain* DX_SwapChain = nullptr;
ID3D11Device* DX_Device = nullptr;
D3D_FEATURE_LEVEL UsedFeatureLevel;
ID3D11DeviceContext* DX_ImmediateContext = nullptr;

ID3D11Texture2D* DX_BackBuffer = nullptr;
ID3D11RenderTargetView* DX_RenderTargetView = nullptr;

IDXGIFactory1* DX_Factory = nullptr;

ID3D11RasterizerState* DX_RasterizerState = nullptr;
ID3D11Texture2D* DX_DepthStencil = nullptr;
ID3D11DepthStencilView* DX_DepthStencilView = nullptr;
ID3D11BlendState* DX_BlendState = nullptr;

#define RGB_TO_FLOAT4(R, G, B) { float(R) / 255.0f, float(G) / 255.0f, float(B) / 255.0f, 1.0f }

void OxEd_ImGui_Draw()
{
}

void Graphics_DX11::UpdateAndDraw()
{
    float ClearColor[] = RGB_TO_FLOAT4(30, 30, 46);
    float fDepth = 1.0f;
    DX_ImmediateContext->ClearRenderTargetView(DX_RenderTargetView, ClearColor);
    DX_ImmediateContext->ClearDepthStencilView(DX_DepthStencilView, D3D11_CLEAR_DEPTH, fDepth, 0);

    { // ImGui: Frame begin
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    static bool bImGuiShowDemoWindow = true;
    if (bImGuiShowDemoWindow)
    {
        ImGui::ShowDemoWindow();
    }
    else
    {
        OxEd_ImGui_Draw();
    }

    { // IMGUI: Frame End
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    DX_SwapChain->Present(0, 0);
}


int Graphics_DX11::Init()
{
    HRESULT Result = S_OK;

    D3D_FEATURE_LEVEL SupportedFeatureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    UINT NumSupportedFeatureLevels = ARRAYSIZE(SupportedFeatureLevels);
    D3D_FEATURE_LEVEL D3DFeatureLevel = D3D_FEATURE_LEVEL_11_0;
    (void)D3DFeatureLevel;

    CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)&DX_Factory);

    DXGI_SAMPLE_DESC SharedSampleDesc = {};
    SharedSampleDesc.Count = 4;
    SharedSampleDesc.Quality = (UINT)D3D11_STANDARD_MULTISAMPLE_PATTERN;

    UINT FrameRefreshRate = 60;
    DXGI_SWAP_CHAIN_DESC swapchain_desc = {};
    swapchain_desc.BufferCount = 2;
    swapchain_desc.BufferDesc.Width = WinResX;
    swapchain_desc.BufferDesc.Height = WinResY;
    swapchain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchain_desc.BufferDesc.RefreshRate.Numerator = FrameRefreshRate;
    swapchain_desc.BufferDesc.RefreshRate.Denominator = 1;
    swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchain_desc.OutputWindow = hWindow;
    swapchain_desc.SampleDesc = SharedSampleDesc;
    swapchain_desc.Windowed = true;

    UINT CreateDeviceFlags = 0;
#ifdef _DEBUG
    CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    Result = D3D11CreateDeviceAndSwapChain(
        nullptr,					//IDXGIAdapter* pAdapter
        D3D_DRIVER_TYPE_HARDWARE,	//D3D_DRIVER_TYPE DriverType
        nullptr,					//HMODULE Software
        CreateDeviceFlags,			//UINT Flags
        SupportedFeatureLevels,		//const D3D_FEATURE_LEVEL* pFeatureLevels
        NumSupportedFeatureLevels,	//UINT FeatureLevels
        D3D11_SDK_VERSION,			//UINT SDKVersion
        &swapchain_desc,			//const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc
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

    D3D11_RENDER_TARGET_BLEND_DESC RTVBlendDesc = {};
    RTVBlendDesc.BlendEnable = true;
    RTVBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    RTVBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    RTVBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
    RTVBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
    RTVBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
    RTVBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    RTVBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALPHA;

    D3D11_BLEND_DESC BlendDesc = {};
    BlendDesc.RenderTarget[0] = RTVBlendDesc;

    Result = DX_Device->CreateBlendState(&BlendDesc, &DX_BlendState);
    DXCHECK(Result);

    D3D11_VIEWPORT Viewport_Desc = {};
    Viewport_Desc.Width = (FLOAT)WinResX;
    Viewport_Desc.Height = (FLOAT)WinResY;
    Viewport_Desc.MinDepth = 0.0f;
    Viewport_Desc.MaxDepth = 1.0f;
    Viewport_Desc.TopLeftX = 0;
    Viewport_Desc.TopLeftY = 0;
    DX_ImmediateContext->RSSetViewports(1, &Viewport_Desc);

    { // Imgui: Init

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        ImGui_ImplWin32_Init(hWindow);
        ImGui_ImplDX11_Init(DX_Device, DX_ImmediateContext);
    }

    return Result;
}

void Graphics_DX11::Term()
{
    { // ImGui: Shutdown
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}

