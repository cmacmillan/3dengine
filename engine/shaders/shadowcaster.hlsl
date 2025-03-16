// ShaderKind:3D
// DepthEnable: On
// DepthWrite: On
// DepthFunc: Greater
// END_INFO

#include "util.hlsl"
#include "3dcommon.hlsl"
#include "globals.hlsl"

struct VS_Output {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 posCam : TEXCOORD2;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(input.pos, matMVP);
    output.posCam = mul(mul(input.pos, matObjectToWorld), matWorldToCamera);
    output.uv = float2(input.uv.x, 1.0f - input.uv.y);
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    float uZ = maprange(xClipNear, xClipFar, 0.0f, 1.0f, input.posCam.x);
    return float4(uZ.xxx, 1.0);
}