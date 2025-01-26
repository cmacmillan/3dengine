// ShaderKind:3D
//; Texture:mainTexture slot = 0
//; Texture:altTexture slot = 1
// CullMode:None
// DepthEnable: On
// DepthWrite: On
// DepthFunc: Greater
// END_INFO

#pragma pack_matrix(row_major)

cbuffer constants : register(b0)
{
    float4x4 matMVP;
    float4x4 matObjectToWorld;
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
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

/*
Texture2D    mainTexture : register(t0);
SamplerState mainSampler: register(s0);
Texture2D    altTexture : register(t1);
SamplerState altSampler: register(s1);
*/

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(input.pos, matMVP);
    output.uv = float2(input.uv.x, 1.0f - input.uv.y);
    // Idk what space we should do lighting in
    output.normal = mul(input.normal, matObjectToWorld); // Should be inverse transpose of this I think
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return float4(dot(input.normal.xyz, normalize(float3(-1, -.3, 1))).xxx, 1.0);
}
