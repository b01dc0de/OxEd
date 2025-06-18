#ifndef GRAPHICS_DX11_H
#define GRAPHICS_DX11_H

#include "OxEd.h"

struct VertexColor
{
    v4f Position;
    v4f Color;
};

struct VertexTexture
{
    v4f Position;
    v2f TexUV;
};

struct WVPData
{
    m4f World;
    m4f View;
    m4f Proj;
};

int CompileShaderHelper(LPCWSTR SourceFileName, LPCSTR EntryPointFunction, LPCSTR Profile, ID3DBlob** ShaderBlob, const D3D_SHADER_MACRO* Defines = nullptr);

struct Graphics_DX11
{
    static int Init();
    static void Term();
    static void UpdateAndDraw();
};

#endif // GRAPHICS_DX11_H

