#pragma once

#include <unordered_map>

// Also just kinda including things that everything needs to be able to include, should find a better place

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

TYPEK TypekSuper(TYPEK typek);

struct SObject;
struct SObjectManager
{
			SObjectManager();
	void	RegisterObj(SObject * pObj);
	void	UnregisterObj(SObject * pObj);

	// TODO make per-type iterator wrapper around m_mpTypekAryPObj

	std::unordered_map<int, SObject *>	m_mpObjhObj;
	std::vector<SObject *>				m_mpTypekAryPObj[TYPEK_Max] = {};

	int									m_cId = 0;
};
extern SObjectManager g_objman;

template <typename T>
struct SHandle
{
	SHandle() : m_id(-1) {}
	SHandle(int id) : m_id(id) {}
	int m_id = -1;
	T * PT() const
	{
		auto kv = g_objman.m_mpObjhObj.find(m_id);
		if (kv == g_objman.m_mpObjhObj.end()) return nullptr;
		return (T *) kv->second;
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
		return m_id == hOther.m_id;
	}
	bool operator==(const void * pV) const
	{
		return PT() == pV;
	}
	bool operator==(int id) const
	{
		return m_id == id;
	}
};

template <typename T>
bool operator==(const void * pV, const SHandle<T> & hOther)
{
	return pV == hOther.PT();
}

template <typename T>
bool operator==(const int id, const SHandle<T> & hOther)
{
	return id == hOther.m_id;
}

struct SObject  // obj
{
	SObject(TYPEK typek = TYPEK_Object);
	~SObject();
	bool FIsDerivedFrom(TYPEK typek);
	TYPEK m_typek = TYPEK_Nil;
	OBJECT_LIFE_STATE m_ols = OBJECT_LIFE_STATE_Uninitialized;
	int m_nHandle = -1;
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
