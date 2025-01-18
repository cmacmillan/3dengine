#include "camera3d.h"

SCamera3D::SCamera3D(SNodeHandle hNodeParent, float radFovHorizontal, float xNearClip, float xFarClip) :
	super(hNodeParent),
	m_radFovHorizontal(radFovHorizontal),
	m_xNearClip(xNearClip),
	m_xFarClip(xFarClip)
{
	m_typek = TYPEK_Camera3D;
}