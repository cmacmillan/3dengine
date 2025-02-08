#include "sun.h"
#include "engine.h"

SSun::SSun(SNodeHandle hNodeParent, const std::string & strName, TYPEK typek) : super(hNodeParent, strName, typek)
{
	g_game.m_hSun = HSun();
}
