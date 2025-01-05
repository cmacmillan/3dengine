#pragma once

#include <cstddef>
#include "assert.h"

template <typename T>
struct SSlot // slot
{
	T m_t;

	// When allocated points to the owning sslotheapchunk
	//  when free points to the next free slot

	void * m_pV;
};

#define SLOTHEAPCHUNK_SIZE 64

template <typename T>
struct SSlotHeapChunk	// slotheapchunk
{
	bool				FIsFull()
							{ return m_pSlotFree == nullptr; }

						SSlotHeapChunk()
	{
		for (int iSlot = 0; iSlot < SLOTHEAPCHUNK_SIZE; iSlot++)
		{
			m_aSlot[iSlot].m_pV = (iSlot < SLOTHEAPCHUNK_SIZE - 1) ? &m_aSlot[iSlot + 1] : nullptr;
		}

		m_pSlotFree = &m_aSlot[0];
	}

	T *					PTAlloc()
	{
		assert(m_pSlotFree);

		SSlot<T> * pSlot = m_pSlotFree;
		m_pSlotFree = static_cast<SSlot<T> *>(pSlot->m_pV);
		pSlot->m_pV = static_cast<void *>(this);
		assert(pSlot->m_pV);
		return &pSlot->m_t;
	}

	void				FreeSlot(SSlot<T> * pSlot)
	{
		pSlot->m_pV = m_pSlotFree;
		m_pSlotFree = pSlot;
	}

	SSlot<T> *			m_pSlotFree = nullptr;
	SSlot<T>			m_aSlot[SLOTHEAPCHUNK_SIZE];

	SSlotHeapChunk<T> *	m_pSlotheapchunkNext = nullptr;
};

template <typename T>
struct SSlotHeap // slotheap
{
						~SSlotHeap()
	{
		m_fIsDestructing = true;
		SSlotHeapChunk<T> * pSlotheapchunk = m_pSlotheapchunkFree;
		while(pSlotheapchunk)
		{
			SSlotHeapChunk<T> * pSlotheapchunkNext = pSlotheapchunk->m_pSlotheapchunkNext;
			delete pSlotheapchunk;
			pSlotheapchunk = pSlotheapchunkNext;
		}
	}

	T *					PTAlloc()
	{
		if (!m_pSlotheapchunkFree)
		{
			// Allocate a new chunk

			m_pSlotheapchunkFree = new SSlotHeapChunk<T>();
		}

		assert(!m_pSlotheapchunkFree->FIsFull());

		T * pT = m_pSlotheapchunkFree->PTAlloc();

		// When we go from not full to full, remove from linked list

		if (m_pSlotheapchunkFree->FIsFull())
		{
			m_pSlotheapchunkFree = m_pSlotheapchunkFree->m_pSlotheapchunkNext;
		}

		return pT;
	}

	void				FreePT(T * pT)
	{
		if (m_fIsDestructing) // If things try to unregister themselves because we destroy them while we're destroying ourselves, just ignore that
			return;

		size_t cOffset = offsetof(SSlot<T>, m_t);
		SSlot<T> * pSlot = static_cast<SSlot<T> *>(static_cast<void *>(static_cast<char *>(static_cast<void *>(pT)) - cOffset));
		assert(&pSlot->m_t == pT);
		SSlotHeapChunk<T> * pSlotheapchunk = static_cast<SSlotHeapChunk<T> *>(pSlot->m_pV);

		if (pSlotheapchunk->FIsFull())
		{
			// When we go from full to not full, get re-added to the linked list

			pSlotheapchunk->m_pSlotheapchunkNext = m_pSlotheapchunkFree;
			m_pSlotheapchunkFree = pSlotheapchunk;
		}

		pSlotheapchunk->FreeSlot(pSlot);
	}

	SSlotHeapChunk<T> *		m_pSlotheapchunkFree = nullptr;
	bool					m_fIsDestructing = false;
};


void AuditSlotheap();
