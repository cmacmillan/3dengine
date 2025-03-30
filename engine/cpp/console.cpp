#include "console.h"
#include "engine.h"
#include "text.h"

SConsole::SConsole(SNode * pNodeParent, const std::string & strName, TYPEK typek) :super(pNodeParent, strName, typek)
{
	m_hTextConsole = (new SText(g_game.m_hFont, this, "ConsoleText"))->HText();
	m_hTextConsole->m_hMaterial = g_game.m_hMaterialText;
	m_hTextConsole->m_vecScale = float2(0.2f, 0.2f);
	m_hTextConsole->m_gSort = 10.0f;
	m_hTextConsole->m_pos = float2(20.0f, 700.0f);
	m_hTextConsole->m_color = g_rgbaBlack;
}

std::string SConsole::StrPrint()
{
	std::string str = "";

	for (const SEntry & entry : m_lEntry)
	{
		str += entry.m_str;
	}

	return str;
}

void SConsole::Print(std::string str, double systRealtimeExpire)
{
	if (systRealtimeExpire == g_game.m_systRealtime)
	{
		m_lEntry.push_back({ str, 0.0f, 1 });
	}
	else
	{
		m_lEntry.push_back({ str, systRealtimeExpire, -1 });
	}
}

void SConsole::Update()
{
	super::Update();

	auto it = m_lEntry.begin();
	while (it != m_lEntry.end())
	{
		auto itNext = std::next(it);
		if (it->m_cFrame == -1)
		{
			if (it->m_systRealtimeExpire < g_game.m_systRealtime)
			{
				m_lEntry.erase(it);
			}
		}
		else
		{
			if (it->m_cFrame == 0)
			{
				m_lEntry.erase(it);
			}
			else
			{
				it->m_cFrame--;
			}
		}
		it = itNext;
	}
}
