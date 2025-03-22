#include "uinode.h"

SUiNode::SUiNode(SNode * pNodeParent, const std::string & str, TYPEK typek) : super(pNodeParent, str, typek)
{
}

void SUiNode::GetRenderConstants(SUiNodeRenderConstants * pUinoderc)
{
	pUinoderc->m_posCenter = m_pos;
	pUinoderc->m_vecScale = m_vecScale;
	pUinoderc->m_color = RgbaSrgbFromLinear(m_color);
}