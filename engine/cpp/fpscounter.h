#pragma once

#include "engine.h"

struct SFpsCounter : SNode // fps
{
	typedef SNode super;
	SFpsCounter(SHandle<SNode> hNodeParent);
	~SFpsCounter();
	SHandle<SFpsCounter> HFps() { return (SHandle<SFpsCounter>) m_nHandle; }

	void Update() override;

	STextHandle		m_hText;

	float			m_adT[60];
	int				m_idT = 0;
	int				m_cdT = 0;
};

typedef SHandle<SFpsCounter> SFpsCounterHandle;