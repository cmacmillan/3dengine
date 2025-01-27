#include "mesh.h"

void PushQuad2D(float2 posMin, float2 posMax, float2 uvMin, float2 uvMax, std::vector<SVertData3D> * paryVertdata, std::vector<unsigned short> * paryIIndex)
{
	int iVertexStart = paryVertdata->size();
	paryIIndex->push_back(iVertexStart);
	paryIIndex->push_back(iVertexStart+1);
	paryIIndex->push_back(iVertexStart+2);
	paryIIndex->push_back(iVertexStart+3);
	paryIIndex->push_back(iVertexStart+4);
	paryIIndex->push_back(iVertexStart+5);

	paryVertdata->push_back({ {Point(posMin.m_x, posMax.m_y,0)}, g_vecZAxis, {uvMin.m_x, uvMin.m_y} });
	paryVertdata->push_back({ {Point(posMin.m_x, posMin.m_y,0)}, g_vecZAxis, {uvMin.m_x, uvMax.m_y} });
	paryVertdata->push_back({ {Point(posMax.m_x, posMin.m_y,0)}, g_vecZAxis, {uvMax.m_x, uvMax.m_y} });
	paryVertdata->push_back({ {Point(posMin.m_x, posMax.m_y,0)}, g_vecZAxis, {uvMin.m_x, uvMin.m_y} });
	paryVertdata->push_back({ {Point(posMax.m_x, posMin.m_y,0)}, g_vecZAxis, {uvMax.m_x, uvMax.m_y} });
	paryVertdata->push_back({ {Point(posMax.m_x, posMax.m_y,0)}, g_vecZAxis, {uvMax.m_x, uvMin.m_y} });
}

void PushQuad3D(std::vector<SVertData3D> * paryVertdata, std::vector<unsigned short> * paryIIndex)
{
	int iVertexStart = paryVertdata->size();
	paryIIndex->push_back(iVertexStart);
	paryIIndex->push_back(iVertexStart+2);
	paryIIndex->push_back(iVertexStart+1);
	paryIIndex->push_back(iVertexStart);
	paryIIndex->push_back(iVertexStart+3);
	paryIIndex->push_back(iVertexStart+2);

	Point posLowerRight = Point(0.0f, -1.0f, -1.0f);
	Point posLowerLeft = Point(0.0f, 1.0f, -1.0f);
	Point posUpperRight = Point(0.0f, -1.0f, 1.0f);
	Point posUpperLeft = Point(0.0f, 1.0f, 1.0f);

	paryVertdata->push_back({ posLowerLeft,  -g_vecXAxis, float2(0.0f, 0.0f) });
	paryVertdata->push_back({ posLowerRight, -g_vecXAxis, float2(1.0f, 0.0f) });
	paryVertdata->push_back({ posUpperRight, -g_vecXAxis ,float2(1.0f, 1.0f) });
	paryVertdata->push_back({ posUpperLeft, -g_vecXAxis ,float2(0.0f, 1.0f) });
}

SMesh3D::SMesh3D() : super()
{
	m_typek = TYPEK_Mesh3D;
}

