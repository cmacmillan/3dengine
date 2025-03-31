#include "util.hlsl"
#include "globals.hlsl"

cbuffer constants : register(b1)
{
    float2 posObject;
    float2 vecScaleObject;
    float4 uniformColor;
};

struct VS_INPUT
{
    float4 pos : POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VS_Output {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEX;
};

Texture2D    fontTexture : register(t0);
SamplerState mysampler : register(s0);

VS_Output vs_main(VS_INPUT vsinput)
{
    VS_Output output;
    output.pos = float4((vecScaleObject * vsinput.pos.xy + posObject) / vecWinSize - 1.0f, 0.0f, 1.0f);
    output.color = uniformColor;
    output.uv = vsinput.uv;
    return output;
}

float4 rgbaAvgSample(VS_Output input)
{
    float2 vecPixel = float2(ddx(input.uv.x), ddy(input.uv.y));

    // that's change per pixel, so the next pixel u

    // https://blog.demofox.org/2015/04/23/4-rook-antialiasing-rgss/

    float uv0 = float2(-3.0f / 8.0f, 1.0f / 8.0f);
    float uv1 = float2(1.0f / 8.0f, 3.0f / 8.0f);
    float uv2 = float2(3.0f / 8.0f, -1.0f / 8.0f);
    float uv3 = float2(-1.0f / 8.0f, -3.0f / 8.0f);

    float4 rgba0 = fontTexture.Sample(mysampler, input.uv + uv0 * vecPixel);
    float4 rgba1 = fontTexture.Sample(mysampler, input.uv + uv1 * vecPixel);
    float4 rgba2 = fontTexture.Sample(mysampler, input.uv + uv2 * vecPixel);
    float4 rgba3 = fontTexture.Sample(mysampler, input.uv + uv3 * vecPixel);   

    return (rgba0 + rgba1 + rgba2 + rgba3) / 4.0f;
}