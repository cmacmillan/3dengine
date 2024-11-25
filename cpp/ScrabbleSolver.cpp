#include "scrabblesolver.h"

#include "util.h"
#include "windows.h"
#include "boardlayouts.h"

SScrabbleSolver::SScrabbleSolver(const char * pChzWordgraphFile)
{
	std::string strPath = std::string(WORDGRAPH_PATH) + pChzWordgraphFile;

    HANDLE hFile = CreateFileA(strPath.c_str(),
                       GENERIC_READ,		   // open for reading
                       0,                      // do not share
                       NULL,                   // default security
                       OPEN_EXISTING,		   // open existing only
                       FILE_ATTRIBUTE_NORMAL,  // normal file
                       NULL);                  // no attr. template

    if (hFile == INVALID_HANDLE_VALUE) 
    { 
		MessageBoxA(0, "Loading wordgraph failed", "Fatal Error", MB_OK);
		exit;
    }

	int cBytesFile = GetFileSize(hFile, nullptr);
	m_pB = new unsigned char[cBytesFile];

	if (!ReadFile(hFile, m_pB, cBytesFile, nullptr, nullptr))
	{
		MessageBoxA(0, "Loading wordgraph failed", "Fatal Error", MB_OK);
		exit;
	}

    m_pWordfragRoot = (SWordFrag *)m_pB;
	Audit();
}

void RackAddSwap(SRack * pRack, int iCh, char ch)
{
    // Undoes RemoveSwap

    if (iCh == pRack->m_aryCh.m_c) // only here to avoid an assert
    {
        pRack->m_aryCh.Append(ch);
        return;
    }

    pRack->m_aryCh.Append(pRack->m_aryCh[iCh]);
    pRack->m_aryCh[iCh] = ch;
}

void SScrabbleSolver::WalkHorizontal(const SSolverBoard & solverboard, int xMic, int xMac, int y, SRack * pRack, SMove * pMove, SWordFrag * pWordfrag, std::vector<SMove> * paryMove)
{
    int cLinks = pWordfrag->CLinks();
    int * ai = pWordfrag->AI();
    if (pWordfrag->FIsWord())
    {
        std::string strBuilder = "";
        for (int i = pMove->m_iChMic; i < pMove->m_iChMac; i++)
        {
            strBuilder += pMove->m_aCh[i];
        }
        if (strBuilder == "fitnes")
        {
            printf("blah");
        }
        paryMove->push_back(*pMove);
    }

    // Fisword check and add to arymove?

	for (int iLink = 0; iLink < cLinks; iLink++)
	{
        char chLink = pWordfrag->m_aCh[iLink];
        char chRack = ToLower(chLink);
        if (FIsUpper(chLink)) // upper = append
        {
            if (xMac == DX_GRID)
                continue;
            int iCh = IFind(pRack->m_aryCh, chRack);
            if (iCh == -1)
                continue;

			int iOffset = ai[iLink];
			SWordFrag * pWordfragNext = (SWordFrag *)&m_pB[iOffset];

            // TODO TODO TODO TODO check the perpendicular dimension to ensure this play either has no neighbors or creates a valid word with them if it does

            int xMacNext = xMac + 1;
            
            // we may have bumped into another word

            int cChPushed = 0;
            {   
				bool fInvalid = false;
				while (xMacNext < DX_GRID && solverboard.ChFromCoords(xMacNext, y) != '\0')
				{
                    char chNext = solverboard.ChFromCoords(xMacNext, y);
					pWordfragNext = PWordfragFollowAppend(chNext, pWordfragNext);
                    pMove->AppendCh(chRack);
                    cChPushed++;
					if (pWordfragNext == nullptr)
					{
						fInvalid = true;
						break;
					}
                    xMacNext++;
				}
                if (fInvalid)
                {
                    // Revert

					for (int i = 0; i < cChPushed; i++)
					{
						pMove->UnAppendCh();
					}
					continue;
                }
            }

            // Remove the piece we wanna play from the rack & mutate the move

            pRack->m_aryCh.RemoveSwap(iCh);
            pMove->AppendCh(chRack);
            cChPushed++;

            WalkHorizontal(solverboard, xMic, xMacNext, y, pRack, pMove, pWordfragNext, paryMove);

            // Undo rack and move mutation

            RackAddSwap(pRack, iCh, chRack);
            for (int i = 0; i < cChPushed; i++)
            {
                pMove->UnAppendCh();
            }
            
        }
        else // lower = prepend
        {
            if (xMic == 0)
                continue;
            int iCh = IFind(pRack->m_aryCh, chRack);
            if (iCh == -1)
                continue;

			int iOffset = ai[iLink];
			SWordFrag * pWordfragNext = (SWordFrag *)&m_pB[iOffset];

            // TODO TODO TODO TODO check the perpendicular dimension to ensure this play either has no neighbors or creates a valid word with them if it does

            int xMicNext = xMic - 1;

            // we may have bumped into another word

            int cChPushed = 0;
            {   
				bool fInvalid = false;
				while (xMicNext - 1 > -1 && solverboard.ChFromCoords(xMicNext - 1, y) != '\0')
				{
                    char chNext = solverboard.ChFromCoords(xMicNext - 1, y);
					pWordfragNext = PWordfragFollowPrepend(chNext, pWordfragNext);
                    pMove->PrependCh(chRack);
                    cChPushed++;
					if (pWordfragNext == nullptr)
					{
						fInvalid = true;
						break;
					}
                    xMicNext--;
				}
                if (fInvalid)
                {
                    // Revert

					for (int i = 0; i < cChPushed; i++)
					{
						pMove->UnAppendCh();
					}
					continue;
                }
            }

            // Remove the piece we wanna play from the rack & mutate the move

            pRack->m_aryCh.RemoveSwap(iCh);
            pMove->PrependCh(chRack);
            cChPushed++;

            WalkHorizontal(solverboard, xMicNext, xMac, y, pRack, pMove, pWordfragNext, paryMove);

            // Undo rack and move mutation

            RackAddSwap(pRack, iCh, chRack);
            for (int i = 0; i < cChPushed; i++)
            {
                pMove->UnPrependCh();
            }
        }
	}

}

