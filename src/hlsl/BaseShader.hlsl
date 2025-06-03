/////////////////////
// BaseShader.hlsl //
/////////////////////

#ifndef ENABLE_VERTEX_COLOR
#define ENABLE_VERTEX_COLOR (0)
#endif // ENABLE_VERTEX_COLOR
#ifndef ENABLE_VERTEX_TEXTURE
#define ENABLE_VERTEX_TEXTURE (0)
#endif // ENABLE_VERTEX_TEXTURE

#if ENABLE_VERTEX_TEXTURE
Texture2D MainTexture : register(t0);
SamplerState MainSampler : register(s0);
#endif // ENABLE_VERTEX_TEXTURE

cbuffer WorldViewProjBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Proj;
}

struct VS_INPUT
{
    float4 Pos : POSITION;
#if ENABLE_VERTEX_COLOR
    float4 RGBA : COLOR;
#endif // ENABLE_VERTEX_COLOR
#if ENABLE_VERTEX_TEXTURE
    float2 TexUV : TEXCOORD;
#endif // ENABLE_VERTEX_TEXTURE
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
#if ENABLE_VERTEX_COLOR
    float4 RGBA : COLOR;
#endif // ENABLE_VERTEX_COLOR
#if ENABLE_VERTEX_TEXTURE
    float2 TexUV : TEXCOORD;
#endif // ENABLE_VERTEX_TEXTURE
};

VS_OUTPUT VSMain(VS_INPUT Input)
{
    VS_OUTPUT Output;
#if 0
    Output.Pos = Input.Pos;
#else
    Output.Pos = mul(Input.Pos, World);
    Output.Pos = mul(Output.Pos, View);
    Output.Pos = mul(Output.Pos, Proj);
#endif // WVP TRANSFORM
#if ENABLE_VERTEX_COLOR
    Output.RGBA = Input.RGBA;
#endif // ENABLE_VERTEX_COLOR
#if ENABLE_VERTEX_TEXTURE
    Output.TexUV = Input.TexUV;
#endif // ENABLE_VERTEX_TEXTURE
    return Output;
}

float4 PSMain(VS_OUTPUT Input) : SV_Target
{
#if ENABLE_VERTEX_COLOR
    return Input.RGBA;
#endif // ENABLE_VERTEX_COLOR
#if ENABLE_VERTEX_TEXTURE
    return MainTexture.Sample(MainSampler, Input.TexUV);
#endif // ENABLE_VERTEX_TEXTURE
    return float4(0.0, 0.0, 0.0, 1.0);
}

