#pragma once

#include "object.h"
#include "node.h"
#include "color.h"

struct SUiNodeRenderConstants
{
	float2 m_posCenter;
	float2 m_vecScale;
	SRgba m_color;
};

struct SUiNode : SNode // uinode
{
	typedef SNode super;
	SUiNodeHandle HUinode() { return (SUiNodeHandle) m_nHandle; }

	SUiNode(SNode * pNodeParent, const std::string & strName, TYPEK typek = TYPEK_UiNode);
	void GetRenderConstants(SUiNodeRenderConstants * pUinoderc);

	float2 m_pos = { 0.0f, 0.0f };
	float2 m_vecScale = { 1.0f, 1.0f };

	SRgba m_color = SRgba(1.0f, 1.0f, 1.0f, 1.0f);
	float m_gSort = 0.0f; // Lower = drawn first
	SMaterialHandle m_hMaterial = -1;
	SMesh3DHandle m_hMesh = -1;
};