void SScrabbleSolver::FindValidMoves(const SSolverBoard & solverboard, SRack rack, std::vector<SMove> * paryMove)
{
    // Assuming that solverboard is valid

    // For now ignore transpositions
    // depth first, eventually early out if we detect we've been somewhere already

    // Start with only horizontal   

#define TEST_SPECIFIC_CELL 1
#define SPECIFIC_CELL_X 4
#define SPECIFIC_CELL_Y 9

#if !TEST_SPECIFIC_CELL
    int xSearchMic = 0;
    int xSearchMac = DX_GRID;
    int ySearchMic = 0;
    int ySearchMac = DY_GRID;
#else
    int xSearchMic = SPECIFIC_CELL_X;
    int xSearchMac = SPECIFIC_CELL_X + 1;
    int ySearchMic = SPECIFIC_CELL_Y;
    int ySearchMac = SPECIFIC_CELL_Y + 1;
#endif

    for (int x = xSearchMic; x < xSearchMac; x++)
    {
        for (int y = ySearchMic; y < ySearchMac; y++)
        {
            // Can't play in an occupied square

            if (solverboard.ChFromCoords(x, y) != '\0')
                continue;

            // Can't play unless we are adjacent to something

            bool fAdjacentLeft = x > 0 && solverboard.ChFromCoords(x-1, y) != '\0';
			bool fAdjacentRight = x < DX_GRID - 1 && solverboard.ChFromCoords(x + 1, y) != '\0';
			//bool fAdjacentDown = y > 0 && solverboard.ChFromCoords(x, y - 1) != '\0';
			//bool fAdjacentUp = y < DX_GRID - 1 && solverboard.ChFromCoords(x, y + 1) != '\0';
            //if (!fAdjacentLeft && !fAdjacentRight && !fAdjacentUp && !fAdjacentDown)
            if (!fAdjacentLeft && !fAdjacentRight)
                continue;

            SRack rackCur = rack;
            SMove move;
            move.m_fIsHorizontal = true;
            move.m_y = y;
            move.m_iChMic = 32; // arbitrary, just want enough space on either side
            move.m_iChMac = 32;

            int xMic;
            int xMac;
            SWordFrag * pWordfrag = m_pWordfragRoot;
            if (fAdjacentLeft)
            {
				int xCur = x - 1;
                while (xCur > -1 && solverboard.ChFromCoords(xCur, y))
                {
                    char ch = solverboard.ChFromCoords(xCur, y);
                    pWordfrag = PWordfragFollowPrepend(ch, pWordfrag);
                    move.PrependCh(ch);
                    xCur--;
                }
				xMic = xCur + 1;
				xMac = x;
            }
            else
            {
				int xCur = x + 1;
                while (xCur < DX_GRID && solverboard.ChFromCoords(xCur, y))
                {
                    char ch = solverboard.ChFromCoords(xCur, y);
                    pWordfrag = PWordfragFollowAppend(ch, pWordfrag);
                    move.AppendCh(ch);
                    xCur++;
                }
                xMic = x + 1;
                xMac = xCur;
            }
            move.m_x = xMic;

            // BB could pack all the static info into a context struct to not constantly copy

            WalkHorizontal(solverboard, xMic, xMac, y, &rackCur, &move, pWordfrag, paryMove);
        }
    }
}

bool SScrabbleSolver::FIsWord(const char * pChzWord)
{
    SWordFrag * pWordfragCur = m_pWordfragRoot;
    int cLenWord = strlen(pChzWord);
    for (int iChWord = 0; iChWord < cLenWord; iChWord++)
    {
        if (SWordFrag * pWordfragNext = PWordfragFollowAppend(pChzWord[iChWord], pWordfragCur))
        {
            pWordfragCur = pWordfragNext;
        }
        else
        {
            return false;
        }
    }
    return pWordfragCur->FIsWord();
}

