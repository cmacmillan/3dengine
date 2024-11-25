
cbuffer constants : register(b0)
{
    float2 posObject;
    float2 vecScaleObject;
    float4 uniformColor;
};

cbuffer globals : register(b1)
{
    float2 posCamera;
    float2 vecSizeCamera;
    float time;
};

struct VS_INPUT
{
    float2 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_Output {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEX;
};

Texture2D    mytexture : register(t0);
SamplerState mysampler : register(s0);

VS_Output vs_main(VS_INPUT vsinput)
{
    VS_Output output;
    output.pos = float4((vecScaleObject * vsinput.pos + posObject - posCamera)/vecSizeCamera, 0.0f, 1.0f);
    output.color = uniformColor;
    output.uv = vsinput.uv;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return mytexture.Sample(mysampler, input.uv) * input.color;
}