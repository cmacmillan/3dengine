// ShaderKind:3D
// Texture:mainTexture slot = 0
// Texture:sunShadowTexture slot = 1
// DepthEnable: On
// DepthWrite: On
// DepthFunc: Greater
// END_INFO

#pragma pack_matrix(row_major)

cbuffer constants : register(b0)
{
    float4x4 matMVP;
    float4x4 matObjectToWorld;
    float4x4 matObjectToWorldInverseTranspose;
};

cbuffer globals : register(b1)
{
    float time;
    float deltaTime;
    float2 vecWinSize;
    float4x4 matCameraToWorld;
    float4x4 matWorldToCamera;
	float4x4 matClipToWorld;
    float4x4 matWorldToClip;
    float4x4 matWorldToShadowClip;
	float4	normalSunDir;
	float	xClipNear;
	float	xClipFar;
	float2	vecPadding;
};

struct VS_Input {
    float4 pos : POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VS_Output {
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 posShadow : TEXCOORD2;
};

Texture2D    mainTexture : register(t0);
SamplerState mainSampler: register(s0);

Texture2D    sunShadowTexture : register(t1);
SamplerState sunShadowSampler : register(s1);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(input.pos, matMVP);
    output.posShadow = mul(mul(input.pos, matObjectToWorld), matWorldToShadowClip);
    output.uv = float2(input.uv.x, 1.0f - input.uv.y);

    // BB currently doing lighting in world space, probably should use camera space

    // NOTE apparently supposed to renormalize after inverse transpose multiplication
    output.normal = normalize(mul(input.normal, matObjectToWorldInverseTranspose));
    return output;
}

float maprange(float a1, float b1, float a2, float b2, float input)
{
    float lerp = (input - a1) / (b1 - a1);
    return a2 + (b2 - a2) * lerp;
}

float4 ps_main(VS_Output input) : SV_Target
{
    float light = dot(input.normal, normalSunDir.xyz);
    float gShadow = 0.0f;
    if (input.posShadow.w != 0.0f)
    {
        input.posShadow /= input.posShadow.w;
        float2 uvShadow = (input.posShadow.xy + 1.0) * 0.5;
        uvShadow.y = 1.0f - uvShadow.y;
        float gEpsilon = 0.0003f; // BB should be based on some world distance probably
        if (input.posShadow.z + gEpsilon <= 1.0f - sunShadowTexture.Sample(sunShadowSampler, uvShadow).x)
        {
            gShadow = 1.0f;
        }
    }

    float gLightAmbient = .1f;
    float3 color = max(gLightAmbient, (1.0f - gShadow) * light) * mainTexture.Sample(mainSampler, input.uv);
    return float4(color, 1.0);
}
