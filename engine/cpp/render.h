#pragma once

#include "object.h"
#include "color.h"

void Draw3D(std::vector<SDrawNode3D *> * parypDrawnode3DToRender, Mat matWorldToClip, bool fDrawAsShadowcaster);
void Draw3DSingle(const SMaterial * pMaterial, const SMesh3D * pMesh, Mat matModel, Mat matWorldToClip, SRgba rgba = SRgba(1.0f, 1.0f, 1.0f, 1.0f));
void BindMaterialTextures(const SMaterial * pMaterial, const SShader * pShader);
void UnbindTextures(const SShader * pShader);
void BindGlobalsForCamera(SCamera3D * pCamera, SCamera3D * pCameraShadow);
