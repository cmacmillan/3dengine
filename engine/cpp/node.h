#pragma once

#include "object.h"

struct SNode : SObject // node
{
	typedef SObject super;
	SNodeHandle HNode() { return (SNodeHandle) m_nHandle; }

	SNode(SNodeHandle hNodeParent);
	void SetParent(SNodeHandle hNodeParent);

	virtual void Update() {}

	SNodeHandle m_hNodeParent = -1;

	SNodeHandle m_hNodeSiblingPrev = -1;
	SNodeHandle m_hNodeSiblingNext = -1;

	SNodeHandle m_hNodeChildFirst = -1;
	SNodeHandle m_hNodeChildLast = -1;

};

