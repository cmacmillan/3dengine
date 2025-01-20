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

	paryVertdata->push_back({ {Point(posMin.m_x, posMax.m_y,0)}, {uvMin.m_x, uvMin.m_y} });
	paryVertdata->push_back({ {Point(posMax.m_x, posMin.m_y,0)}, {uvMax.m_x, uvMax.m_y} });
	paryVertdata->push_back({ {Point(posMin.m_x, posMin.m_y,0)}, {uvMin.m_x, uvMax.m_y} });
	paryVertdata->push_back({ {Point(posMin.m_x, posMax.m_y,0)}, {uvMin.m_x, uvMin.m_y} });
	paryVertdata->push_back({ {Point(posMax.m_x, posMax.m_y,0)}, {uvMax.m_x, uvMin.m_y} });
	paryVertdata->push_back({ {Point(posMax.m_x, posMin.m_y,0)}, {uvMax.m_x, uvMax.m_y} });
}

void PushVert(Point pos, float2 uv, std::vector<SVertData3D> * paryVertdata, int * pI)
{
	SVertData3D * vertdata = &(*paryVertdata)[(*pI)++];
	vertdata->m_pos = pos;
	vertdata->m_uv = uv;
}

void PushQuad3D(std::vector<SVertData3D> * paryVertdata, std::vector<unsigned short> * paryIIndex)
{
	int iVertexStart = paryVertdata->size();
	paryIIndex->push_back(iVertexStart);
	paryIIndex->push_back(iVertexStart+1);
	paryIIndex->push_back(iVertexStart+2);
	paryIIndex->push_back(iVertexStart+3);
	paryIIndex->push_back(iVertexStart+4);
	paryIIndex->push_back(iVertexStart+5);

	Point posLowerLeft = Point(0.0f, -1.0f, -1.0f);
	Point posLowerRight = Point(0.0f, 1.0f, -1.0f);
	Point posUpperLeft = Point(0.0f, -1.0f, 1.0f);
	Point posUpperRight = Point(0.0f, 1.0f, 1.0f);

	paryVertdata->push_back({ posUpperRight, float2(1.0f, 1.0f) });
	paryVertdata->push_back({ posLowerLeft, float2(0.0f, 0.0f) });
	paryVertdata->push_back({ posLowerRight, float2(1.0f, 0.0f) });
	paryVertdata->push_back({ posLowerLeft, float2(0.0f, 0.0f) });
	paryVertdata->push_back({ posUpperRight, float2(1.0f, 1.0f) });
	paryVertdata->push_back({ posUpperLeft, float2(0.0f, 1.0f) });
}

SMesh3D::SMesh3D() : super()
{
	m_typek = TYPEK_Mesh3D;
}
