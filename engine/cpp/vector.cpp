
#include "vector.h"
#include "string.h" // for memset
#include "util.h"

// float 2

float2 float2::operator/(float g) const
{
	return float2(m_x / g, m_y / g);
}

float2 float2::operator*(float g) const
{
	return float2(m_x * g, m_y * g);
}

float2 float2::operator/(float2 vec) const
{
	return float2(m_x / vec.m_x, m_y / vec.m_y);
}

float2 float2::operator*(float2 vec) const
{
	return float2(m_x * vec.m_x, m_y * vec.m_y);
}

float2 float2::operator+(float2 vec) const
{
	return float2(m_x + vec.m_x, m_y + vec.m_y);
}

float2 float2::operator-(float2 vec) const
{
	return float2(m_x - vec.m_x, m_y - vec.m_y);
}

float2 operator*(float g, const float2& vec) {
	return vec * g;
}

float2 operator/(float g, const float2 & vec)
{
	return float2(g / vec.m_x,  g / vec.m_y);
}

// float4

float4 float4::operator/(float g) const
{
	return float4(m_x / g, m_y / g, m_z / g, m_w / g);
}

float4 float4::operator*(float g) const
{
	return float4(m_x * g, m_y * g, m_z * g, m_w * g);
}

float4 float4::operator/(float4 vec) const
{
	return float4(m_x / vec.m_x, m_y / vec.m_y, m_z / vec.m_z, m_w / vec.m_w);
}

float4 float4::operator*(float4 vec) const
{
	return float4(m_x * vec.m_x, m_y * vec.m_y, m_z * vec.m_z, m_w * vec.m_w);
}

float4 float4::operator+(float4 vec) const
{
	return float4(m_x + vec.m_x, m_y + vec.m_y, m_z + vec.m_z, m_w + vec.m_w);
}

float4 float4::operator-(float4 vec) const
{
	return float4(m_x - vec.m_x, m_y - vec.m_y, m_z - vec.m_z, m_w - vec.m_w);
}

float4 operator*(float g, const float4& vec) {
	return vec * g;
}

float4 operator/(float g, const float4 & vec)
{
	return float4(g / vec.m_x,  g / vec.m_y, g / vec.m_z, g / vec.m_w);
}

float GDot(const float4 & vec0, const float4 & vec1)
{
	return vec0.m_x * vec1.m_x + vec0.m_y * vec1.m_y + vec0.m_z * vec1.m_z + vec0.m_w * vec1.m_w;
}

bool FIsNear(const float4 & vec0, const float4 & vec1)
{
	return	FIsNear(vec0.m_x, vec1.m_x) &&
			FIsNear(vec0.m_y, vec1.m_y) &&
			FIsNear(vec0.m_z, vec1.m_z) &&
			FIsNear(vec0.m_w, vec1.m_w);
}

// Mat

Mat::Mat()
{
	memset(this, 0, sizeof(Mat));
}

float4 operator*(const float4 & vec, const Mat & mat)
{
	Mat matTranspose = mat.MatTranspose();

	return float4(
				GDot(matTranspose.m_aVec[0], vec),
				GDot(matTranspose.m_aVec[1], vec), 
				GDot(matTranspose.m_aVec[2], vec), 
				GDot(matTranspose.m_aVec[3], vec));
}

bool FIsNear(const Mat & mat0, const Mat & mat1)
{
	for (int i = 0; i < 4; i++)
	{
		if (!FIsNear(mat0.m_aVec[i], mat1.m_aVec[i]))
			return false;
	}

	return true;
}

Mat Mat::operator*(const Mat & mat) const
{
	Mat matRetr;
	Mat matTranspose = mat.MatTranspose();
	for (int i = 0; i < 4; i++)
	{
		matRetr.m_aVec[i] = float4(
								GDot(matTranspose.m_aVec[0], m_aVec[i]), 
								GDot(matTranspose.m_aVec[1], m_aVec[i]), 
								GDot(matTranspose.m_aVec[2], m_aVec[i]), 
								GDot(matTranspose.m_aVec[3], m_aVec[i]));
	}
	return matRetr;
}

Mat Mat::MatTranspose() const
{
	float4 vecCol0 = float4(m_aVec[0].m_x, m_aVec[1].m_x, m_aVec[2].m_x, m_aVec[3].m_x);
	float4 vecCol1 = float4(m_aVec[0].m_y, m_aVec[1].m_y, m_aVec[2].m_y, m_aVec[3].m_y);
	float4 vecCol2 = float4(m_aVec[0].m_z, m_aVec[1].m_z, m_aVec[2].m_z, m_aVec[3].m_z);
	float4 vecCol3 = float4(m_aVec[0].m_w, m_aVec[1].m_w, m_aVec[2].m_w, m_aVec[3].m_w);
	return Mat(vecCol0, vecCol1, vecCol2, vecCol3);
}



void AuditVectors()
{
	Mat matA = Mat(
					float4(1,0,0,0),
					float4(1,2,3,4),
					float4(0,0,0,0),
					float4(0,0,0,-2));
	Mat matB = Mat(
					float4(2,0,2,2),
					float4(3,0,3,3),
					float4(4,5,6,7),
					float4(-1,-4,-3,-2));
	Mat matResultExpected = Mat(
								float4(2, 0, 2, 2),
								float4(16, -1, 14, 21),
								float4(0, 0, 0, 0),
								float4(2, 8, 6, 4));

	Mat matResult = matA * matB;

	ASSERT(FIsNear(matResult, matResultExpected));
}
