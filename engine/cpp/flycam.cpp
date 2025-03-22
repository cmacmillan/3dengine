#include "flycam.h"
#include "engine.h"
#include "player.h"

SFlyCam::SFlyCam(SNode * pNodeParent, const std::string & strName, TYPEK typek) : super(pNodeParent, strName, typek)
{
	m_hCamera3D = (new SCamera3D(this, "FlyCamCamera", RadFromDeg(90.0f), 0.1f, 700.0f))->HCamera3D();
}

void SFlyCam::Update()
{
	super::Update();

	if (g_game.m_edits != EDITS_Editor)
		return;

	// Look for double clicks

	{
		TWEAKABLE float s_dTDoubleClick = 0.5f;

		TWEAKABLE float s_sCursor = 10.0f;

		float2 vecCursor = g_game.VecCursor();
		int cSystDoubleClick = 0;
		for (int i = 0; i < g_game.m_aryClickhist.CCapacity(); i++)
		{
			double systRealtime = g_game.m_aryClickhist[i].m_systRealtime;

			if (systRealtime == SYST_INVALID)
				continue;

			if (systRealtime > m_systRealtimeLastDoubleClick && systRealtime + s_dTDoubleClick > g_game.m_systRealtime && SLength(vecCursor - g_game.m_aryClickhist[i].m_posCursor) < s_sCursor)
			{
				cSystDoubleClick++;
			}
		}

		if (cSystDoubleClick >= 2)
		{
			// Double click occured

			m_systRealtimeLastDoubleClick = g_game.m_systRealtime;

			Point posRaycast;
			if (g_game.FRaycastCursor(&posRaycast))
			{
				m_systRealtimeDetatched = g_game.m_systRealtime;
				m_posCenterDetatched = m_posCenter;
				m_sRadiusCenterDetatched = m_sRadiusCenter;
				m_posCenter = posRaycast;
				m_sRadiusCenter = FLYCAM_RADIUS_DEFAULT;
			}
		}
	}

	TWEAKABLE float s_gScrollSpeed = .03f;
	m_sRadiusCenter += s_gScrollSpeed * -g_game.m_sScroll;
	m_sRadiusCenter = GMax(1.0f, m_sRadiusCenter);

	bool fAltDown = g_game.m_mpVkFDown[VK_MENU];
	bool fMiddleMouseInteracting = fAltDown && g_game.m_mpVkFDown[VK_MBUTTON] && g_game.m_uiidActive == g_uiidNil;
	bool fLeftMouseInteracting = fAltDown && g_game.m_mpVkFDown[VK_LBUTTON] && g_game.m_uiidActive == g_uiidNil;

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
		TWEAKABLE float s_gDragSpeedMin = 0.001f;
		TWEAKABLE float s_rDragRadiusScalar = 0.001f;
		float gScalar = GMax(s_gDragSpeedMin, m_sRadiusCenter * s_rDragRadiusScalar);
		m_posCenter = m_posCenter + gScalar * dX * VecYWorld();
		m_posCenter = m_posCenter + gScalar* dY * VecZWorld();
	}

	if (fInteracting)
	{
		m_xCursorPrev = g_game.m_xCursor;
		m_yCursorPrev = g_game.m_yCursor;
	}

	float gMoveSpeed = g_game.m_mpVkFDown[VK_SHIFT] ? 40.0f : (g_game.m_mpVkFDown[VK_M] ? 1.0f : 10.0f);

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

	TWEAKABLE float s_dTDetatched = 0.1f;
	if (s_dTDetatched + m_systRealtimeDetatched > g_game.m_systRealtime)
	{
		float uLerp = GClamp((g_game.m_systRealtime - m_systRealtimeDetatched) / s_dTDetatched, 0.0f, 1.0f);
		SetPosWorld(Lerp(m_posCenterDetatched, m_posCenter, uLerp) + Lerp(m_sRadiusCenterDetatched, m_sRadiusCenter, uLerp) * -vecTowardsCenter);
	}
	else
	{
		SetPosWorld(m_posCenter + m_sRadiusCenter * -vecTowardsCenter);
	}
}
