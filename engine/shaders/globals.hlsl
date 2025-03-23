#ifndef __GLOBALS_HLSL__
#define __GLOBALS_HLSL__

cbuffer globals : register(b0)
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
    float   radHFov;
	float	padding;
};

#endif // end include guard