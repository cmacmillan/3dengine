#pragma once

struct SRgba
{
	SRgba(float r, float g, float b, float a) :
		m_r(r),
		m_g(g),
		m_b(b),
		m_a(a)
	{ }

	bool	operator==(const SRgba & rgbaOther) const;

	float m_r;
	float m_g;
	float m_b;
	float m_a;
};

extern SRgba g_rgbaRed;
extern SRgba g_rgbaBlue;
extern SRgba g_rgbaGreen;
extern SRgba g_rgbaYellow;
extern SRgba g_rgbaPink;
extern SRgba g_rgbaCyan;

void AuditRgba();
