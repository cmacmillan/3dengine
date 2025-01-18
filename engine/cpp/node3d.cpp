#include "node3d.h"

SNode3D::SNode3D(SNodeHandle hNodeParent) :
	super(hNodeParent),
	m_transformLocal()
{
	m_typek = TYPEK_Node3D;
}