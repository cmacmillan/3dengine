// ShaderKind:Skybox
// Texture:skyTexture slot=0
// DepthEnable: Off
// DepthWrite: Off
// DepthFunc: Always
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
    float2 uv : TEXCOORD;
};

struct VS_Output {
    float4 pos : SV_POSITION;
    float3 uv : TEXCOORD;
};

Texture2D    skyTexture : register(t0);
SamplerState skySampler: register(s0);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(input.pos, matMVP);
    //output.uv = 
    //output.uv = input.uv;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return float4(1, 1, 1, 1);
    /*
    float4 vecMain =
    float4 vecAlt = altTexture.Sample(mainSampler, input.uv);
    float alpha = vecAlt.a * (sin(time) + 1) * .5f;
    return float4(vecMain.rgb * (1.0 - alpha) + alpha * vecAlt.rgb, 1.0);
    */
}

