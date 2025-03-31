
#include "fpscounter.h"
#include "util.h"

SFpsCounter::SFpsCounter(SNode * pNodeParent, const std::string & str) : super(pNodeParent, str)
{
	m_hText = (new SText(g_game.m_hFont, this, "FpsTextMain"))->HText();
	m_hText->m_hMaterial = g_game.m_hMaterialText;
	m_hText->SetText("FPS: ___");
	m_hText->m_vecScale = float2(0.3f, 0.3f);
	m_hText->m_gSort = 10.0f;
	m_hText->m_pos = float2(0.0f, 0.0f);
	m_hText->m_color = { 0.0f, 0.0f, 0.0f, 1.0f };
}

SFpsCounter::~SFpsCounter()
{
	delete m_hText.PT();
}

void SFpsCounter::Update()
{
	super::Update();

	float2 vecWinSize = g_game.VecWinSize();

	m_cdT = NMin(++m_cdT, DIM(m_adTRealtime));
	m_idT = (m_idT + 1) % m_cdT;
	m_adTRealtime[m_idT] = g_game.m_systRealtime - m_systRealtime;
	m_systRealtime = g_game.m_systRealtime;

	float gdTAvg = 0.0f;
	float dTWorst = 0.0f;
	for (int i = 0; i < m_cdT; i++)
	{
		gdTAvg += m_adTRealtime[i];
		dTWorst = GMax(dTWorst, m_adTRealtime[i]);
	}
	gdTAvg /= m_cdT;

	if (gdTAvg == 0.0f)
		return;

	float gFps = 1.0f / gdTAvg;

	// TODO fps counter should be a node 2d these things are parented to

	m_hText->SetText(StrPrintf("avg: %.2fms (%.0ffps)\nworst: %.2fms\n", 1000 * gdTAvg, GRound(gFps, 0), 1000 * dTWorst));

	// BB why do we have to scale vecwinsize by 2?

	m_hText->m_pos = float2(20.0f, 2.0f * vecWinSize.m_y - 20.0f);
}
