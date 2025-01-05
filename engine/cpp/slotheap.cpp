#include "slotheap.h"
#include "util.h"
#include <vector>

void AuditSlotheap()
{
	// audit count and pointer aliasing

	{
		SSlotHeap<int> slotheapN;

		struct SPair
		{
			int * pN;
			int nCorrect;
		};

		std::vector<SPair> aryPair;

		for (int i = 0; i < 200; i++)
		{
			// push 20

			for (int j = 0; j < 20; j++)
			{
				int * pN = slotheapN.PTAlloc();
				int nCorrect = i * 10000 + j;
				*pN = nCorrect;
				aryPair.push_back({ pN, nCorrect });
			}

			// pop 10

			for (int j = 0; j < 10; j++)
			{
				int iRandish = i * 5357 + j * 137;
				int iInRange = iRandish % aryPair.size();
				slotheapN.FreePT(aryPair[iInRange].pN);

				aryPair.erase(aryPair.begin() + iInRange);
			}

			int cFree = 0;

			std::vector<SSlotHeapChunk<int> *> arySlotheapchunk;
			SSlotHeapChunk<int> * pSlotheapchunk = slotheapN.m_pSlotheapchunkFree;
			while (pSlotheapchunk)
			{
				arySlotheapchunk.push_back(pSlotheapchunk);

				SSlot<int> * pSlot = pSlotheapchunk->m_pSlotFree;
				while (pSlot)
				{
					cFree++;
					pSlot = (SSlot<int> *)pSlot->m_pV;
				}
				pSlotheapchunk = pSlotheapchunk->m_pSlotheapchunkNext;
			}

			for (SPair & pair : aryPair)
			{
				size_t cOffset = offsetof(SSlot<int>, m_t);
				SSlot<int> * pSlot = static_cast<SSlot<int> *>(static_cast<void *>(static_cast<char *>(static_cast<void *>(pair.pN)) - cOffset));

				bool fFound = false;
				for (SSlotHeapChunk<int> * pSlotheapchunkIter : arySlotheapchunk)
				{
					if (pSlotheapchunkIter == pSlot->m_pV)
					{
						fFound = true;
					}
				}

				if (!fFound)
				{
					arySlotheapchunk.push_back((SSlotHeapChunk<int> *)pSlot->m_pV);
				}
			}

			// Count up all the unique slotheapchunks by counting the free ones, and checking all the
			//  allocated things for which slotheap they are using

			ASSERT(cFree + aryPair.size() == arySlotheapchunk.size() * SLOTHEAPCHUNK_SIZE);

			for (SPair & pair : aryPair)
			{
				ASSERT(*pair.pN == pair.nCorrect);
			}
		}
	}
}
