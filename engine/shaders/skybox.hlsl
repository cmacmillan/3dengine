// ShaderKind:Skybox
// Texture:skyTexture slot=0
// DepthEnable: Off
// DepthWrite: Off
// DepthFunc: Always
// CullMode: Back
// END_INFO

#pragma pack_matrix(row_major)

cbuffer constants : register(b0)
{
    float4x4 matMVP;
    float4x4 matObjectToWorld;
    float4x4 matObjecToWorldInverseTranspose;
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
    float3 uv : TEXCOORD;
};

Texture2D    skyTexture : register(t0);
SamplerState skySampler: register(s0);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(input.pos, matMVP);
    output.uv = mul(input.pos, matObjectToWorld).xyz - matCameraToWorld[3].xyz;

    // bunch of handy functions in here https://learn.microsoft.com/en-us/windows/win32/numerics_h/float4x4-structure

    //output.uv = input.uv;
    return output;
}

float maprange(float a1, float b1, float a2, float b2, float input)
{
    float lerp = (input - a1) / (b1 - a1);
    return a2 + (b2 - a2) * lerp;
}

float4 ps_main(VS_Output input) : SV_Target
{
    float PI = 3.1415926535;

    float3 normal = normalize(input.uv);

    // Project down onto x-y plane
    
    float gDotUp = dot(normal, float3(0, 0, 1));
    float3 normalXy = normalize(normal - gDotUp * float3(0, 0, 1));
    float3 vecCross = cross(float3(1.0, 0.0, 0.0), normalXy);
    float x = acos(dot(float3(1.0,0.0,0.0),normalXy)) / PI;
    if (dot(vecCross, float3(0.0, 0.0, 1.0)) < 0.0)
    {
        x = -x;
    }
    x = maprange(-1.0, 1.0, 0.0, 1.0, x);

    float y = maprange(-1.0,1.0,0.0,1.0f,gDotUp);

    // flip y (d3d sux)
    
    y = 1.0f - y;

    return skyTexture.Sample(skySampler, float2(x,y));
}

