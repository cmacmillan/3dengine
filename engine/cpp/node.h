#pragma once

#include "object.h"

struct SNode : SObject // node
{
	typedef SObject super;
	SNodeHandle HNode() { return (SNodeHandle) m_nHandle; }

	SNode(SNode * pNodeParent, const std::string & strName, TYPEK typek = TYPEK_Node);
	~SNode();
	SNode * PNodeParent();

	virtual void SetParent(SNode * pNodeParent);

	virtual void Update() {}

	std::string m_strName;

	// NOTE allowing these to be raw pointers since you're nodes are responsible for removing themselves
	//  from this data structure before they destruct

	SNode *		m_pNodeSiblingPrev = nullptr;
	SNode *		m_pNodeSiblingNext = nullptr;

	SNode *		m_pNodeChildFirst = nullptr;
	SNode *		m_pNodeChildLast = nullptr;

protected:
	SNode *		m_pNodeParent = nullptr;
};

