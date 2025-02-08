#pragma once

#include "object.h"

struct SVertData3D
{
	Point	m_pos;
	Vector	m_normal;
	float2	m_uv;
};
CASSERT(sizeof(SVertData3D) == 40); // Update shaders & input layout

void PushQuad2D(float2 posMin, float2 posMax, float2 uvMin, float2 uvMax, std::vector<SVertData3D> * paryVertdata, std::vector<unsigned short> * paryIIndex);
void PushQuad3D(std::vector<SVertData3D> * paryVertdata, std::vector<unsigned short> * paryIIndex);

struct SMesh3D : SObject // mesh
{
	typedef SObject super;
	SMesh3D(TYPEK typek = TYPEK_Mesh3D);
	SMesh3DHandle HMesh() { return (SMesh3DHandle) m_nHandle; }

	std::string						m_strName = "";

	std::vector<SVertData3D>		m_aryVertdata = {};
	std::vector<unsigned short>		m_aryIIndex = {};

	// Data used while rendering only
	//  This could potentially be factored out and stored as a pointer or something

	unsigned int					m_iVertdata = -1;
	unsigned int					m_cVerts = -1;
	unsigned int					m_iIndexdata = -1;
	unsigned int					m_cIndicies= -1;
};
