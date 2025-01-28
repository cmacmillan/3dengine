#pragma once

#include "object.h"

void Draw3D(std::vector<SDrawNode3D *> * parypDrawnode3DToRender, SCamera3D * pCamera);
void BindMaterialTextures(const SMaterial * pMaterial, const SShader * pShader);
