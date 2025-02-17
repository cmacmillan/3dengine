#pragma once

#include "object.h"
#include "node3d.h"

struct SFlyCam : SNode3D
{
	typedef SNode3D super;
	SFlyCam(SNode * pNodeParent, const std::string & strName, TYPEK typek = TYPEK_FlyCam);
	SFlycamHandle HFlycam() { return (SFlycamHandle) m_nHandle; }

	void Update() override;

	SCamera3DHandle m_hCamera3D = -1;

	bool m_fMouseControls = false;
	bool m_fInteracting = false;
	int m_xCursorPrev = -1;
	int m_yCursorPrev = -1;
	Point m_posCenter = g_posZero;
	float m_sRadiusCenter = 10.0f;
};
