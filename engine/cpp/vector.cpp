
#include "vector.h"

// float 2

float2 float2::operator/(float g) const
{
	return float2(m_x / g, m_y / g);
}

float2 float2::operator*(float g) const
{
	return float2(m_x * g, m_y * g);
}

float2 float2::operator/(float2 vec) const
{
	return float2(m_x / vec.m_x, m_y / vec.m_y);
}

float2 float2::operator*(float2 vec) const
{
	return float2(m_x * vec.m_x, m_y * vec.m_y);
}

float2 float2::operator+(float2 vec) const
{
	return float2(m_x + vec.m_x, m_y + vec.m_y);
}

float2 float2::operator-(float2 vec) const
{
	return float2(m_x - vec.m_x, m_y - vec.m_y);
}

float2 operator*(float g, const float2& vec) {
	return vec * g;
}

float2 operator/(float g, const float2 & vec)
{
	return float2(g / vec.m_x,  g / vec.m_y);
}

// float4

float4 float4::operator/(float g) const
{
	return float4(m_x / g, m_y / g, m_z / g, m_w / g);
}

float4 float4::operator*(float g) const
{
	return float4(m_x * g, m_y * g, m_z * g, m_w * g);
}

float4 float4::operator/(float4 vec) const
{
	return float4(m_x / vec.m_x, m_y / vec.m_y, m_z / vec.m_z, m_w / vec.m_w);
}

float4 float4::operator*(float4 vec) const
{
	return float4(m_x * vec.m_x, m_y * vec.m_y, m_z * vec.m_z, m_w * vec.m_w);
}

float4 float4::operator+(float4 vec) const
{
	return float4(m_x + vec.m_x, m_y + vec.m_y, m_z + vec.m_z, m_w + vec.m_w);
}

float4 float4::operator-(float4 vec) const
{
	return float4(m_x - vec.m_x, m_y - vec.m_y, m_z - vec.m_z, m_w - vec.m_w);
}

float4 operator*(float g, const float4& vec) {
	return vec * g;
}

float4 operator/(float g, const float4 & vec)
{
	return float4(g / vec.m_x,  g / vec.m_y, g / vec.m_z, g / vec.m_w);
}