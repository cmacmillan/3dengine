#pragma once

#include "engine.h"

struct STimingContext
{
	STimingContext(const char * pChzName, float dTDisplay = 0.0f);
	~STimingContext();

	LARGE_INTEGER	m_perfcountStart;
	std::string		m_strName;
	float			m_dTDisplay;
};
