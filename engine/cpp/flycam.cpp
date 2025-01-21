#include "flycam.h"
#include "engine.h"

SFlyCam::SFlyCam(SNodeHandle hNodeParent, const std::string & strName) : super(hNodeParent, strName)
{
	m_typek = TYPEK_FlyCam;

	m_hCamera3D = (new SCamera3D(HNode(), "FlyCamCamera", RadFromDeg(90.0f), 0.1, 100.0f))->HCamera3D();
	g_game.m_hCamera3DMain = m_hCamera3D;
}

void SFlyCam::Update()
{
	super::Update();

	// DBG REMOVE ME!!!!
	//SetQuatWorld(Quat(0.0f, 1.0f, 0.0f, 0.0f)));

	float gMoveSpeed = g_game.m_mpVkFDown[VK_SHIFT] ? 50.0f : 5.0f;

	if (g_game.m_mpVkFDown[VK_UP])
	{
		SetPosWorld(PosWorld() + MatObjectToWorld().VecX() * gMoveSpeed * g_game.m_dT);
	}

	if (g_game.m_mpVkFDown[VK_DOWN])
	{
		SetPosWorld(PosWorld() + MatObjectToWorld().VecX() * gMoveSpeed * -g_game.m_dT);
	}

	if (g_game.m_mpVkFDown[VK_LEFT])
	{
		SetPosWorld(PosWorld() + MatObjectToWorld().VecY() * gMoveSpeed * g_game.m_dT);
	}

	if (g_game.m_mpVkFDown[VK_RIGHT])
	{
		SetPosWorld(PosWorld() + MatObjectToWorld().VecY() * gMoveSpeed * -g_game.m_dT);
	}

#define FLYCAM_LOOKAT 0
#if FLYCAM_LOOKAT
	SetQuatWorld(QuatLookAt(VecNormalize(g_game.m_hPlaneTest->PosWorld() - PosWorld()), g_vecZAxis));
#else
	if (g_game.m_mpVkFDown[VK_A])
	{
		SetQuatWorld(QuatWorld() * QuatAxisAngle(g_vecZAxis, g_game.m_dT));
	}

	if (g_game.m_mpVkFDown[VK_D])
	{
		SetQuatWorld(QuatWorld() * QuatAxisAngle(g_vecZAxis, -g_game.m_dT));
	}

	if (g_game.m_mpVkFDown[VK_W])
	{
		SetQuatWorld(QuatWorld() * QuatAxisAngle(g_vecYAxis, -g_game.m_dT));
	}

	if (g_game.m_mpVkFDown[VK_S])
	{
		SetQuatWorld(QuatWorld() * QuatAxisAngle(g_vecYAxis, g_game.m_dT));
	}

	if (g_game.m_mpVkFDown[VK_E])
	{
		SetQuatWorld(QuatWorld() * QuatAxisAngle(g_vecXAxis, g_game.m_dT));
	}

	if (g_game.m_mpVkFDown[VK_Q])
	{
		SetQuatWorld(QuatWorld() * QuatAxisAngle(g_vecXAxis, -g_game.m_dT));
	}
#endif
}
