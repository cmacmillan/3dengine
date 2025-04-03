#pragma once

#include "object.h"
#include "engine.h"
#include "text.h"

struct SFpsCounter : SNode // fps
{
	typedef SNode super;
	SFpsCounter(SNode * pNodeParent, const std::string & str);
	~SFpsCounter();
	SHandle<SFpsCounter> HFps() { return (SHandle<SFpsCounter>) m_h; }

	void Update() override;

	STextHandle		m_hText;

	float			m_adTRealtime[4*120];
	int				m_idT = 0;
	int				m_cdT = 0;
	double			m_systRealtime = 0.0;
};

typedef SHandle<SFpsCounter> SFpsCounterHandle;