#pragma once

#include "object.h"

struct SMesh3D;

SMesh3D * PMeshLoadSingle(const char * pChzPath);
void SpawnScene(const char * pChzPath);
