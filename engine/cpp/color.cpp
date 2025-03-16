#include "color.h"
#include "util.h"

void AuditRgba()
{
}

bool SRgba::operator==(const SRgba & rgbaOther) const
{
	return m_r == rgbaOther.m_r &&
			m_b == rgbaOther.m_g &&
			m_g == rgbaOther.m_b &&
			m_a == rgbaOther.m_a;
}

SRgba g_rgbaRed = SRgba(1.0f, 0.0f, 0.0f, 1.0f);
SRgba g_rgbaGreen = SRgba(0.0f, 1.0f, 0.0f, 1.0f);
SRgba g_rgbaBlue = SRgba(0.0f, 0.0f, 1.0f, 1.0f);
SRgba g_rgbaYellow = SRgba(0.0f, 1.0f, 1.0f, 1.0f);
SRgba g_rgbaPink = SRgba(1.0f, 0.0f, 1.0f, 1.0f);
SRgba g_rgbaCyan = SRgba(0.0f, 1.0f, 1.0f, 1.0f);
