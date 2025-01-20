#include "timingcontext.h"
#include "engine.h"

STimingContext::STimingContext(const char * pChzName, float dTDisplay)
{
	m_strName = pChzName;
	m_dTDisplay = dTDisplay;
	QueryPerformanceCounter(&m_perfcountStart);
}

STimingContext::~STimingContext()
{
	LARGE_INTEGER perfcount;
	QueryPerformanceCounter(&perfcount);

	float dT = float((double) (perfcount.QuadPart - m_perfcountStart.QuadPart) / (double) g_game.m_perfCounterFrequency);

	g_game.PrintConsole(StrPrintf("%s took %.1fms\n", m_strName.c_str(), 1000.0f * dT), m_dTDisplay);
}
