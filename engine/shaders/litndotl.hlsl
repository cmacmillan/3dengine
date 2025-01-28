// ShaderKind:3D
// Texture:mainTexture slot = 0
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
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

Texture2D    mainTexture : register(t0);
SamplerState mainSampler: register(s0);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(input.pos, matMVP);
    output.uv = float2(input.uv.x, 1.0f - input.uv.y);

    // BB currently doing lighting in world space, probably should use camera space

    // NOTE apparently supposed to renormalize after inverse transpose multiplication
    output.normal = normalize(mul(input.normal, matObjectToWorldInverseTranspose));
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    float light = dot(input.normal, normalize(float3(-1, -.3, 1)));
    float3 color = light * mainTexture.Sample(mainSampler, input.uv);
    //float3 color = light.xxx; // temp
    return float4(color, 1.0);
}
