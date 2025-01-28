#include "flycam.h"
#include "engine.h"

SFlyCam::SFlyCam(SNodeHandle hNodeParent, const std::string & strName) : super(hNodeParent, strName)
{
	m_typek = TYPEK_FlyCam;

	m_hCamera3D = (new SCamera3D(HNode(), "FlyCamCamera", RadFromDeg(90.0f), 0.1, 700.0f))->HCamera3D();
	g_game.m_hCamera3DMain = m_hCamera3D;
	m_hCamera3D->m_xNearClip = -100;
	m_hCamera3D->SetOrthographic(100.0f);
}

void SFlyCam::Update()
{
	super::Update();

	float gMoveSpeed = g_game.m_mpVkFDown[VK_SHIFT] ? 40.0f : (g_game.m_mpVkFDown[VK_M] ? 1.0f : 10.0f);
	float gRotSpeed = 1.0f;

	Vector vecObjForward = VecProjectOnTangent(MatObjectToWorld().VecX(), g_vecZAxis);
	if (g_game.m_mpVkFDown[VK_W])
		SetPosWorld(PosWorld() + vecObjForward * gMoveSpeed * g_game.m_dT);

	if (g_game.m_mpVkFDown[VK_S])
		SetPosWorld(PosWorld() - vecObjForward * gMoveSpeed * g_game.m_dT);

	Vector vecObjLeft = VecProjectOnTangent(MatObjectToWorld().VecY(), g_vecZAxis);
	if (g_game.m_mpVkFDown[VK_A])
		SetPosWorld(PosWorld() + vecObjLeft * gMoveSpeed * g_game.m_dT);

	if (g_game.m_mpVkFDown[VK_D])
		SetPosWorld(PosWorld() - vecObjLeft * gMoveSpeed * g_game.m_dT);

	if (g_game.m_mpVkFDown[VK_SPACE])
		SetPosWorld(PosWorld() + g_vecZAxis * gMoveSpeed * g_game.m_dT);

	if (g_game.m_mpVkFDown[VK_CONTROL])
		SetPosWorld(PosWorld() - g_vecZAxis * gMoveSpeed * g_game.m_dT);

#define FLYCAM_LOOKAT 0
#if FLYCAM_LOOKAT
	SetQuatWorld(QuatLookAt(VecNormalize(g_game.m_hPlaneTest->PosWorld() - PosWorld()), g_vecZAxis));
#else
	if (g_game.m_mpVkFDown[VK_LEFT])
	{
		SetQuatWorld(QuatAxisAngle(g_vecZAxis, gRotSpeed*g_game.m_dT) * QuatWorld());
	}

	if (g_game.m_mpVkFDown[VK_RIGHT])
	{
		SetQuatWorld(QuatAxisAngle(g_vecZAxis, gRotSpeed*-g_game.m_dT) * QuatWorld());
	}

	if (g_game.m_mpVkFDown[VK_UP])
	{
		SetQuatWorld(QuatWorld() * QuatAxisAngle(g_vecYAxis, gRotSpeed*-g_game.m_dT));
	}

	if (g_game.m_mpVkFDown[VK_DOWN])
	{
		SetQuatWorld(QuatWorld() * QuatAxisAngle(g_vecYAxis, gRotSpeed*g_game.m_dT));
	}
#endif
}
