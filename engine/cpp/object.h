#pragma once

// Also just kinda including things that everything needs to be able to include, should find a better place

#include "Tracy.hpp"

#include "util.h"
#include "vector.h"

enum OBJECT_LIFE_STATE
{
	OBJECT_LIFE_STATE_Uninitialized = 0,
	OBJECT_LIFE_STATE_Registered = 1,
	OBJECT_LIFE_STATE_Unregistered = 2,
};

enum TYPEK
{
	// NOTE When adding new elements to this, make sure to add them to TypekSuper too!!!
	// NOTE When adding new elements to this, make sure to add them to TypekSuper too!!!
	// NOTE When adding new elements to this, make sure to add them to TypekSuper too!!!

	TYPEK_Object,
		TYPEK_Texture,
		TYPEK_Shader,
		TYPEK_Material,
		TYPEK_Node,
			TYPEK_UiNode,
				TYPEK_Text,
				TYPEK_Console,
			TYPEK_FpsCounter,
			TYPEK_Node3D,
				TYPEK_DrawNode3D,
					TYPEK_GoalRing,
				TYPEK_Camera3D,
				TYPEK_FlyCam,
				TYPEK_Sun,
				TYPEK_Player,
				TYPEK_PhysCube,
				TYPEK_DynSphere,
		TYPEK_Font,
		TYPEK_Mesh3D,

	TYPEK_Max,
	TYPEK_Min = 0,
	TYPEK_Nil = -1,

	// NOTE When adding new elements to this, make sure to add them to TypekSuper too!!!
	// NOTE When adding new elements to this, make sure to add them to TypekSuper too!!!
	// NOTE When adding new elements to this, make sure to add them to TypekSuper too!!!
};

struct SObject;

struct SObjectSlot
{
	// Only storing the genid wouldn't be any smaller due to alignment, 
	//  and I suspect checking equality against an idential value will be nice and fast
	//  whereas if this was just the genid we'd have to bit shift

	u64	m_h;
	SObject * m_pObj;
};

TYPEK TypekSuper(TYPEK typek);
bool FIsDerivedFromSlow(TYPEK typek, TYPEK typekSuper);

u32 NGenerationFromHandle(u64 h);
u32 IObjslotFromHandle(u64 h);
u64 HFromIObjslotAndNGeneration(u32 iObjslot, u32 nGeneration);

#define C_OBJECT_MAX 100000

struct SObjectManager
{
			SObjectManager();
	void	RegisterObj(SObject * pObj);
	void	UnregisterObj(SObject * pObj);

	// TODO make per-type iterator wrapper around m_mpTypekAryPObj

	SObjectSlot *						m_aObjslot = nullptr;
	std::vector<SObject *>				m_mpTypekAryPObj[TYPEK_Max] = {};
	bool *								m_mpTypekMpTypekFIsSuper;

protected:
	u32 *								m_aiObjslotFree = nullptr;
	int									m_chFree = -1;
};
extern SObjectManager g_objman;

//                           1 2 3 4 5 6 7 8
//#define HANDLE_GEN_ID_MASK 0x0000000000000000

template <typename T>
struct SHandle
{
	SHandle() : m_h(-1) {}
	SHandle(u64 h) : m_h(h) {}

	T * PT() const
	{
		ZoneScoped;

		if (m_h == -1)
			return nullptr;

		SObjectSlot * pObjslot = &g_objman.m_aObjslot[IObjslotFromHandle(m_h)];

		if (pObjslot->m_h != m_h) // Compare gen ids, don't waste time bit shifting
			return nullptr;

		return (T*)pObjslot->m_pObj;
	}

	T * operator->() const
	{
		return PT();
	}

	T & operator*() const
	{
		T * pT = PT();
		if (pT == nullptr)
		{
			// Deliberately crash
			*((char *)nullptr) = 0;
		}
		return *pT;
	}

	bool operator==(const SHandle<T> & hOther) const
	{
		return m_h == hOther.m_h;
	}

	bool operator==(u64 h) const
	{
		return m_h == h;
	}

	u64 m_h = -1;
};

template <typename T>
bool operator==(const void * pV, const SHandle<T> & hOther)
{
	return pV == hOther.PT();
}

template <typename T>
bool operator==(const u64 h, const SHandle<T> & hOther)
{
	return h == hOther.m_h;
}

struct SObject  // obj
{
	SObject(TYPEK typek = TYPEK_Object);
	~SObject();
	bool FIsDerivedFrom(TYPEK typek);
	TYPEK m_typek = TYPEK_Nil;
	OBJECT_LIFE_STATE m_ols = OBJECT_LIFE_STATE_Uninitialized;
	u64 m_h = -1;
};

struct SMesh3D;
typedef SHandle<SMesh3D> SMesh3DHandle;

struct SMesh2D;
typedef SHandle<SMesh2D> SMesh2DHandle;

struct STexture;
typedef SHandle<STexture> STextureHandle;

struct SShader;
typedef SHandle<SShader> SShaderHandle;

struct SMaterial;
typedef SHandle<SMaterial> SMaterialHandle;

struct SNode;
typedef SHandle<SNode> SNodeHandle; 

struct SNode3D;
typedef SHandle<SNode3D> SNode3DHandle;

struct SCamera3D;
typedef SHandle<SCamera3D> SCamera3DHandle;

struct SDrawNode3D;
typedef SHandle<SDrawNode3D> SDrawNode3DHandle;

struct SUiNode;
typedef SHandle<SUiNode> SUiNodeHandle;

struct SText;
typedef SHandle<SText> STextHandle;

struct SFont;
typedef SHandle<SFont> SFontHandle;

struct SFlyCam;
typedef SHandle<SFlyCam> SFlycamHandle;

struct SConsole;
typedef SHandle<SConsole> SConsoleHandle;

struct SGoalRing;
typedef SHandle<SGoalRing> SGoalRingHandle;

struct SSun;
typedef SHandle<SSun> SSunHandle;

struct SPlayer;
typedef SHandle<SPlayer> SPlayerHandle;

struct SPhysCube;
typedef SHandle<SPhysCube> SPhysCubeHandle;

struct SDynSphere;
typedef SHandle<SDynSphere> SDynSphereHandle;
