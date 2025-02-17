#include "flycam.h"
#include "engine.h"
#include "player.h"

SFlyCam::SFlyCam(SNode * pNodeParent, const std::string & strName, TYPEK typek) : super(pNodeParent, strName, typek)
{
	m_hCamera3D = (new SCamera3D(this, "FlyCamCamera", RadFromDeg(90.0f), 0.1, 700.0f))->HCamera3D();

	m_fMouseControls = true;
}

void SFlyCam::Update()
{
	super::Update();

	if (g_game.m_edits != EDITS_Editor)
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
			Quat quatLocal = QuatAxisAngle(g_vecZAxis, -dX * gRotSpeed) * QuatLocal();
			quatLocal = QuatAxisAngle(VecNormalize(VecProjectOnTangent(vecLeft, g_vecZAxis)), dY * gRotSpeed) * quatLocal;
			Vector normalForward = VecRotate(g_vecXAxis, quatLocal);

			// Do nothing if this will put us too close to the poles

			// BB (chasem) instead of doing this we should be decomposing the rotation, because as written if you go sideways and even a tiny bit up
			//  while near the pole your sideways input will get eaten
			//  This also doesn't handle the case if you whip the camera across the pole

			TWEAKABLE float s_gEpsilon = .99f;

			if (GAbs(GDot(normalForward, g_vecZAxis)) < s_gEpsilon)
			{
				Vector vecLeftCorrected = VecCross(g_vecZAxis, normalForward);
				Vector normalUpCorrected = VecNormalize(VecCross(normalForward, vecLeftCorrected));
				SetQuatLocal(QuatLookAt(normalForward, normalUpCorrected));
			}
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
