#pragma once

struct float2
{
	float2() : m_x(0.0f), m_y(0.0f) {}
	float2(float x, float y) : m_x(x), m_y(y) {}
	float m_x, m_y;
	float2 operator/(float g) const;
	float2 operator*(float g) const;
	float2 operator/(float2 vec) const;
	float2 operator*(float2 vec) const;
	float2 operator+(float2 vec) const;
	float2 operator-(float2 vec) const;
};

float2 operator*(float g, const float2 & vec);
float2 operator/(float g, const float2 & vec);

struct float4
{
	float4() : m_x(0.0f), m_y(0.0f), m_z(0.0f), m_w(0.0f) {}
	float4(float x, float y, float z, float w) : m_x(x), m_y(y), m_z(z), m_w(w) {}
	float m_x, m_y, m_z, m_w;
	float4 operator/(float g) const;
	float4 operator*(float g) const;
	float4 operator/(float4 vec) const;
	float4 operator*(float4 vec) const;
	float4 operator+(float4 vec) const;
	float4 operator-(float4 vec) const;
};

float4 operator*(float g, const float4 & vec);
float4 operator/(float g, const float4 & vec);

struct Transform
{
	float4 m_aVec[4];
};

