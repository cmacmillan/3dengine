#pragma once

#include "object.h"
#include "color.h"

extern int g_cDraw3D;

struct SFrustum
{
	Point m_posMin;
	Point m_posMinYzMaxX;
	Point m_posMinXzMaxY;
	Point m_posMinXyMaxZ;
	Point m_posMinXMaxYz;
	Point m_posMinYMaxXz;
	Point m_posMinZMaxXy;
	Point m_posMax;
};


SFrustum FrustumTransform(const SFrustum & frustum, const Mat & mat);
bool FInFrustum(const SFrustum & frustumWorld, const Mat & matWorldToObject, Point posSphereLocal, float sRadiusLocal);


void Draw3D(std::vector<SDrawNode3D *> * parypDrawnode3DToRender, const Mat & matWorldToClip, const Mat & matWorldToCamera, const SFrustum & frustum, bool fDrawAsShadowcaster);
void Draw3DSingle(const SMaterial * pMaterial, const SMesh3D * pMesh, const Mat & matModel, const Mat & matWorldToClip, const Mat & matWorldToCamera, const SFrustum & frustum, SRgba rgba = SRgba(1.0f, 1.0f, 1.0f, 1.0f));
void BindMaterialTextures(const SMaterial * pMaterial, const SShader * pShader);
void UnbindTextures(const SShader * pShader);
void BindGlobalsForCamera(SCamera3D * pCamera, SCamera3D * pCameraShadow);
