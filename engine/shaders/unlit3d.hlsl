// ShaderKind:3D
// Texture:mainTexture slot = 0
// Texture:altTexture slot = 1
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
    float4x4 matObjecToWorldInverseTranspose;
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
    float2 uv : TEXCOORD;
};

Texture2D    mainTexture : register(t0);
SamplerState mainSampler: register(s0);
Texture2D    altTexture : register(t1);
SamplerState altSampler: register(s1);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(input.pos, matMVP);
    output.uv = float2(input.uv.x, 1.0f - input.uv.y);
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    float4 vecMain = mainTexture.Sample(mainSampler, input.uv);   
    float4 vecAlt = altTexture.Sample(mainSampler, input.uv);
    float alpha = vecAlt.a * (sin(time) + 1) * .5f;
    return float4(vecMain.rgb * (1.0 - alpha) + alpha * vecAlt.rgb, 1.0);
}