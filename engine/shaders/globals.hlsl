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
	float2	vecPadding;
};