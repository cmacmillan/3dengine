#pragma once

#include "gui.h"
#include "move.h"
#include <array>

inline int IChFromCoords(int x, int y) { return y * DX_GRID + x; }

struct SSolverBoard
{
	SSolverBoard(SScrabbleGridHandle hGrid);
	SSolverBoard(const char * pChzBoardLayout);
	char ChFromCoords(int x, int y) const { return m_aCh[IChFromCoords(x, y)]; }
	char m_aCh[DX_GRID * DY_GRID];
};

struct SRack
{
	SRack(const char * pChzRack);
	SFixArray<char, 7> m_aryCh; // STORED IN LOWERCASE
};

struct SWordFrag
{
	unsigned char m_n;
	char m_aCh[0];
	bool FIsWord();
	int CLinks();
	int * AI();
};

struct SScrabbleSolver
{
	SScrabbleSolver(const char * pChzWordgraphFile);

	void FindValidMoves(const SSolverBoard & solverboard, SRack rack, std::vector<SMove> * paryMove);
	void WalkHorizontal(const SSolverBoard & solverboard, int xMic, int xMac, int y, SRack * pRack, SMove * pMove, SWordFrag * pWordfrag, std::vector<SMove> * paryMove);
	bool FIsWord(const char * pChzWord);

	SWordFrag * PWordfragFollowPrepend(char ch, SWordFrag * pWordfragCur);
	SWordFrag * PWordfragFollowAppend(char ch, SWordFrag * pWordfragCur);

	unsigned char * m_pB = nullptr;

	SWordFrag * m_pWordfragRoot = nullptr;

	void Audit();
};
