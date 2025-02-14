#pragma once

#include "object.h"
#include "node.h"

struct SNode3D : SNode // node3D
{
	typedef SNode super;
	SNode3DHandle HNode3D() { return (SNode3DHandle) m_nHandle; }

	SNode3D(SNodeHandle hNodeParent, const std::string & strName, TYPEK typek = TYPEK_Node3D);

	void SetParent(SNodeHandle hNodeParent) override;

	SNode3D * PNode3DParent();

	Point PosLocal() { return m_transformLocal.m_pos; }
	Point PosWorld() { return m_matObjectToWorldCache.m_aVec[3]; }
	void SetPosLocal(Point pos);
	void SetPosWorld(Point pos);

	Quat QuatLocal() { return m_transformLocal.m_quat; }
	Quat QuatWorld() { return m_quatObjectToWorldCache; }
	void SetQuatLocal(Quat quat);
	void SetQuatWorld(Quat quat);

	Vector VecScaleLocal() { return m_transformLocal.m_vecScale; }
	void SetVecScaleLocal(Vector vecScale);

	Mat MatObjectToWorld() { return m_matObjectToWorldCache;  }

	Vector VecXWorld() { return Vector(m_matObjectToWorldCache.m_aVec[0]); }
	Vector VecYWorld() { return Vector(m_matObjectToWorldCache.m_aVec[1]); }
	Vector VecZWorld() { return Vector(m_matObjectToWorldCache.m_aVec[2]); }

	void UpdateSelfAndChildTransformCache();

protected:
	Mat MatObjectToParent() { return m_transformLocal.Mat(); }
	Quat QuatWorldToParent();
	Quat QuatParentToWorld();
	Mat MatWorldToParent();
	Mat MatParentToWorld();

	Transform	m_transformLocal;

	Mat			m_matObjectToWorldCache = g_matIdentity;
	Quat		m_quatObjectToWorldCache = g_quatIdentity;
};
