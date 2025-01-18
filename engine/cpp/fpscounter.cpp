
#include "fpscounter.h"
#include "util.h"

SFpsCounter::SFpsCounter(SHandle<SNode> hNodeParent) : super(hNodeParent)
{
	m_hText = (new SText(g_game.m_hFont, HNode()))->HText();
	m_hText->m_hMaterial = g_game.m_hMaterialText;
	m_hText->SetText("FPS: ___");
	m_hText->m_vecScale = float2(0.3f, 0.3f);
	m_hText->m_gSort = 10.0f;
	m_hText->m_pos = float2(0.0f, 0.0f);
	m_hText->m_color = { 0.0f, 0.0f, 0.0f, 1.0f };

	m_hTextFps = (new SText(g_game.m_hFont, HNode()))->HText();
	m_hTextFps->m_hMaterial = g_game.m_hMaterialText;
	m_hTextFps->SetText("FPS: ___");
	m_hTextFps->m_vecScale = float2(0.3f, 0.3f);
	m_hTextFps->m_gSort = 10.0f;
	m_hTextFps->m_pos = float2(0.0f, 0.0f);
	m_hTextFps->m_color = { 0.0f, 0.0f, 0.0f, 1.0f };
}

SFpsCounter::~SFpsCounter()
{
	delete m_hText.PT();
	delete m_hTextFps.PT();
}

void SFpsCounter::Update()
{
	super::Update();

	float2 vecWinSize = g_game.VecWinSize();

	m_cdT = NMin(++m_cdT, DIM(m_adT));
	m_idT = (m_idT + 1) % m_cdT;
	m_adT[m_idT] = g_game.m_dTSyst - m_dTSyst;
	m_dTSyst = g_game.m_dTSyst;

	float gdTAvg = 0.0f;
	float dTWorst = 0.0f;
	for (int i = 0; i < m_cdT; i++)
	{
		gdTAvg += m_adT[i];
		dTWorst = GMax(dTWorst, m_adT[i]);
	}
	gdTAvg /= m_cdT;

	if (gdTAvg == 0.0f)
		return;

	float gFps = 1.0f / gdTAvg;

	// TODO fps counter should be a node 2d these things are parented to

	m_hText->SetText(StrPrintf("avg: %.1fms \nworst: %.1fms\nhistory:%i", GRound(1000 * gdTAvg, 1), GRound(1000 * dTWorst, 1), m_cdT));
	m_hTextFps->SetText(StrPrintf("(%.0ffps)", GRound(gFps, 0)));

	// BB why do we have to scale vecwinsize by 2?

	m_hText->m_pos = float2(20.0f, 2.0f * vecWinSize.m_y - 20.0f);
	m_hTextFps->m_pos = float2(350.0f, 2.0f * vecWinSize.m_y - 20.0f);
}
