#pragma once

#include "object.h"
#include "color.h"

extern int g_cDraw3D;

struct SFrustum
{
	Point m_posMin = g_posZero;
	Point m_posMinYzMaxX = g_posZero;
	Point m_posMinXzMaxY = g_posZero;
	Point m_posMinXyMaxZ = g_posZero;
	Point m_posMinXMaxYz = g_posZero;
	Point m_posMinYMaxXz = g_posZero;
	Point m_posMinZMaxXy = g_posZero;
	Point m_posMax = g_posZero;

	void DebugDraw(SRgba rgba = g_rgbaAqua);
};


SFrustum FrustumTransform(const SFrustum & frustum, const Mat & mat);
bool FInFrustum(const SFrustum & frustumWorld, Point posSphereWorld, float sRadiusWorld);
Point PosClosestInFrustum(const Point & posPoint, const SFrustum & frustum);

void Draw3D(std::vector<SDrawNode3D *> * parypDrawnode3DToRender, const Mat & matWorldToClip, const Mat & matWorldToCamera, const SFrustum & frustum, bool fDrawAsShadowcaster);
void Draw3DSingle(const SMaterial * pMaterial, const SMesh3D * pMesh, const Mat & matModel, const Mat & matWorldToClip, const Mat & matWorldToCamera, const SFrustum & frustum, float gMaxScale, SRgba rgba = SRgba(1.0f, 1.0f, 1.0f, 1.0f));
void BindMaterialTextures(const SMaterial * pMaterial, const SShader * pShader);
void UnbindTextures(const SShader * pShader);
void BindGlobalsForCamera(SCamera3D * pCamera, SCamera3D * pCameraShadow);
