#pragma once

#pragma once

#include "object.h"
#include "node3d.h"

struct SPlayer : SNode3D
{
	typedef SNode3D super;
	SPlayer(SNode * pNodeParent, const std::string & strName, TYPEK typek = TYPEK_Player);
	SPlayerHandle HPlayer() { return (SPlayerHandle) m_h; }

	void Update() override;

	SCamera3DHandle m_hCamera3D = -1;
};
