// ShaderKind:3D
// DepthEnable: On
// DepthFunc: Always
// DepthWrite: Off
// CullMode: None 
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
    output.pos = mul(input.pos, matMVP);
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return rgba;
}