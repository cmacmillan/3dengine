#pragma once

#include "object.h"
#include "node3d.h"

struct SSun : SNode3D
{
	typedef SNode3D super;
	SSun(SNode * pNodeParent, const std::string & strName, TYPEK typek = TYPEK_Sun);
	SSunHandle HSun() { return (SSunHandle) m_h; }
};