SWordFrag * SScrabbleSolver::PWordfragFollowPrepend(char ch, SWordFrag * pWordfragCur)
{
	char chCur = ToLower(ch); // Lower = prepend
	int cLinksCur = pWordfragCur->CLinks();
	SWordFrag * pWordfragNext = nullptr;
	for (int iLink = 0; iLink < cLinksCur; iLink++)
	{
		if (pWordfragCur->m_aCh[iLink] == chCur)
		{
			int iOffset = pWordfragCur->AI()[iLink];
			return (SWordFrag *)&m_pB[iOffset];
		}
	}
    return nullptr;
}

SWordFrag * SScrabbleSolver::PWordfragFollowAppend(char ch, SWordFrag * pWordfragCur)
{
	char chCur = ToUpper(ch); // Upper = append
	int cLinksCur = pWordfragCur->CLinks();
	SWordFrag * pWordfragNext = nullptr;
	for (int iLink = 0; iLink < cLinksCur; iLink++)
	{
		if (pWordfragCur->m_aCh[iLink] == chCur)
		{
			int iOffset = pWordfragCur->AI()[iLink];
			return (SWordFrag *)&m_pB[iOffset];
		}
	}
    return nullptr;
}

void SScrabbleSolver::Audit()
{
    ASSERT(FIsWord("cat"));
    ASSERT(!FIsWord("asfkaljfwei"));
    ASSERT(FIsWord("taco"));
    ASSERT(!FIsWord("tacoasdf"));
    ASSERT(!FIsWord("cattaco"));
    ASSERT(!FIsWord("f"));
    ASSERT(FIsWord("BlUnDeR"));
    ASSERT(!FIsWord("blanderbuss"));
    ASSERT(!FIsWord("a")); // one character words aren't allowed
    ASSERT(!FIsWord("i"));
    ASSERT(!PWordfragFollowPrepend('t', m_pWordfragRoot)->FIsWord());
    ASSERT(PWordfragFollowPrepend('a', PWordfragFollowPrepend('t', m_pWordfragRoot))->FIsWord());
}

bool SWordFrag::FIsWord()
{
    return (m_n & (1 << 7)) != 0;
}

int SWordFrag::CLinks()
{
    return m_n & ~(1 << 7);
}

int * SWordFrag::AI()
{
    int iOffset = sizeof(SWordFrag) + CLinks();
    int iOffsetOnFour = iOffset >> 2;
    int iOffsetRoundedDown = iOffsetOnFour << 2;
    if (iOffset != iOffsetRoundedDown)
    {
        iOffset += 4 - (iOffset - iOffsetRoundedDown);
    }
    return (int *)(this + iOffset);
}

SSolverBoard::SSolverBoard(SScrabbleGridHandle hGrid)
{
    SScrabbleGrid * pGrid = &*hGrid;
    for (int x = 0; x < DX_GRID; x++)
    {
        for (int y = 0; y < DY_GRID; y++)
        {
            int i = y * DX_GRID + x;
            SScrabbleTileHandle hTile = pGrid->m_ahTileGrid[i];
            if (const SScrabbleTile * pTile = hTile.PT())
            {
                m_aCh[i] = pTile->m_chLetter;
            }
            else
            {
                m_aCh[i] = '\0';
            }
        }
    }
}

SSolverBoard::SSolverBoard(const char * pChzBoardLayout)
{
    ASSERT(strlen(pChzBoardLayout) == DX_GRID * DY_GRID);
    for (int x = 0; x < DX_GRID; x++)
    {
        for (int y = 0; y < DY_GRID; y++)
        {
            // Reversing y so 0, 0 is the bottom left corner
            int iSrc = IChFromCoords(x, (DY_GRID - 1) - y);
            int iDst = IChFromCoords(x, y);
            char ch = pChzBoardLayout[iSrc];
            if (ch == '0')
                ch = '\0';
            else
                ch = ToLower(ch);
            m_aCh[iDst] = ch;
        }
    }
}

void SMove::PrependCh(char ch)
{
    if (m_fIsHorizontal)
        m_x--;
    else
        m_y++; // up is increasing, bb do we need to reverse whether we increment or decement the mic? Probably
    ASSERT(m_iChMic > 0);
	m_iChMic--;
	m_aCh[m_iChMic] = ch;
}

void SMove::AppendCh(char ch)
{
    ASSERT(m_iChMac < DIM(m_aCh));
	m_aCh[m_iChMac] = ch;
	m_iChMac++;
}

void SMove::UnPrependCh()
{
    if (m_fIsHorizontal)
        m_x++;
    else
        m_y--; // up is increasing Are there aother places we need to consider this for y? Maybe consider just letting top left be 0, 0, since then words go forward in both directions
	m_iChMic++;
}

void SMove::UnAppendCh()
{
	m_iChMac--;
}

SRack::SRack(const char * pChzRack)
{
    int cRack = strlen(pChzRack);
    ASSERT(cRack <= DIM(m_aryCh.m_a));
    for (int i = 0; i < cRack; i++)
    {
        m_aryCh.Append(ToLower(pChzRack[i]));
    }
}
