#pragma once

#include "object.h"
#include "uinode.h"
#include <list>

struct SConsole : SUiNode // console
{
	typedef SUiNode super;
	SConsoleHandle	HConsole() { return (SConsoleHandle) m_nHandle; }

					SConsole(SNode * pNodeParent, const std::string & strName, TYPEK typek = TYPEK_Console);

	void			Update() override;
	std::string		StrPrint();
	void			Print(std::string str, double systRealtimeExpire);

	struct SEntry // entry
	{
		std::string m_str = {};
		double		m_systRealtimeExpire = 0.0;
	};

	STextHandle m_hTextConsole = -1;
	std::list<SEntry> m_lEntry;
};
