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
// END_INFO

#include "text2dcommon.hlsl"

float4 ps_main(VS_Output input) : SV_Target
{
    float4 rgbaAvg = rgbaAvgSample(input);

    return float4(input.color.rgb, input.color.a * rgbaAvg.r);
}