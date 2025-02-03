#include "flycam.h"
#include "engine.h"
#include "player.h"

SFlyCam::SFlyCam(SNodeHandle hNodeParent, const std::string & strName) : super(hNodeParent, strName)
{
	m_typek = TYPEK_FlyCam;

	m_hCamera3D = (new SCamera3D(HNode(), "FlyCamCamera", RadFromDeg(90.0f), 0.1, 700.0f))->HCamera3D();
	g_game.m_hCamera3DMain = m_hCamera3D;

	m_fActive = true;
	m_fMouseControls = true;
}

void SFlyCam::Update()
{
	super::Update();

	SPlayer * pPlayer = g_game.m_hPlayer.PT();
	if (g_game.m_mpVkFJustPressed[VK_H])
	{
		bool fActiveNext = !m_fActive || !pPlayer;
		if (fActiveNext != m_fActive)
		{
			m_fActive = fActiveNext;

			if (m_fActive)
			{
				// Activating, copy position

				m_posCenter = pPlayer->PosWorld();
				m_transformLocal.m_quat = g_quatIdentity;
				m_sRadiusCenter = 10.0f;
				g_game.m_hCamera3DMain = m_hCamera3D;
			}
			else
			{
				// Deactivating

				g_game.m_hCamera3DMain = pPlayer->m_hCamera3D;
			}
		}
	}

	g_game.PrintConsole(m_fActive ? "Orbit cam\n" : "Player cam\n");

	if (!m_fActive)
		return;

	float gMoveSpeed = g_game.m_mpVkFDown[VK_SHIFT] ? 40.0f : (g_game.m_mpVkFDown[VK_M] ? 1.0f : 10.0f);

	if (m_fMouseControls)
	{
		TWEAKABLE float s_gScrollSpeed = .03f;
		m_sRadiusCenter += s_gScrollSpeed * -g_game.m_sScroll;
		m_sRadiusCenter = GMax(1.0f, m_sRadiusCenter);

		bool fAltDown = g_game.m_mpVkFDown[VK_MENU];
		bool fMiddleMouseInteracting = fAltDown && g_game.m_mpVkFDown[VK_MBUTTON];
		bool fLeftMouseInteracting = fAltDown && g_game.m_mpVkFDown[VK_LBUTTON];

		bool fInteracting = fMiddleMouseInteracting || fLeftMouseInteracting;
		if (fInteracting != m_fInteracting)
		{
			m_fInteracting = fInteracting;

			if (fInteracting)
			{
				m_xCursorPrev = g_game.m_xCursor;
				m_yCursorPrev = g_game.m_yCursor;
			}
		}

		int dX = g_game.m_xCursor - m_xCursorPrev;
		int dY = g_game.m_yCursor - m_yCursorPrev;

		Vector vecLeft = VecRotate(g_vecYAxis, m_transformLocal.m_quat);
		if (fLeftMouseInteracting)
		{
			float gRotSpeed = .005f;
			SetQuatLocal(QuatAxisAngle(g_vecZAxis, -dX * gRotSpeed) * m_transformLocal.m_quat);
			SetQuatLocal(QuatAxisAngle(VecNormalize(VecProjectOnTangent(vecLeft, g_vecZAxis)), dY * gRotSpeed) * m_transformLocal.m_quat);
		}

		if (fMiddleMouseInteracting)
		{
			float gMoveSpeed = .05f;
			m_posCenter = m_posCenter + gMoveSpeed * dX * VecYWorld();
			m_posCenter = m_posCenter + gMoveSpeed * dY * VecZWorld();		
		}

		if (fInteracting)
		{
			m_xCursorPrev = g_game.m_xCursor;
			m_yCursorPrev = g_game.m_yCursor;
		}

		if (g_game.m_mpVkFDown[VK_SPACE])
			m_posCenter = m_posCenter + g_vecZAxis * gMoveSpeed * g_game.m_dT;

		if (g_game.m_mpVkFDown[VK_CONTROL])
			m_posCenter = m_posCenter - g_vecZAxis * gMoveSpeed * g_game.m_dT;

		Vector vecObjForward = VecProjectOnTangent(MatObjectToWorld().VecX(), g_vecZAxis);
		if (g_game.m_mpVkFDown[VK_W])
			m_posCenter = m_posCenter + vecObjForward * gMoveSpeed * g_game.m_dT;

		if (g_game.m_mpVkFDown[VK_S])
			m_posCenter = m_posCenter - vecObjForward * gMoveSpeed * g_game.m_dT;

		Vector vecObjLeft = VecProjectOnTangent(MatObjectToWorld().VecY(), g_vecZAxis);
		if (g_game.m_mpVkFDown[VK_A])
			m_posCenter = m_posCenter + vecObjLeft * gMoveSpeed * g_game.m_dT;

		if (g_game.m_mpVkFDown[VK_D])
			m_posCenter = m_posCenter - vecObjLeft * gMoveSpeed * g_game.m_dT;

		Vector vecTowardsCenter = VecRotate(g_vecXAxis, m_transformLocal.m_quat);
		SetPosWorld(m_posCenter + m_sRadiusCenter * -vecTowardsCenter);
	}
	else
	{
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

		if (g_game.m_mpVkFDown[VK_LEFT])
		{
			SetQuatWorld(QuatAxisAngle(g_vecZAxis, gRotSpeed * g_game.m_dT) * QuatWorld());
		}

		if (g_game.m_mpVkFDown[VK_RIGHT])
		{
			SetQuatWorld(QuatAxisAngle(g_vecZAxis, gRotSpeed * -g_game.m_dT) * QuatWorld());
		}

		if (g_game.m_mpVkFDown[VK_UP])
		{
			SetQuatWorld(QuatWorld() * QuatAxisAngle(g_vecYAxis, gRotSpeed * -g_game.m_dT));
		}

		if (g_game.m_mpVkFDown[VK_DOWN])
		{
			SetQuatWorld(QuatWorld() * QuatAxisAngle(g_vecYAxis, gRotSpeed * g_game.m_dT));
		}
	}
}
