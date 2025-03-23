// ShaderKind:3D
// DepthEnable: On
// DepthFunc: Always
// DepthWrite: On ; writing while depthfunc always may mean we write a lower value into the depth buffer?
// CullMode: Front
// Shadowcast: Off
// BlendEnable: On
// BlendOp: Add
// SrcBlend: SrcAlpha
// DestBlend: InvSrcAlpha
// END_INFO

#include "util.hlsl"
#include "3dcommon.hlsl"
#include "globals.hlsl"

struct VS_Output {
    float4 pos : SV_POSITION;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    float4 posWorld = mul(input.pos, matObjectToWorld);
    float4 posCam = mul(posWorld, matV);
    float gScale = 2.0f;
    float gScalar = gScale * tan(radHFov * .5) * posCam.x / length(vecWinSize);
    output.pos = mul(posWorld + normalize(mul(input.normal, matObjectToWorldInverseTranspose))  * gScalar, matVP);
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return rgba;
}