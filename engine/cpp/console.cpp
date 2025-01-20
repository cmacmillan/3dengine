#include "console.h"
#include "engine.h"
#include "text.h"

SConsole::SConsole(SNodeHandle hNodeParent, const std::string & strName) :super(hNodeParent, strName)
{
	m_typek = TYPEK_Console;

	m_hTextConsole = (new SText(g_game.m_hFont, HNode(), "ConsoleText"))->HText();
	m_hTextConsole->m_hMaterial = g_game.m_hMaterialText;
	m_hTextConsole->m_vecScale = float2(0.2f, 0.2f);
	m_hTextConsole->m_gSort = 10.0f;
	m_hTextConsole->m_pos = float2(20.0f, 500.0f);
	m_hTextConsole->m_color = { 0.0f, 0.0f, 0.0f, 1.0f };
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

void SConsole::Print(std::string str, double dTSyst)
{
	m_lEntry.push_back({ str, dTSyst });
}

void SConsole::Update()
{
	super::Update();

	auto it = m_lEntry.begin();
	while (it != m_lEntry.end())
	{
		auto itNext = std::next(it);
		if (it->m_dTSystExpire < g_game.m_dTSyst)
		{
			m_lEntry.erase(it);
		}
		it = itNext;
	}
}
