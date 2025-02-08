#include "uinode.h"

SUiNode::SUiNode(SNodeHandle hNodeParent, const std::string & str, TYPEK typek) : super(hNodeParent, str, typek)
{
}

void SUiNode::GetRenderConstants(SUiNodeRenderConstants * pUinoderc)
{
	pUinoderc->m_posCenter = m_pos;
	pUinoderc->m_vecScale = m_vecScale;
	pUinoderc->m_color = m_color;
}