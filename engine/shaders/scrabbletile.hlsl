// ShaderKind:Ui
// END_INFO

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
    float3 junk;
};

cbuffer scrabbletile : register(b2)
{
    float2 uvTopLeft;
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

Texture2D    texAlbedo : register(t0);
SamplerState samplerstateAlbedo : register(s0);

Texture2D    texNormal : register(t1);
SamplerState samplerstateNormal : register(s1);

VS_Output vs_main(VS_INPUT vsinput)
{
    VS_Output output;
    output.pos = float4((vecScaleObject * vsinput.pos + posObject - posCamera)/vecSizeCamera, 0.0f, 1.0f);
    output.color = uniformColor;
    output.uv = vsinput.uv * (1/8.0f) + uvTopLeft;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    // d3d is left handed, so z is forward
    float3 lightDir = normalize(float3(-1, -1, 1));
    //float3 lightDir = normalize(float3(cos(time), sin(time), .3f));
    float lightStrength = 1.5f;
    float3 normal = texNormal.Sample(samplerstateNormal, input.uv);
    normal.y = 1.0f - normal.y;
    normal.xy = (normal.xy * 2.0f) - float2(1.0f, 1.0f);
    normal.z = -normal.z; //sqrt(1.0f - saturate(dot(normal.xy, normal.xy)));

    // TODO Better shading model

    return lightStrength*saturate(dot(normal, -lightDir)) * texAlbedo.Sample(samplerstateAlbedo, input.uv) * input.color;
}