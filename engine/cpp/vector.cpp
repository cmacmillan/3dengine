
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

float4 float4::operator-() const
{
	return float4(-m_x, -m_y, -m_z, -m_w);
}

float4 operator*(float g, const float4& vec) 
{
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

// Vector

Vector VecUnitX() { return Vector(1, 0, 0); }
Vector VecUnitY() { return Vector(0, 1, 0); }
Vector VecUnitZ() { return Vector(0, 0, 1); }
Vector VecZero() { return Vector(0, 0, 0); }

Vector operator*(float g, const Vector & vec)
{
	return g * vec.m_vec;
}

Vector operator/(float g, const Vector & vec)
{
	return g / vec.m_vec;
}

float GDot(const Vector & vec0, const Vector & vec1)
{
	return GDot(vec0.m_vec, vec1.m_vec);
}

bool FIsNear(const Vector & vec0, const Vector & vec1)
{
	return FIsNear(vec0.m_vec, vec1.m_vec);
}

Vector::Vector(const float4 & vec) : 
	m_vec(vec) 
{ 
	ASSERT(vec.m_w == 0.0f); 
}

Vector::Vector(const Point & pos)
{
	m_vec = pos.m_vec;
	m_vec.m_w = 0.0f;
}

Vector Vector::operator+(const Vector & vec) const
{
	return m_vec + vec.m_vec;
}

Vector Vector::operator-(const Vector & vec) const
{
	return m_vec + -vec.m_vec;
}

Vector Vector::operator*(float g) const
{
	return m_vec * g;
}

Vector Vector::operator/(float g) const
{
	return m_vec / g;
}

Point Vector::operator+(const Point & pos) const
{
	return m_vec + pos.m_vec;
}

Vector Vector::operator-() const
{
	return -m_vec;
}

// Point

Point PosZero() { return Point(0, 0, 0); }

Point::Point(const float4 & vec) :
	m_vec(vec)
{
	ASSERT(vec.m_w == 1.0f);
}

Point::Point(const Vector & vec)
{
	m_vec = vec.m_vec;
	m_vec.m_w = 1.0f;
}

Point Point::operator+(const Vector & vec) const
{
	return m_vec + vec.m_vec;
}

Point Point::operator-(const Vector & vec) const
{
	return m_vec - vec.m_vec;
}

Vector Point::operator-(const Point & pos) const
{
	return m_vec - pos.m_vec;
}

bool FIsNear(const Point & pos0, const Point & pos1)
{
	return FIsNear(pos0.m_vec, pos1.m_vec);
}

// Mat

Mat::Mat()
{
	memset(this, 0, sizeof(Mat));
}

Mat MatTranslate(Point pos)
{
	return Mat(VecUnitX().m_vec, VecUnitY().m_vec, VecUnitZ().m_vec, pos.m_vec);
}

Mat MatTranslate(Vector vec)
{
	return Mat(VecUnitX().m_vec, VecUnitY().m_vec, VecUnitZ().m_vec, Point(vec).m_vec);
}

Mat MatScale(Vector vec)
{
	return Mat(
			vec.m_vec.m_x * VecUnitX().m_vec, 
			vec.m_vec.m_y * VecUnitY().m_vec, 
			vec.m_vec.m_z * VecUnitZ().m_vec, 
			float4(0, 0, 0, 1));
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

Vector operator*(const Vector & vec, const Mat & mat)
{
	return vec.m_vec * mat;
}

Point operator*(const Point & pos, const Mat & mat)
{
	return pos.m_vec * mat;
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

Quat Quat::operator*(const Quat & quat) const
{
	// i^2 = j^2 = k^2 = ijk = -1

	// By applying the above rule, keeping in mind that Quaternions are of the form Q = a + bi + cj + dk
	//  and then distributing you arrive at the following:

	return Quat(
				m_a * quat.m_a - m_b * quat.m_b - m_c * quat.m_c - m_d * quat.m_d,
				m_a * quat.m_b + m_b * quat.m_a + m_c * quat.m_d - m_d * quat.m_c,
				m_a * quat.m_c - m_b * quat.m_d + m_c * quat.m_a + m_d * quat.m_b,
				m_a * quat.m_d + m_b * quat.m_c - m_c * quat.m_b + m_d * quat.m_a);
}

Quat QuatAxisAngle(const Vector & normal, float radAngle)
{
	// Constructs a quat such that when a vec is rotated using the rotate function the appropriate axis angle will have been applied

	float4 vec = normal.m_vec * GSin(radAngle * .5f);

	return Quat(GCos(radAngle * .5), vec.m_x, vec.m_y, vec.m_z);
}

float4 Rotate(const float4 & vec, const Quat & quat)
{
	// Construct a copy with sign-flopped i,j,k

	Quat quatInv = Quat(quat.m_a, -quat.m_b, -quat.m_c, -quat.m_d);

	// Convert input point into a quat

	Quat quatPoint = Quat(0.0f, vec.m_x, vec.m_y, vec.m_z);

	Quat quatResult = quat * quatPoint * quatInv;

	return float4(quatResult.m_b, quatResult.m_c, quatResult.m_d, vec.m_w);
}

Vector Rotate(const Vector & vec, const Quat & quat)
{
	return Rotate(vec.m_vec, quat);
}

Point Rotate(const Point & pos, const Quat & quat)
{
	return Rotate(pos.m_vec, quat);
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
	Mat matResult = matA * matB;
	Mat matResultExpected = Mat(
								float4(2, 0, 2, 2),
								float4(16, -1, 14, 21),
								float4(0, 0, 0, 0),
								float4(2, 8, 6, 4));

	ASSERT(FIsNear(matResult, matResultExpected));

	float4 vec = float4(1, 2, 3, 4);

	float4 vecResult = vec * matB;
	float4 vecResultExpected = float4(16, -1, 14, 21);

	ASSERT(FIsNear(vecResult, vecResultExpected));

	Vector vecC = Vector(1, 2, 3);
	Vector vecD = Vector(-1, 5, 0);
	float gDotResult = GDot(vecC, vecD);
	float gDotResultExpected = 9;

	ASSERT(FIsNear(gDotResult, gDotResultExpected));

	Vector vecAddResult = vecC + vecD;
	Vector vecAddResultExpected = Vector(0, 7, 3);

	ASSERT(FIsNear(vecAddResult, vecAddResultExpected));

	Vector vecSubResult = vecC - vecD;
	Vector vecSubResultExpected = Vector(2, -3, 3);

	ASSERT(FIsNear(vecSubResult, vecSubResultExpected));

	Vector vecNegateResult = -vecC;
	Vector vecNegateResultExpected = Vector(-1, -2, -3);

	ASSERT(FIsNear(vecNegateResult, vecNegateResultExpected));

	Point posE = Point(10, -5, 4);
	Point posF = Point(5, 6, 7);

	Vector vecPosSubResult = posE - posF;
	Vector vecPosSubResultExpected = Vector(5, -11, -3);

	ASSERT(FIsNear(vecPosSubResult, vecPosSubResultExpected));

	Point posAddMulResult = posE + vecC * 10.0f;
	Point posAddMulResultOrder = vecC * 10.0f + posE;
	Point posAddMulResultExpected = Point(20.0f, 15.0f, 34.0f);

	ASSERT(FIsNear(posAddMulResult, posAddMulResultExpected));
	ASSERT(FIsNear(posAddMulResultOrder, posAddMulResultExpected));

	Mat matTranslate = MatTranslate(Point(10, -10, -2));

	Vector vecTranslateResult = vecC * matTranslate;
	Vector vecTranslateResultExpected = vecC;

	ASSERT(FIsNear(vecTranslateResult, vecC));

	Point posTranslateResult = posE * matTranslate;
	Point posTranslateResultExpected = Point(20, -15, 2);

	ASSERT(FIsNear(posTranslateResult, posTranslateResultExpected));

	Mat matScale = MatScale(Vector(1, 2, -3));

	Point posScaleResult = posE * matScale;
	Point posScaleResultExpected = Point(10, -10, -12);

	ASSERT(FIsNear(posScaleResult, posScaleResultExpected));

	Point posLerp0 = Point(-1, -1, -1);
	Point posLerp1 = Point(1, 1, 1);
	Point posLerpResult = Lerp(posLerp0, posLerp1, 0.5f);
	Point posLerpResultExpected = PosZero();

	ASSERT(FIsNear(posLerpResult, posLerpResultExpected));

	Point posRotate = Point(0, -1, 0);
	Point posRotateResultExpected = Point(0, 0, -1);
	Point posRotateResult = Rotate(posRotate, QuatAxisAngle(Vector(1, 0, 0), PI / 2.0f));

	ASSERT(FIsNear(posRotateResult, posRotateResultExpected));
}
