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
    float time;
    float2 vecWinSize;
    float padding;
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
    output.pos = float4((vecScaleObject * vsinput.pos + posObject) / vecWinSize - 1.0f, 0.0f, 1.0f);
    output.color = uniformColor;
    output.uv = vsinput.uv;
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

    // How do we mipmap normal maps?
    // TODO Better shading model
    return lightStrength*saturate(dot(normal, -lightDir)) * texAlbedo.Sample(samplerstateAlbedo, input.uv) * input.color;
    //return texAlbedo.Sample(samplerstateAlbedo, input.uv) * input.color;
    //return lightStrength*saturate(dot(normal, -lightDir)) * texAlbedo.SampleLevel(samplerstateAlbedo, input.uv, 10.f) * input.color;

    //return texAlbedo.SampleLevel(samplerstateAlbedo, input.uv, (sin(time) + 1) * 6.0);

    /*
    float uv0 = float2(-3.0f / 8.0f, 1.0f / 8.0f);
    float uv1 = float2(1.0f / 8.0f, 3.0f / 8.0f);
    float uv2 = float2(3.0f / 8.0f, -1.0f / 8.0f);
    float uv3 = float2(-1.0f / 8.0f, -3.0f / 8.0f);

    float2 vecPixel = float2(ddx(input.uv.x), ddy(input.uv.y));

    float power = 2.2f;
    float4 colorAlbedo0 = pow(texAlbedo.Sample(samplerstateAlbedo, input.uv + uv0 * vecPixel), power);
    float4 colorAlbedo1 = pow(texAlbedo.Sample(samplerstateAlbedo, input.uv + uv1 * vecPixel), power);
    float4 colorAlbedo2 = pow(texAlbedo.Sample(samplerstateAlbedo, input.uv + uv2 * vecPixel), power);
    float4 colorAlbedo3 = pow(texAlbedo.Sample(samplerstateAlbedo, input.uv + uv3 * vecPixel), power);

    float4 colorAlbedo = pow((colorAlbedo0 + colorAlbedo0 + colorAlbedo0 + colorAlbedo0) / 4.0f, 1./power); //texAlbedo.Sample(samplerstateAlbedo, input.uv);

       return lightStrength*saturate(dot(normal, -lightDir)) * colorAlbedo * input.color;
    */
}