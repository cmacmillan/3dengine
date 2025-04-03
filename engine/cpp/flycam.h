#pragma once

#include "object.h"
#include "node3d.h"

#define FLYCAM_RADIUS_DEFAULT 20.0f

struct SFlyCam : SNode3D
{
	typedef SNode3D super;
	SFlyCam(SNode * pNodeParent, const std::string & strName, TYPEK typek = TYPEK_FlyCam);
	SFlycamHandle HFlycam() { return (SFlycamHandle) m_h; }

	void Update() override;

	SCamera3DHandle m_hCamera3D = -1;

	bool m_fInteracting = false;
	int m_xCursorPrev = -1;
	int m_yCursorPrev = -1;

	Point m_posCenter = g_posZero;
	float m_sRadiusCenter = FLYCAM_RADIUS_DEFAULT;

	double		m_systRealtimeDetatched = SYST_INVALID;
	Point		m_posCenterDetatched = g_posZero;
	float		m_sRadiusCenterDetatched = 10.0f;

	double m_systRealtimeLastDoubleClick = SYST_INVALID;
};
