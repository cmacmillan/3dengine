// ShaderKind:3D
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
    float2 vecWinSize;
    float padding;
    float4x4 matCameraToWorld;
    float4x4 matWorldToCamera;
	float4x4 matClipToWorld;
    float4x4 matWorldToClip;
};

struct VS_Input {
    float4 pos : POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VS_Output {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(input.pos, matMVP);
    output.uv = float2(input.uv.x, 1.0f - input.uv.y);
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return float4(.6f,.6f,1.0f, 1.0);
}