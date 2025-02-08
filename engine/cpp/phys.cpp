#include "phys.h"

SPhysCube::SPhysCube(SNodeHandle hNodeParent, const std::string & strName) : super(hNodeParent, strName)
{
	m_typek = TYPEK_PhysCube;
}
