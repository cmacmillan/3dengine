#include "player.h"

#include "engine.h"
#include "camera3d.h"

SPlayer::SPlayer(SNodeHandle hNodeParent, const std::string & strName) : super (hNodeParent, strName)
{
	m_typek = TYPEK_Player;

	m_hCamera3D = (new SCamera3D(HNode(), "PlayerCammera", RadFromDeg(103.0f), 0.1, 700.0f))->HCamera3D();
	g_game.m_hPlayer = HPlayer();
}

void SPlayer::Update()
{
}
