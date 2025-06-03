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

ID3D11Buffer* DX_TriangleVxBuffer = nullptr;
ID3D11Buffer* DX_TriangleIxBuffer = nullptr;
ID3D11VertexShader* DX_ColorVxShader = nullptr;
ID3D11PixelShader* DX_ColorPxShader = nullptr;
ID3D11InputLayout* DX_ColorInputLayout = nullptr;

ID3D11Buffer* DX_QuadVxBuffer = nullptr;
ID3D11Buffer* DX_QuadIxBuffer = nullptr;
ID3D11VertexShader* DX_TextureVxShader = nullptr;
ID3D11PixelShader* DX_TexturePxShader = nullptr;
ID3D11InputLayout* DX_TextureInputLayout = nullptr;
ID3D11Texture2D* DebugTexture = nullptr;
ID3D11ShaderResourceView* DebugTexture_SRV = nullptr;
ID3D11SamplerState* DebugSamplerState = nullptr;

ID3D11Buffer* DX_WVPBuffer = nullptr;


int CompileShaderHelper
(
    LPCWSTR SourceFileName,
    LPCSTR EntryPointFunction,
    LPCSTR Profile,
    ID3DBlob * *ShaderBlob,
    const D3D_SHADER_MACRO * Defines
)
{
    HRESULT Result = S_OK;

    if (SourceFileName == nullptr || EntryPointFunction == nullptr || Profile == nullptr || ShaderBlob == nullptr)
    {
        return E_INVALIDARG;
    }

    *ShaderBlob = nullptr;

    UINT CompileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
    CompileFlags |= D3DCOMPILE_DEBUG;
    CompileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* OutBlob = nullptr;
    ID3DBlob* ErrorMsgBlob = nullptr;

    Result = D3DCompileFromFile
    (
        SourceFileName,
        Defines,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        EntryPointFunction,
        Profile,
        CompileFlags,
        0, //UINT Flags2
        &OutBlob,
        &ErrorMsgBlob
    );

    if (FAILED(Result) && OutBlob)
    {
        OutBlob->Release();
        OutBlob = nullptr;
    }
    if (ErrorMsgBlob)
    {
        OutputDebugStringA((char*)ErrorMsgBlob->GetBufferPointer());
        ErrorMsgBlob->Release();
    }

    *ShaderBlob = OutBlob;

    return Result;
};

