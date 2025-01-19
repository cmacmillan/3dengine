#include "node.h"

SNode::SNode(SNodeHandle hNodeParent, const std::string & str) :
	super()
{
	SetParent(hNodeParent);

	m_typek = TYPEK_Node;
	m_strName = str;
}

void SNode::SetParent(SNodeHandle hNodeParent)
{
	if (m_hNodeParent == hNodeParent)
		return;

	// Leave previous node

	if (m_hNodeParent != -1)
	{
		if (m_hNodeSiblingPrev != -1 && m_hNodeSiblingNext != -1)
		{
			// Have previous and next sibling

			m_hNodeSiblingPrev->m_hNodeSiblingNext = m_hNodeSiblingNext;
			m_hNodeSiblingNext->m_hNodeSiblingPrev = m_hNodeSiblingPrev;
			m_hNodeSiblingPrev = -1;
			m_hNodeSiblingNext = -1;
		}
		else if (m_hNodeSiblingPrev != -1 && m_hNodeSiblingNext == -1)
		{
			// Have previous sibling only (we were the last sibling)

			m_hNodeParent->m_hNodeChildLast = m_hNodeSiblingPrev;
			m_hNodeSiblingPrev->m_hNodeSiblingNext = -1;
			m_hNodeSiblingPrev = -1;
		}
		else if (m_hNodeSiblingPrev == -1 && m_hNodeSiblingNext != -1)
		{
			// Have next sibiling only (we were the first sibling)

			m_hNodeParent->m_hNodeChildFirst = m_hNodeSiblingNext;
			m_hNodeSiblingNext->m_hNodeSiblingPrev = -1;
			m_hNodeSiblingNext = -1;
		}
		else
		{
			// Have no siblings

			m_hNodeParent->m_hNodeChildFirst = -1;
			m_hNodeParent->m_hNodeChildLast = -1;
		}
	}

	m_hNodeParent = hNodeParent;

	// Enter new node

	if (m_hNodeParent != -1)
	{
		// Append

		// BB should probably eventually support setting specific index

		if (m_hNodeParent->m_hNodeChildLast != -1)
		{
			ASSERT(m_hNodeParent->m_hNodeChildFirst != -1);

			m_hNodeParent->m_hNodeChildLast->m_hNodeSiblingNext = HNode();
			m_hNodeSiblingPrev = m_hNodeParent->m_hNodeChildLast;
			m_hNodeParent->m_hNodeChildLast = HNode();
		}
		else
		{
			m_hNodeParent->m_hNodeChildFirst = HNode();
			m_hNodeParent->m_hNodeChildLast = HNode();
		}
	}
}
