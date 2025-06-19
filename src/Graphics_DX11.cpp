#include "Graphics_DX11.h"
#include "Utils.h"

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
}
using namespace Graphics_DX11_State;

#define RGB_TO_FLOAT4(R, G, B) { float(R) / 255.0f, float(G) / 255.0f, float(B) / 255.0f, 1.0f }

struct HexFile 
{
    const char* FileName = nullptr;

    size_t FileSize = 0;
    u8* FileContents = nullptr;

    size_t HexTextSize = 0;
    char* HexText = nullptr;

    void Release();
    void SetFile(FileContentsT& NewFile);
};

void HexFile::Release()
{
    if (FileName) { delete[] FileName; }
    if (FileContents) { delete[] FileContents; }
    if (HexText) { delete[] HexText; }

    *this = {};
}

void HexFile::SetFile(FileContentsT& NewFile)
{
    Release();

    FileName = NewFile.Name;
    FileSize = NewFile.Size;
    FileContents = NewFile.Contents;

    // Create hex text representation of file
    {
        HexTextSize = (FileSize * 3) + 1;
        HexText = new char[HexTextSize];
        size_t WriteIdx = 0;
        for (size_t ByteIdx = 0; ByteIdx < FileSize; ByteIdx++)
        {
            HexText[WriteIdx++] = GetHighHex(FileContents[ByteIdx]);
            HexText[WriteIdx++] = GetLowHex(FileContents[ByteIdx]);
            HexText[WriteIdx++] = ' ';
        }
        HexText[WriteIdx++] = '\0';
        ASSERT(WriteIdx == HexTextSize);
    }

}

namespace OxEd_State
{
    HexFile ActiveFile;
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
        FileContentsT NewActiveFile = {};
        ReadFileContents(FileNameBuffer, NewActiveFile);
        ActiveFile.SetFile(NewActiveFile);
    }
    else
    { // User canceled dialog, or other error occured
        //DWORD ExError = CommDlgExtendedError();
        //DebugBreak();
    }
}

void OxEd_ImGui_DrawMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open"))
            {
                OxEd_Win32OpenFileDialog();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void OxEd_ImGui_DrawActiveFile()
{
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    constexpr int BytesPerLine = 32;
    constexpr int LineNumberWidth = 1;
    constexpr int LineWidth = (BytesPerLine * 3) + 1 + (LineNumberWidth + 1);

    constexpr static int StartLine = 0;
    int NumLines = ActiveFile.FileSize / BytesPerLine + (ActiveFile.FileSize % BytesPerLine == 0 ?  0 : 1);

    ImVec4 ForegroundColor(198.0f/255.0f, 166.0f/255.0f, 247.0f/255.0f, 1.0f);

    if (ImGui::Begin("OxEd_ImGui_DrawActiveFile", nullptr, flags))
    {
        ImGui::Text("%s", ActiveFile.FileName);
        ImGui::BeginChild("ActiveFile_Contents");
        ImGui::PushStyleColor(ImGuiCol_Text, ForegroundColor);
        for (int LineIdx = 0; LineIdx < NumLines; LineIdx++)
        {
            size_t BeginIdx = LineWidth * LineIdx;
            size_t EndIdx = Clamp((size_t)(BeginIdx + LineWidth - 1), (size_t)0, ActiveFile.HexTextSize - 1);
            const char* LineBegin = ActiveFile.HexText + BeginIdx;
            const char* LineEnd = ActiveFile.HexText + EndIdx;
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
    if (ActiveFile.FileContents)
    {
        OxEd_ImGui_DrawActiveFile();
    }
}

void Graphics_DX11::UpdateAndDraw()
{
    DX_ImmediateContext->OMSetRenderTargets(1, &DX_RenderTargetView, DX_DepthStencilView);
    constexpr float ClearColor[] = RGB_TO_FLOAT4(30, 30, 46);
    constexpr float fDepth = 1.0f;
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
#define DXCHECK(Result) if (FAILED(Result)) { return -1; }
#define DXCHECKMSG(Result, Msg) if (FAILED(Result)) { OutputDebugStringA((Msg)); return -1; }

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
    swapchain_desc.OutputWindow = hWindow;
    swapchain_desc.SampleDesc = SharedSampleDesc;
    swapchain_desc.Windowed = true;
    swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    UINT CreateDeviceFlags = 0;
#ifdef _DEBUG
    CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

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

    { // Dear Imgui: Init
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.IniFilename = nullptr;
        io.FontGlobalScale = 2.0f;

        ImGui_ImplWin32_Init(hWindow);
        ImGui_ImplDX11_Init(DX_Device, DX_ImmediateContext);
    }

    return Result;
}

void Graphics_DX11::Term()
{
    ActiveFile.Release();

    { // Dear ImGui: Shutdown
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

#define SAFE_RELEASE(Ptr) if (Ptr) {Ptr->Release();}

    SAFE_RELEASE(DX_BackBuffer);
    SAFE_RELEASE(DX_RenderTargetView);
    SAFE_RELEASE(DX_RasterizerState);
    SAFE_RELEASE(DX_DepthStencil);
    SAFE_RELEASE(DX_DepthStencilView);


    SAFE_RELEASE(DX_Factory);
    SAFE_RELEASE(DX_SwapChain);
    SAFE_RELEASE(DX_ImmediateContext);
    SAFE_RELEASE(DX_Device);
}

