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

    float4 rgbaOutline = float4(0.0, 0.0, 0.0, 1.);
    float4 rgbaText = float4(0.9, 0.9, 0.9, 1.);
    float outline = rgbaAvg.a;
    float glyph = rgbaAvg.r;
    float opacity = outline;
    float3 rgb = (outline * rgbaOutline.rgb) * (1.0f - glyph) + glyph * rgbaText.rgb;
    return float4(rgb, opacity);
}