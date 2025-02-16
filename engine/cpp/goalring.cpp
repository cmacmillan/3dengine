#include "goalring.h"

#include "engine.h"

SGoalRing::SGoalRing(SNodeHandle hNodeParent, const std::string & strName, TYPEK typek) : super(hNodeParent, strName, typek)
{
}

void SGoalRing::Update()
{
	super::Update();

	SetQuatLocal(QuatLocal() * QuatAxisAngle(g_vecYAxis, g_game.m_dT));

	g_game.DebugDrawSphere(PosWorld(), 1.0f + GSin(g_game.m_dTSyst*10.)+.01f, .2f);
}
