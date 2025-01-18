#pragma once

#include "object.h"

struct SVertData3D
{
	Point	m_pos;
	float2	m_uv;
};
CASSERT(sizeof(SVertData3D) == 24);

struct SVertData2D
{
	float2	m_pos;
	float2	m_uv;
};
CASSERT(sizeof(SVertData2D) == 16);

void PushQuad2D(float2 posMin, float2 posMax, float2 uvMin, float2 uvMax, std::vector<SVertData2D> * paryVertdata, std::vector<unsigned short> * paryIIndex);
void PushQuad3D(std::vector<SVertData3D> * paryVertdata, std::vector<unsigned short> * paryIIndex);

struct SMesh3D : SObject // mesh
{
	typedef SObject super;
	SMesh3D();
	SMesh3DHandle HMesh() { return (SMesh3DHandle) m_nHandle; }

	std::vector<SVertData3D>		m_aryVertdata;
	std::vector<unsigned short>		m_aryIIndex;

	// Data used while rendering only
	//  This could potentially be factored out and stored as a pointer or something

	unsigned int					m_iVertdata = -1;
	unsigned int					m_cVerts = -1;
	unsigned int					m_iIndexdata = -1;
	unsigned int					m_cIndicies= -1;
};

struct SMesh2D : SObject // mesh
{
	typedef SObject super;
	SMesh2D();
	SMesh2DHandle HMesh() { return (SMesh2DHandle) m_nHandle; }

	std::vector<SVertData2D>		m_aryVertdata;
	std::vector<unsigned short>		m_aryIIndex;

	// Data used while rendering only
	//  This could potentially be factored out and stored as a pointer or something

	unsigned int					m_iVertdata = -1;
	unsigned int					m_cVerts = -1;
	unsigned int					m_iIndexdata = -1;
	unsigned int					m_cIndicies= -1;
};

