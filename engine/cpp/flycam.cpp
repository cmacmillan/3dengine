#include "flycam.h"
#include "engine.h"

SFlyCam::SFlyCam(SNodeHandle hNodeParent) : super(hNodeParent)
{
	m_typek = TYPEK_FlyCam;

	m_hCamera3D = (new SCamera3D(g_game.m_hNodeRoot, RadFromDeg(90.0f), 0.1, 100.0f))->HCamera3D();
	g_game.m_hCamera3DMain = m_hCamera3D;
}

void SFlyCam::Update()
{
}
