#pragma once

#include "object.h"

void Draw3D(std::vector<SDrawNode3D *> * parypDrawnode3DToRender, SCamera3D * pCamera, bool fDrawAsShadowcaster);
void BindMaterialTextures(const SMaterial * pMaterial, const SShader * pShader);
void UnbindTextures(const SShader * pShader);
void BindGlobalsForCamera(SCamera3D * pCamera, SCamera3D * pCameraShadow);
