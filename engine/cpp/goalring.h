#pragma once

#include "object.h"
#include "drawnode3d.h"

struct SGoalRing : SDrawNode3D
{
	typedef SDrawNode3D super;
	SGoalRing(SNodeHandle hNodeParent, const std::string & strName, TYPEK typek = TYPEK_GoalRing);
	SGoalRingHandle HFlycam() { return (SGoalRingHandle) m_nHandle; }

	void Update() override;
};
