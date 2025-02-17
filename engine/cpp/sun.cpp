#include "sun.h"
#include "engine.h"

SSun::SSun(SNode * pNodeParent, const std::string & strName, TYPEK typek) : super(pNodeParent, strName, typek)
{
	g_game.m_hSun = HSun();
}
