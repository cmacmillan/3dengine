// ShaderKind:Ui
// Texture:fontTexture slot=0
// BlendEnable:On
// SrcBlend:SrcAlpha
// DestBlend:InvSrcAlpha
// BlendOp:Add
// RtWriteMask:RGBA
// BlendOpAlpha:Add
// SrcBlendAlpha:SrcAlpha
// DestBlendAlpha:InvSrcAlpha
// DepthEnable: Off
// DepthWrite: Off
// DepthFunc: Always
// CullMode: Front ; BB wonky cull mode because in the 3d pipeline we invert when we go from the x->z dimension (flopping the winding order)
// END_INFO

cbuffer constants : register(b0)
{
    float2 posObject;
    float2 vecScaleObject;
    float4 uniformColor;
};

cbuffer globals : register(b1)
{
    float time;
    float2 vecWinSize;
    float padding;
    float4x4 matCameraToWorld;
    float4x4 matWorldToCamera;
	float4x4 matClipToWorld;
    float4x4 matWorldToClip;
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

float4 ps_main(VS_Output input) : SV_Target
{
    float2 vecPixel = float2(ddx(input.uv.x), ddy(input.uv.y));

    // that's change per pixel, so the next pixel u

    // https://blog.demofox.org/2015/04/23/4-rook-antialiasing-rgss/

    float uv0 = float2(-3.0f / 8.0f, 1.0f / 8.0f);
    float uv1 = float2(1.0f / 8.0f, 3.0f / 8.0f);
    float uv2 = float2(3.0f / 8.0f, -1.0f / 8.0f);
    float uv3 = float2(-1.0f / 8.0f, -3.0f / 8.0f);

    float gAlpha0 = fontTexture.Sample(mysampler, input.uv + uv0 * vecPixel).r;
    float gAlpha1 = fontTexture.Sample(mysampler, input.uv + uv1 * vecPixel).r;
    float gAlpha2 = fontTexture.Sample(mysampler, input.uv + uv2 * vecPixel).r;
    float gAlpha3 = fontTexture.Sample(mysampler, input.uv + uv3 * vecPixel).r;

    float gAlpha = (gAlpha0 + gAlpha1 + gAlpha2 + gAlpha3) / 4.0f;
    //gAlpha = 1.0f;
    return float4(input.color.rgb, input.color.a * gAlpha);
}