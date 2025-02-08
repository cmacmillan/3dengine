#include "player.h"

#include "engine.h"
#include "camera3d.h"
#include "flycam.h"

SPlayer::SPlayer(SNodeHandle hNodeParent, const std::string & strName, TYPEK typek) : super (hNodeParent, strName, typek)
{
	m_hCamera3D = (new SCamera3D(HNode(), "PlayerCammera", RadFromDeg(103.0f), 0.1, 700.0f))->HCamera3D();
	g_game.m_hPlayer = HPlayer();
}

void SPlayer::Update()
{
	super::Update();

	if (g_game.m_edits != EDITS_Player)
		return;

	// TODO keep cursor in the screen when in 'player' mode

	if (g_game.m_fWindowFocused)
	{
		float2 vecWinSize = g_game.VecWinSize();
		float2 vecWinTopLeft = g_game.VecWinTopLeft();
		SetCursorPos(vecWinTopLeft.m_x + vecWinSize.m_x / 2, vecWinTopLeft.m_y + vecWinSize.m_y / 2);
	}
}
