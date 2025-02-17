#include "node.h"

SNode::SNode(SNode * pNodeParent, const std::string & str, TYPEK typek) :
	super(typek)
{
	SetParent(pNodeParent);

	m_strName = str;
}

SNode::~SNode()
{
	// TODO reparent all children to the root node I guess?

	ASSERT(!m_pNodeChildFirst);

	SetParent(nullptr);
}

SNode * SNode::PNodeParent()
{
	return m_pNodeParent;
}

void SNode::SetParent(SNode * pNodeParent)
{
	if (m_pNodeParent == pNodeParent)
		return;

	// Leave previous node

	if (m_pNodeParent != nullptr)
	{
		if (m_pNodeSiblingPrev != nullptr && m_pNodeSiblingNext != nullptr)
		{
			// Have previous and next sibling

			m_pNodeSiblingPrev->m_pNodeSiblingNext = m_pNodeSiblingNext;
			m_pNodeSiblingNext->m_pNodeSiblingPrev = m_pNodeSiblingPrev;
			m_pNodeSiblingPrev = nullptr;
			m_pNodeSiblingNext = nullptr;
		}
		else if (m_pNodeSiblingPrev != nullptr && m_pNodeSiblingNext == nullptr)
		{
			// Have previous sibling only (we were the last sibling)

			m_pNodeParent->m_pNodeChildLast = m_pNodeSiblingPrev;
			m_pNodeSiblingPrev->m_pNodeSiblingNext = nullptr;
			m_pNodeSiblingPrev = nullptr;
		}
		else if (m_pNodeSiblingPrev == nullptr && m_pNodeSiblingNext != nullptr)
		{
			// Have next sibiling only (we were the first sibling)

			m_pNodeParent->m_pNodeChildFirst = m_pNodeSiblingNext;
			m_pNodeSiblingNext->m_pNodeSiblingPrev = nullptr;
			m_pNodeSiblingNext = nullptr;
		}
		else
		{
			// Have no siblings

			m_pNodeParent->m_pNodeChildFirst = nullptr;
			m_pNodeParent->m_pNodeChildLast = nullptr;
		}
	}

	m_pNodeParent = pNodeParent;

	// Enter new node

	if (m_pNodeParent != nullptr)
	{
		// Append

		// BB should probably eventually support setting specific index

		if (m_pNodeParent->m_pNodeChildLast != nullptr)
		{
			ASSERT(m_pNodeParent->m_pNodeChildFirst != nullptr);

			m_pNodeParent->m_pNodeChildLast->m_pNodeSiblingNext = this;
			m_pNodeSiblingPrev = m_pNodeParent->m_pNodeChildLast;
			m_pNodeParent->m_pNodeChildLast = this;
		}
		else
		{
			m_pNodeParent->m_pNodeChildFirst = this;
			m_pNodeParent->m_pNodeChildLast = this;
		}
	}
}
