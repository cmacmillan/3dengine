#include "node3d.h"

SNode3D::SNode3D(SNode * pNodeParent, const std::string & str, TYPEK typek) :
	super(pNodeParent, str, typek),
	m_transformLocal()
{
	UpdateSelfAndChildTransformCache();
}

void SNode3D::SetParent(SNode * pNodeParent)
{
	super::SetParent(pNodeParent);

	UpdateSelfAndChildTransformCache();
}

SNode3D * SNode3D::PNode3DParent()
{
	SNode3D * pNode3D = nullptr;
	if (SNode * pNode = m_pNodeParent)
	{
		if (pNode->FIsDerivedFrom(TYPEK_Node3D))
		{
			return static_cast<SNode3D *>(pNode);
		}
	}

	return nullptr;
}

void SNode3D::SetPosQuatScaleLocal(Point pos, Quat quat, Vector vecScale)
{
	m_transformLocal.m_pos = pos;
	m_transformLocal.m_quat = quat;
	m_transformLocal.m_vecScale = vecScale;
	UpdateSelfAndChildTransformCache();
}

void SNode3D::SetPosLocal(Point pos)
{
	m_transformLocal.m_pos = pos;
	UpdateSelfAndChildTransformCache();
}

void SNode3D::SetQuatLocal(Quat quat)
{
	m_transformLocal.m_quat = quat;
	UpdateSelfAndChildTransformCache();
}

void SNode3D::SetVecScaleLocal(Vector vecScale)
{
	m_transformLocal.m_vecScale = vecScale;
	UpdateSelfAndChildTransformCache();
}

void SNode3D::UpdateSelfAndChildTransformCache()
{
	m_matObjectToWorldCache = m_transformLocal.Mat() * MatParentToWorld();
	m_quatObjectToWorldCache = QuatParentToWorld() * m_transformLocal.m_quat;

	SNode * pNode = m_pNodeChildFirst;
	while (pNode != nullptr)
	{
		if (pNode->FIsDerivedFrom(TYPEK_Node3D))
		{
			static_cast<SNode3D *>(pNode)->UpdateSelfAndChildTransformCache();
		}
		pNode = pNode->m_pNodeSiblingNext;
	}
}

void SNode3D::SetPosWorld(Point pos)
{
	m_transformLocal.m_pos = pos * MatWorldToParent();
	UpdateSelfAndChildTransformCache();
}

void SNode3D::SetQuatWorld(Quat quat)
{
	m_transformLocal.m_quat = QuatWorldToParent() * quat;
	UpdateSelfAndChildTransformCache();
}

Quat SNode3D::QuatWorldToParent()
{
	SNode3D * pNode3DParent = PNode3DParent();
	return (pNode3DParent) ? pNode3DParent->QuatWorld().Inverse() : g_quatIdentity;
}

Mat SNode3D::MatWorldToParent()
{
	SNode3D * pNode3DParent = PNode3DParent();
	return (pNode3DParent) ? pNode3DParent->MatObjectToWorld().MatInverse() : g_matIdentity;
}

Mat SNode3D::MatParentToWorld()
{
	if (SNode3D * pNode3DParent = PNode3DParent())
	{
		return pNode3DParent->m_matObjectToWorldCache;
	}

	return g_matIdentity;
}

Quat SNode3D::QuatParentToWorld()
{
	if (SNode3D * pNode3DParent = PNode3DParent())
	{
		return pNode3DParent->m_quatObjectToWorldCache;
	}

	return g_quatIdentity;
}
