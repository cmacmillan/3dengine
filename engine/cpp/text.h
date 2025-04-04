#pragma once

#include "object.h"
#include "uinode.h"

struct SText : SUiNode // text
{
	typedef SUiNode super;
	STextHandle HText() { return (STextHandle) m_h; }

	SText(SFontHandle hFont, SNode * pNodeParent, const std::string & str, TYPEK typek = TYPEK_Text);
	~SText();

	void SetText(const std::string & str);

	SFontHandle m_hFont = -1;
	std::string m_str;
};
