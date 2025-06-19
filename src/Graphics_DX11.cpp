#include "Graphics_DX11.h"
#include "Utils.h"

#define DXCHECK(Result) if (FAILED(Result)) { return -1; }
#define DXCHECKMSG(Result, Msg) if (FAILED(Result)) { OutputDebugStringA((Msg)); return -1; }

namespace Graphics_DX11_State
{
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
}
using namespace Graphics_DX11_State;

#define RGB_TO_FLOAT4(R, G, B) { float(R) / 255.0f, float(G) / 255.0f, float(B) / 255.0f, 1.0f }

namespace OxEd_State
{
    FileContentsT ActiveFile = {};
}
using namespace OxEd_State;

void OxEd_Win32OpenFileDialog()
{
    char* FileNameBuffer = new char[MAX_PATH]{};

    OPENFILENAMEA DialogState = {};
    DialogState.lStructSize = sizeof(OPENFILENAMEA);
    DialogState.hwndOwner = hWindow;
    DialogState.hInstance = nullptr;
    DialogState.lpstrFilter = nullptr;
    DialogState.lpstrCustomFilter = nullptr;
    DialogState.nFilterIndex = 0;
    DialogState.lpstrFile = FileNameBuffer;
    DialogState.nMaxFile = MAX_PATH;
    DialogState.lpstrFileTitle = nullptr;
    DialogState.lpstrInitialDir = nullptr;
    DialogState.lpstrTitle = nullptr;
    DialogState.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
    DialogState.lpstrDefExt = nullptr;
    DialogState.lCustData = 0;
    DialogState.lpfnHook = nullptr;
    BOOL bResult = GetOpenFileNameA(&DialogState);
    if (bResult)
    { // User hit 'OK'
        Release(ActiveFile);
        ReadFileContents(FileNameBuffer, ActiveFile);
    }
    else
    { // User canceled dialog, or other error occured
        DWORD ExError = CommDlgExtendedError();
        DebugBreak();
    }
}

void OxEd_ImGui_DrawMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", "Ctrl+O"))
            {
                OxEd_Win32OpenFileDialog();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

char Hack_GetHex(u8 Value)
{
    if (0 <= Value && Value <= 0xF)
    {
        if (Value <= 9)
        {
            return Value + 0x30;
        }
        else
        {
            return Value + 0x41;
        }
    }
}

char Hack_GetHighHex(u8 Value)
{
    return Hack_GetHex((Value & 0xF0) >> 4);
}

char Hack_GetLowHex(u8 Value)
{
    return Hack_GetHex(Value & 0x0F);
}

void OxEd_ImGui_DrawActiveFile()
{
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    constexpr int BytesPerLine = 32;
    constexpr int NumLines = 8;
    constexpr int LineNumberWidth = 1;
    constexpr int LineWidth = (BytesPerLine * 3) + 1 + (LineNumberWidth + 1);
    constexpr int HexBufferSize = LineWidth * NumLines + 1;

    static int StartLine = 0;

    char TextHex[HexBufferSize] = {};
    { // Fill hex buffer
        if (ActiveFile.Contents)
        {
            int WriteIdx = 0;
            for (int LineIdx = 0; LineIdx < NumLines; LineIdx++)
            {
                int DummyLineNumber = (LineIdx + StartLine) % 10;
                TextHex[WriteIdx++] = DummyLineNumber + 0x30;
                TextHex[WriteIdx++] = ' ';
                for (int ByteIdx = 0; ByteIdx < BytesPerLine; ByteIdx++)
                {
                    int ByteIdxInFile = (LineIdx + StartLine) * BytesPerLine + ByteIdx;
                    TextHex[WriteIdx++] = Hack_GetHighHex(ActiveFile.Contents[ByteIdxInFile]);
                    TextHex[WriteIdx++] = Hack_GetLowHex(ActiveFile.Contents[ByteIdxInFile]);
                    TextHex[WriteIdx++] = ' ';
                }
                TextHex[WriteIdx++] = '\n';
            }
            TextHex[WriteIdx++] = '\0';
        }
    }

    ImVec4 FG_Color(198.0f/255.0f, 166.0f/255.0f, 247.0f/255.0f, 1.0f);
    if (ImGui::Begin("OxEd_ImGui_DrawActiveFile", nullptr, flags))
    {
        ImGui::Text("%s", ActiveFile.Name);
        ImGui::BeginChild("ActiveFile_Contents");
        ImGui::PushStyleColor(ImGuiCol_Text, FG_Color);
        for (int LineIdx = 0; LineIdx < NumLines; LineIdx++)
        {
            const char* LineBegin = TextHex + (LineWidth * LineIdx);
            const char* LineEnd = TextHex + (LineWidth * LineIdx) + LineWidth;
            ImGui::TextUnformatted(LineBegin, LineEnd);
        }
        ImGui::PopStyleColor();

        ImGui::EndChild();
    }
    ImGui::End();
}

void OxEd_ImGui_Draw()
{
    OxEd_ImGui_DrawMenuBar();
    if (ActiveFile.Contents)
    {
        OxEd_ImGui_DrawActiveFile();
    }
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

    OxEd_ImGui_Draw();
    static bool bImGuiShowDemoWindow = true;
    if (bImGuiShowDemoWindow)
    {
        ImGui::ShowDemoWindow();
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
        io.FontGlobalScale = 2.0f;

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