VertexColor Vertices_Triangle[] =
{
    {{0.0f, 0.5f, 0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
    {{0.5f, -0.5f, 0.5f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
    {{-0.5f, -0.5f, 0.5f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}}
};
UINT Indices_Triangle[] =
{
    0, 2, 1
};

VertexTexture Vertices_Quad[]
{
    {{-0.5f, +0.5f, +0.5f, +1.0f}, {+0.0f, +0.0f}},
    {{+0.5f, +0.5f, +0.5f, +1.0f}, {+1.0f, +0.0f}},
    {{-0.5f, -0.5f, +0.5f, +1.0f}, {+0.0f, +1.0f}},
    {{+0.5f, -0.5f, +0.5f, +1.0f}, {+1.0f, +1.0f}},
};
UINT Indices_Quad[] =
{
    0, 2, 1,
    1, 2, 3
};

int InitGraphics()
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

    // Triangle Vx/Ix
    {
        D3D11_BUFFER_DESC VertexBufferDesc = { sizeof(Vertices_Triangle), D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0 };
        D3D11_SUBRESOURCE_DATA VertexBufferInitData = { Vertices_Triangle, 0, 0 };
        Result = DX_Device->CreateBuffer(&VertexBufferDesc, &VertexBufferInitData, &DX_TriangleVxBuffer);
        DXCHECK(Result);

        D3D11_BUFFER_DESC IndexBufferDesc = { sizeof(Indices_Triangle), D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, 0 };
        D3D11_SUBRESOURCE_DATA IndexBufferInitData = { Indices_Triangle, 0, 0 };
        Result = DX_Device->CreateBuffer(&IndexBufferDesc, &IndexBufferInitData, &DX_TriangleIxBuffer);
        DXCHECK(Result);
    }

    // VertexColor shaders
    {
        ID3DBlob* VSCodeBlob = nullptr;
        ID3DBlob* PSCodeBlob = nullptr;

        const D3D_SHADER_MACRO VxColorDefines[] =
        {
            "ENABLE_VERTEX_COLOR", "1",
            "ENABLE_VERTEX_TEXTURE", "0",
            NULL, NULL
        };
        Result = CompileShaderHelper(L"src/hlsl/BaseShader.hlsl", "VSMain", "vs_5_0", &VSCodeBlob, VxColorDefines);
        DXCHECKMSG(Result, "Failed to compile Vertex Shader! :(\n");

        Result = CompileShaderHelper(L"src/hlsl/BaseShader.hlsl", "PSMain", "ps_5_0", &PSCodeBlob, VxColorDefines);
        DXCHECKMSG(Result, "Failed to compile Pixel Shader! :(\n");

        if (VSCodeBlob && PSCodeBlob)
        {
            Result = DX_Device->CreateVertexShader(VSCodeBlob->GetBufferPointer(), VSCodeBlob->GetBufferSize(), nullptr, &DX_ColorVxShader);
            DXCHECKMSG(Result, "Device could not create vertex shader! :(\n");

            Result = DX_Device->CreatePixelShader(PSCodeBlob->GetBufferPointer(), PSCodeBlob->GetBufferSize(), nullptr, &DX_ColorPxShader);
            DXCHECKMSG(Result, "Device could not create pixel shader! :(\n");

            D3D11_INPUT_ELEMENT_DESC InputLayoutDesc[] =
            {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };
            UINT NumInputElements = ARRAYSIZE(InputLayoutDesc);

            Result = DX_Device->CreateInputLayout(InputLayoutDesc, NumInputElements, VSCodeBlob->GetBufferPointer(), VSCodeBlob->GetBufferSize(), &DX_ColorInputLayout);
            DXCHECKMSG(Result, "Device could not create input layout! :(\n");
        }
        if (VSCodeBlob) { VSCodeBlob->Release(); }
        if (PSCodeBlob) { PSCodeBlob->Release(); }
    }

    D3D11_BUFFER_DESC WVPBufferDesc = {};
    WVPBufferDesc.ByteWidth = sizeof(WVPData);
    WVPBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    WVPBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    WVPBufferDesc.CPUAccessFlags = 0;
    DXCHECK(DX_Device->CreateBuffer(&WVPBufferDesc, nullptr, &DX_WVPBuffer));

    // Quad Vx/Ix
    {
        D3D11_BUFFER_DESC QuadVxBufferDesc = { sizeof(Vertices_Quad), D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0 };
        D3D11_SUBRESOURCE_DATA QuadVxBufferInitData = { Vertices_Quad, 0, 0 };
        Result = DX_Device->CreateBuffer(&QuadVxBufferDesc, &QuadVxBufferInitData, &DX_QuadVxBuffer);
        DXCHECK(Result);

        D3D11_BUFFER_DESC QuadIxBufferDesc = { sizeof(Indices_Quad), D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, 0 };
        D3D11_SUBRESOURCE_DATA IndexBufferInitData = { Indices_Quad, 0, 0 };
        Result = DX_Device->CreateBuffer(&QuadIxBufferDesc, &IndexBufferInitData, &DX_QuadIxBuffer);
        DXCHECK(Result);
    }

    // DebugTexture
    {
        Image32 BMPImage = {};
        GetDebugBMP(BMPImage);

        D3D11_SUBRESOURCE_DATA DebugTexDataDesc[] = { {} };
        DebugTexDataDesc[0].pSysMem = BMPImage.PixelBuffer;
        DebugTexDataDesc[0].SysMemPitch = sizeof(u32) * BMPImage.Width;
        DebugTexDataDesc[0].SysMemSlicePitch = sizeof(u32) * BMPImage.Width * BMPImage.Height;
        D3D11_TEXTURE2D_DESC DebugTextureDesc = {};
        DebugTextureDesc.Width = BMPImage.Width;
        DebugTextureDesc.Height = BMPImage.Height;
        DebugTextureDesc.MipLevels = 1;
        DebugTextureDesc.ArraySize = 1;
        DebugTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        DebugTextureDesc.SampleDesc.Count = 1;
        DebugTextureDesc.SampleDesc.Quality = 0;
        DebugTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        DebugTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        DebugTextureDesc.CPUAccessFlags = 0;
        DebugTextureDesc.MiscFlags = 0;
        DXCHECK(DX_Device->CreateTexture2D(&DebugTextureDesc, &DebugTexDataDesc[0], &DebugTexture));
        DXCHECK(DX_Device->CreateShaderResourceView(DebugTexture, nullptr, &DebugTexture_SRV));

        D3D11_TEXTURE_ADDRESS_MODE AddressMode = D3D11_TEXTURE_ADDRESS_WRAP;
        D3D11_SAMPLER_DESC DebugTextureSamplerDesc = {};
        DebugTextureSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        DebugTextureSamplerDesc.AddressU = AddressMode;
        DebugTextureSamplerDesc.AddressV = AddressMode;
        DebugTextureSamplerDesc.AddressW = AddressMode;
        DebugTextureSamplerDesc.MipLODBias = 0.0f;
        DebugTextureSamplerDesc.MaxAnisotropy = 0;
        DebugTextureSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        DebugTextureSamplerDesc.MinLOD = 0;
        DebugTextureSamplerDesc.MaxLOD = 0;
        DXCHECK(DX_Device->CreateSamplerState(&DebugTextureSamplerDesc, &DebugSamplerState));

        delete[] BMPImage.PixelBuffer;
    }

    // VertexTexture shaders
    {
        ID3DBlob* VSCodeBlob = nullptr;
        ID3DBlob* PSCodeBlob = nullptr;

        const D3D_SHADER_MACRO VxTextureDefines[] =
        {
            "ENABLE_VERTEX_COLOR", "0",
            "ENABLE_VERTEX_TEXTURE", "1",
            NULL, NULL
        };
        Result = CompileShaderHelper(L"src/hlsl/BaseShader.hlsl", "VSMain", "vs_5_0", &VSCodeBlob, VxTextureDefines);
        DXCHECKMSG(Result, "Failed to compile Vertex Shader! :(\n");
        Result = CompileShaderHelper(L"src/hlsl/BaseShader.hlsl", "PSMain", "ps_5_0", &PSCodeBlob, VxTextureDefines);
        DXCHECKMSG(Result, "Failed to compile Pixel Shader! :(\n");

        if (VSCodeBlob && PSCodeBlob)
        {
            Result = DX_Device->CreateVertexShader(VSCodeBlob->GetBufferPointer(), VSCodeBlob->GetBufferSize(), nullptr, &DX_TextureVxShader);
            DXCHECKMSG(Result, "Device could not create vertex shader! :(\n");

            Result = DX_Device->CreatePixelShader(PSCodeBlob->GetBufferPointer(), PSCodeBlob->GetBufferSize(), nullptr, &DX_TexturePxShader);
            DXCHECKMSG(Result, "Device could not create pixel shader! :(\n");
            D3D11_INPUT_ELEMENT_DESC InputLayoutDesc[] =
            {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            };
            UINT NumInputElements = ARRAYSIZE(InputLayoutDesc);

            Result = DX_Device->CreateInputLayout(InputLayoutDesc, NumInputElements, VSCodeBlob->GetBufferPointer(), VSCodeBlob->GetBufferSize(), &DX_TextureInputLayout);
            DXCHECKMSG(Result, "Device could not create input layout! :(\n");
        }
        if (VSCodeBlob) { VSCodeBlob->Release(); }
        if (PSCodeBlob) { PSCodeBlob->Release(); }
    }

    return Result;
}

void UpdateAndDraw()
{
    UINT Offset = 0;
    m4f IdentityMatrix =
    {
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
    };
    WVPData WVP_Trans = { IdentityMatrix, IdentityMatrix, IdentityMatrix };
    constexpr int WVPBufferSlot = 0;
    DX_ImmediateContext->UpdateSubresource(DX_WVPBuffer, 0, nullptr, &WVP_Trans, sizeof(WVPData), 0);

    // Triangle
    {
        const UINT Stride = sizeof(VertexColor);
        DX_ImmediateContext->IASetInputLayout(DX_ColorInputLayout);
        DX_ImmediateContext->IASetVertexBuffers(0, 1, &DX_TriangleVxBuffer, &Stride, &Offset);
        DX_ImmediateContext->IASetIndexBuffer(DX_TriangleIxBuffer, DXGI_FORMAT_R32_UINT, 0);
        DX_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        DX_ImmediateContext->VSSetShader(DX_ColorVxShader, nullptr, 0);
        DX_ImmediateContext->PSSetShader(DX_ColorPxShader, nullptr, 0);

        DX_ImmediateContext->VSSetConstantBuffers(WVPBufferSlot, 1, &DX_WVPBuffer);

        DX_ImmediateContext->DrawIndexed(ARRAYSIZE(Indices_Triangle), 0u, 0u);
    }

    // Quad
    {
        const UINT Stride = sizeof(VertexTexture);
        DX_ImmediateContext->IASetInputLayout(DX_TextureInputLayout);
        DX_ImmediateContext->IASetVertexBuffers(0, 1, &DX_QuadVxBuffer, &Stride, &Offset);
        DX_ImmediateContext->IASetIndexBuffer(DX_QuadIxBuffer, DXGI_FORMAT_R32_UINT, 0);
        DX_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        DX_ImmediateContext->VSSetShader(DX_TextureVxShader, nullptr, 0);
        DX_ImmediateContext->PSSetShader(DX_TexturePxShader, nullptr, 0);

        DX_ImmediateContext->PSSetShaderResources(0, 1, &DebugTexture_SRV);
        DX_ImmediateContext->PSSetSamplers(0, 1, &DebugSamplerState);

        DX_ImmediateContext->VSSetConstantBuffers(WVPBufferSlot, 1, &DX_WVPBuffer);

        DX_ImmediateContext->DrawIndexed(ARRAYSIZE(Indices_Quad), 0u, 0u);
    }
}

void Draw()
{
    float ClearColor[4] = { 0.125f, 0.175f, 0.3f, 1.0f };
    float fDepth = 1.0f;
    DX_ImmediateContext->ClearRenderTargetView(DX_RenderTargetView, ClearColor);
    DX_ImmediateContext->ClearDepthStencilView(DX_DepthStencilView, D3D11_CLEAR_DEPTH, fDepth, 0);

    UpdateAndDraw();

    DX_SwapChain->Present(0, 0);
}

