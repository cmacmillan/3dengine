#pragma once

#include "object.h"
#include "node.h"

struct SNode3D : SNode // node3D
{
	typedef SNode super;
	SNode3DHandle HNode3D() { return (SNode3DHandle) m_h; }

	SNode3D(SNode * pNodeParent, const std::string & strName, TYPEK typek = TYPEK_Node3D);

	void SetParent(SNode * pNodeParent) override;

	SNode3D * PNode3DParent();

	void SetPosQuatScaleLocal(Point pos, Quat quat, Vector vecScale);

	Point PosLocal() const { return m_transformLocal.m_pos; }
	Point PosWorld() const { return m_matObjectToWorldCache.m_aVec[3]; }
	void SetPosLocal(Point pos);
	void SetPosWorld(Point pos);

	Quat QuatLocal() const { return m_transformLocal.m_quat; }
	Quat QuatWorld() const { return m_quatObjectToWorldCache; }
	void SetQuatLocal(Quat quat);
	void SetQuatWorld(Quat quat);

	Vector VecScaleLocal() const { return m_transformLocal.m_vecScale; }
	void SetVecScaleLocal(Vector vecScale);

	Mat MatObjectToWorld() const { return m_matObjectToWorldCache;  }
	Mat MatObjectToParent() { return m_transformLocal.Mat(); }

	Vector VecXWorld() const { return Vector(m_matObjectToWorldCache.m_aVec[0]); }
	Vector VecYWorld() const { return Vector(m_matObjectToWorldCache.m_aVec[1]); }
	Vector VecZWorld() const { return Vector(m_matObjectToWorldCache.m_aVec[2]); }

	virtual void UpdateSelfAndChildTransformCache();

	float GScaleMaxCache() const { return m_gScaleMaxCache; }

protected:
	Quat QuatWorldToParent();
	Quat QuatParentToWorld();
	Mat MatWorldToParent();
	Mat MatParentToWorld();

	Transform	m_transformLocal;

	Mat			m_matObjectToWorldCache = g_matIdentity;
	Quat		m_quatObjectToWorldCache = g_quatIdentity;
	float		m_gScaleMaxCache; // For dealing with estimating the scale of a nonuniformly scaled bounding sphere
};
