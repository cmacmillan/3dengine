#include "sun.h"
#include "engine.h"

SSun::SSun(SNodeHandle hNodeParent, const std::string & strName) : super(hNodeParent, strName)
{
	m_typek = TYPEK_Sun;

	g_game.m_hSun = HSun();
}
