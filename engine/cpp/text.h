#pragma once

#include "object.h"
#include "uinode.h"

struct SText : SUiNode // text
{
	typedef SUiNode super;
	STextHandle HText() { return (STextHandle) m_nHandle; }

	SText(SFontHandle hFont, SNodeHandle hNodeParent, const std::string & str);
	~SText();

	void SetText(const std::string & str);

	SFontHandle m_hFont = -1;
	std::string m_str;
};
