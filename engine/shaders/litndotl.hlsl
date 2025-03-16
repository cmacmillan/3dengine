// ShaderKind:3D
// Texture:mainTexture slot = 0
// Texture:sunShadowTexture slot = 1
// DepthEnable: On
// DepthWrite: On
// DepthFunc: Greater
// END_INFO

#include "util.hlsl"
#include "3dcommon.hlsl"
#include "globals.hlsl"

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

float4 ps_main(VS_Output input) : SV_Target
{
    float light = dot(input.normal, normalSunDir.xyz);
    float gShadow = 0.0f;
    if (input.posShadow.w != 0.0f)
    {
        input.posShadow /= input.posShadow.w;
        float2 uvShadow = (input.posShadow.xy + 1.0) * 0.5;
        uvShadow.y = 1.0f - uvShadow.y;
        if (uvShadow.x >= 0.0 && uvShadow.y >= 0.0 && uvShadow.x <= 1.0 && uvShadow.y <= 1.0)
        {
            float gEpsilon = 0.0003f; // BB should be based on some world distance probably
            if (input.posShadow.z + gEpsilon <= 1.0f - sunShadowTexture.Sample(sunShadowSampler, uvShadow).x)
            {
                gShadow = 1.0f;
            }
        }
    }

    float gLightAmbient = .1f;
    float3 color = max(gLightAmbient, (1.0f - gShadow) * light) * mainTexture.Sample(mainSampler, input.uv);
    return float4(color, 1.0);
}
