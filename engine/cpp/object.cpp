#include "object.h"

SObjectManager g_objman;

SObjectManager::SObjectManager()
{
	for (TYPEK typek = TYPEK_Min; typek < TYPEK_Max; typek = TYPEK(typek + 1))
	{
		m_mpTypekAryPObj[typek] = std::vector<SObject *>();
	}
}

TYPEK TypekSuper(TYPEK typek)
{
	switch (typek)
	{
		case TYPEK_Object:	return TYPEK_Nil;
			case TYPEK_Texture:		return TYPEK_Object;
			case TYPEK_Shader:		return TYPEK_Object;
			case TYPEK_Material:	return TYPEK_Object;
			case TYPEK_Node:		return TYPEK_Object;
				case TYPEK_UiNode:		return TYPEK_Node;
					case TYPEK_Text:		return TYPEK_UiNode;
					case TYPEK_Console:		return TYPEK_UiNode;
				case TYPEK_Node3D:		return TYPEK_Node;
					case TYPEK_DrawNode3D:	return TYPEK_Node3D;
						case TYPEK_GoalRing:	return TYPEK_DrawNode3D;
					case TYPEK_Camera3D:	return TYPEK_Node3D;
					case TYPEK_FlyCam:		return TYPEK_Node3D;
					case TYPEK_Sun:			return TYPEK_Node3D;
					case TYPEK_Player:		return TYPEK_Node3D;
					case TYPEK_PhysCube:	return TYPEK_Node3D;
					case TYPEK_DynSphere:	return TYPEK_Node3D;
			case TYPEK_Font:		return TYPEK_Object;
			case TYPEK_Mesh3D:		return TYPEK_Object;
			case TYPEK_FpsCounter:	return TYPEK_Node;

			default:
				{
					ASSERT(false);
					return TYPEK_Nil;
				}
	}
}

void SObjectManager::RegisterObj(SObject * pObj)
{
	int id = m_cId;
	m_cId++;
	ASSERT(pObj->m_ols == OBJECT_LIFE_STATE_Uninitialized);
	pObj->m_ols = OBJECT_LIFE_STATE_Registered;

	TYPEK typek = pObj->m_typek;
	while (typek != TYPEK_Nil)
	{
		m_mpTypekAryPObj[typek].push_back(pObj);
		typek = TypekSuper(typek);
	}

	m_mpObjhObj.emplace(id, pObj);
	pObj->m_nHandle = id;
}

void SObjectManager::UnregisterObj(SObject * pObj)
{
	ASSERT(pObj->m_ols == OBJECT_LIFE_STATE_Registered);

	TYPEK typek = pObj->m_typek;
	while (typek != TYPEK_Nil)
	{
		std::vector<SObject *> & aryPObj = m_mpTypekAryPObj[typek];
		bool fFound = false;
		SObject * pObjLast = aryPObj.size() > 1 ? aryPObj[aryPObj.size() - 1] : nullptr;
		for (int i = 0; i < aryPObj.size(); i++)
		{
			if (pObj == aryPObj[i])
			{
				fFound = true;
				if (pObjLast)
				{
					aryPObj[i] = pObjLast;
				}
				aryPObj.pop_back();
				break;
			}
		}
		ASSERT(fFound);

		typek = TypekSuper(typek);
	}

	pObj->m_ols = OBJECT_LIFE_STATE_Unregistered;
	m_mpObjhObj.erase(m_mpObjhObj.find(pObj->m_nHandle));
}

SObject::SObject(TYPEK typek)
{
	m_typek = typek;
	g_objman.RegisterObj(this);
}

SObject::~SObject()
{
	g_objman.UnregisterObj(this);
}

// BB could be faster and non-recursive

bool SObject::FIsDerivedFrom(TYPEK typek)
{
	for (TYPEK typekIter = m_typek; typekIter != TYPEK_Nil; typekIter = TypekSuper(typekIter))
	{
		if (typekIter == typek)
			return true;
	}

	return false;
}