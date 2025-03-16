// ShaderKind:3D
// DepthEnable: On
// DepthFunc: Always
// DepthWrite: Off
// FillMode: Wireframe
// CullMode: None 
// Shadowcast: Off
// BlendEnable: On
// BlendOp: Add
// SrcBlend: SrcAlpha
// DestBlend: InvSrcAlpha
// END_INFO

#pragma pack_matrix(row_major)

cbuffer constants : register(b0)
{
    float4x4 matMVP;
    float4x4 matObjectToWorld;
    float4x4 matObjectToWorldInverseTranspose;
    float4 rgba;
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
