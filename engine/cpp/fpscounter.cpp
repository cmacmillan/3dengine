
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
}

SFpsCounter::~SFpsCounter()
{
	delete m_hText.PT();
}

void SFpsCounter::Update()
{
	super::Update();

	float2 vecWinSize = g_game.VecWinSize();

	m_cdT = NMin(++m_cdT, DIM(m_adT));
	m_idT = (m_idT + 1) % m_cdT;
	m_adT[m_idT] = g_game.m_dT;

	float gdTAvg = 0.0f;
	float dTWorst = 0.0f;
	for (int i = 0; i < m_cdT; i++)
	{
		gdTAvg += m_adT[i];
		dTWorst = GMax(dTWorst, m_adT[i]);
	}
	gdTAvg /= m_cdT;

	float gFps = 1.0f / gdTAvg;

	float s_gEpsilonNearEnough = .05f;
	if (gFps - NFloor(gFps) < s_gEpsilonNearEnough)
		gFps = NFloor(gFps);

	if (NCeil(gFps) - gFps < s_gEpsilonNearEnough)
		gFps = NCeil(gFps);

	m_hText->SetText(StrPrintf("avg: %.2f\nworst: %.2f\nhistory:%i", gFps, 1.0f / dTWorst, m_cdT));

#if 0
	// BB this seems to imply that our 2d rendering doesn't have the proper coordinates

	// Todo offset by text size rather than arbitrary nonsense

	m_hText->m_pos = float2(vecWinSize.m_x * 1.0f, vecWinSize.m_y * 1.9f);
#else
	m_hText->m_pos = float2(20.0f, vecWinSize.m_y * 1.9f);
#endif
}
