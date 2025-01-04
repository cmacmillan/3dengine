#pragma pack_matrix(row_major)

cbuffer constants : register(b0)
{
    float4x4 matMVP;
};

cbuffer globals : register(b1)
{
    float time;
    float2 vecWinSize;
    float padding;
};

struct VS_Input {
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_Output {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(float4(input.pos, 1.0f), matMVP);
    output.uv = input.uv; // BB
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return float4(input.uv.xy, 0.0f, 1.0f);
    //return mytexture.Sample(mysampler, input.uv);   
}