#include "goalring.h"

#include "engine.h"

SGoalRing::SGoalRing(SNode * pNodeParent, const std::string & strName, TYPEK typek) : super(pNodeParent, strName, typek)
{
}

void SGoalRing::Update()
{
	super::Update();

	SetQuatLocal(QuatLocal() * QuatAxisAngle(g_vecYAxis, g_game.m_dT));
}
