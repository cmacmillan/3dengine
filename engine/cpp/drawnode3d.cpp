#include "drawnode3d.h"

SDrawNode3D::SDrawNode3D(SNodeHandle hNodeParent, const std::string & str) :
	super(hNodeParent, str)
{
	m_typek = TYPEK_DrawNode3D;
}