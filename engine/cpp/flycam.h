#pragma once

#include "object.h"
#include "node3d.h"

struct SFlyCam : SNode3D
{
	typedef SNode3D super;
	SFlyCam(SNodeHandle hNodeParent);
	SFlycamHandle HFlycam() { return (SFlycamHandle) m_nHandle; }

	void Update() override;

	SCamera3DHandle m_hCamera3D = -1;
};
