#pragma once

struct SMove
{
	bool m_fIsHorizontal;
	int m_x; // first character in x
	int m_y; // first character in y
	int m_iChMic; // i in the dimension we care about
	int m_iChMac; //  ...
	char m_aCh[64]; // STORED IN LOWERCASE
	void AppendCh(char ch);
	void PrependCh(char ch);
	void UnPrependCh();
	void UnAppendCh();
};
