#include "goalring.h"

#include "engine.h"

SGoalRing::SGoalRing(SNodeHandle hNodeParent, const std::string & strName) : super(hNodeParent, strName)
{
	m_typek = TYPEK_GoalRing;
}

void SGoalRing::Update()
{
	super::Update();

	SetQuatLocal(QuatLocal() * QuatAxisAngle(g_vecYAxis, g_game.m_dT));
}
