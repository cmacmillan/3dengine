// ShaderKind:Skybox
// Texture:skyTexture slot=0
// DepthEnable: Off
// DepthWrite: Off
// DepthFunc: Always
// CullMode: Back
// END_INFO

#include "util.hlsl"
#include "3dcommon.hlsl"
#include "globals.hlsl"

struct VS_Output {
    float4 pos : SV_POSITION;
    float3 uv : TEXCOORD;
};

Texture2D    skyTexture : register(t0);
SamplerState skySampler: register(s0);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(input.pos, matMVP);
    output.uv = mul(input.pos, matObjectToWorld).xyz - matCameraToWorld[3].xyz;

    // bunch of handy functions in here https://learn.microsoft.com/en-us/windows/win32/numerics_h/float4x4-structure

    //output.uv = input.uv;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    float PI = 3.1415926535;

    float3 normal = normalize(input.uv);

    // Project down onto x-y plane
    
    float gDotUp = dot(normal, float3(0, 0, 1));
    float3 normalXy = normalize(normal - gDotUp * float3(0, 0, 1));
    float3 vecCross = cross(float3(1.0, 0.0, 0.0), normalXy);
    float x = acos(dot(float3(1.0,0.0,0.0),normalXy)) / PI;
    if (dot(vecCross, float3(0.0, 0.0, 1.0)) < 0.0)
    {
        x = -x;
    }
    x = maprange(-1.0, 1.0, 0.0, 1.0, x);

    float y = maprange(-1.0,1.0,0.0,1.0f,gDotUp);

    // flip y (d3d sux)
    
    y = 1.0f - y;

    return skyTexture.Sample(skySampler, float2(x,y));
}

