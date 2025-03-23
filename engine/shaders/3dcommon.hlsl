#ifndef __3DCOMMON_HLSL__
#define __3DCOMMON_HLSL__

// NOTE apparently the convention for include guards is __FILENAME_EXTENSION__

cbuffer constants : register(b1)
{
    float4x4 matMVP;
    float4x4 matVP;
    float4x4 matV;
    float4x4 matObjectToWorld;
    float4x4 matObjectToWorldInverseTranspose;
    float4 rgba;
};

struct VS_Input {
    float4 pos : POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

#endif // end include guard