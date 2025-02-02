#pragma once

#include "object.h"
#include "node3d.h"

struct SSun : SNode3D
{
	typedef SNode3D super;
	SSun(SNodeHandle hNodeParent, const std::string & strName);
	SSunHandle HSun() { return (SSunHandle) m_nHandle; }
};
