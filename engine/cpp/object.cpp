#include "object.h"

SObjectManager g_objman;

SObjectManager::SObjectManager()
{
	size_t cbObj = sizeof(SObjectSlot) * C_OBJECT_MAX;
	m_aObjslot = (SObjectSlot *) malloc(cbObj);

	size_t cbH = sizeof(u32) * C_OBJECT_MAX;
	m_aiObjslotFree = (u32 *) malloc(cbH);
	for (u32 iObjslot = 0; iObjslot < C_OBJECT_MAX; iObjslot++)
	{
		m_aiObjslotFree[iObjslot] = iObjslot;
		m_aObjslot[iObjslot].m_h = HFromIObjslotAndNGeneration(iObjslot, 0);
		m_aObjslot[iObjslot].m_pObj = nullptr;
	}
	m_chFree = C_OBJECT_MAX;

	for (TYPEK typek = TYPEK_Min; typek < TYPEK_Max; typek = TYPEK(typek + 1))
	{
		m_mpTypekAryPObj[typek] = std::vector<SObject *>();
	}

	// Compute FIsDerivedFrom accelerator

	size_t cbTypekMap = TYPEK_Max * TYPEK_Max * sizeof(bool);
	m_mpTypekMpTypekFIsSuper = (bool *)malloc(cbTypekMap);
	for (TYPEK typek = TYPEK_Min; typek < TYPEK_Max; typek = TYPEK(typek + 1))
	{
		int iOffset = typek * TYPEK_Max;
		for (TYPEK typekSuper = TYPEK_Min; typekSuper < TYPEK_Max; typekSuper = TYPEK(typekSuper + 1))
		{
			m_mpTypekMpTypekFIsSuper[iOffset + typekSuper] = FIsDerivedFromSlow(typek, typekSuper);
		}
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

u32 NGenerationFromHandle(u64 h) {
	//            1 2 3 4 5 6 7 8
	return (h & 0xFFFFFFFF00000000) >> 32;
}

u32 IObjslotFromHandle(u64 h)
{
	//           1 2 3 4 5 6 7 8
	return h & 0x00000000FFFFFFFF;
}

u64 HFromIObjslotAndNGeneration(u32 iObjslot, u32 nGeneration)
{
	return (u64(nGeneration) << 32) | iObjslot;
}

void SObjectManager::RegisterObj(SObject * pObj)
{
	ASSERT(m_chFree > 0);
	m_chFree--;
	u32 iObjslot = m_aiObjslotFree[m_chFree];
	ASSERT(iObjslot < C_OBJECT_MAX);
	SObjectSlot * pObjslot = &m_aObjslot[iObjslot];

	ASSERT(pObj->m_ols == OBJECT_LIFE_STATE_Uninitialized);
	pObj->m_ols = OBJECT_LIFE_STATE_Registered;

	TYPEK typek = pObj->m_typek;
	while (typek != TYPEK_Nil)
	{
		m_mpTypekAryPObj[typek].push_back(pObj);
		typek = TypekSuper(typek);
	}

	pObjslot->m_pObj = pObj;
	pObj->m_h = pObjslot->m_h;
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

	u32 iObjslot = IObjslotFromHandle(pObj->m_h);
	u32 nGeneration = NGenerationFromHandle(pObj->m_h);

	SObjectSlot * pObjslot = &m_aObjslot[iObjslot];

	m_aiObjslotFree[m_chFree] = iObjslot;
	m_chFree++;
	
	pObjslot->m_h = HFromIObjslotAndNGeneration(iObjslot, nGeneration + 1);
	pObjslot->m_pObj = nullptr;
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

bool FIsDerivedFromSlow(TYPEK typek, TYPEK typekSuper)
{
	for (TYPEK typekIter = typek; typekIter != TYPEK_Nil; typekIter = TypekSuper(typekIter))
	{
		if (typekIter == typekSuper)
			return true;
	}

	return false;
}

bool SObject::FIsDerivedFrom(TYPEK typekSuper)
{
	return g_objman.m_mpTypekMpTypekFIsSuper[m_typek * TYPEK_Max + typekSuper];
}