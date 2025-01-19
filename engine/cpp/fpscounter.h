#pragma once

#include "object.h"
#include "engine.h"
#include "text.h"

struct SFpsCounter : SNode // fps
{
	typedef SNode super;
	SFpsCounter(SHandle<SNode> hNodeParent, const std::string & str);
	~SFpsCounter();
	SHandle<SFpsCounter> HFps() { return (SHandle<SFpsCounter>) m_nHandle; }

	void Update() override;

	STextHandle		m_hText;
	STextHandle		m_hTextFps;

	float			m_adT[4*120];
	int				m_idT = 0;
	int				m_cdT = 0;
	double			m_dTSyst = 0.0;
};

typedef SHandle<SFpsCounter> SFpsCounterHandle;