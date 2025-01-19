
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

float & float4::operator[](int i)
{
	switch (i)
	{
		case 0:
			return m_x;
		case 1:
			return m_y;
		case 2:
			return m_z;
		case 3:
			return m_w;
		default:
			ASSERT(false);
			return m_x;
	}
}

float4 & float4::operator*=(float g)
{
	*this = *this * g;
	return *this;
}

float4 & float4::operator/=(float g)
{
	*this = *this / g;
	return *this;
}

float4 & float4::operator+=(const float4 & vec)
{
	*this = *this + vec;
	return *this;
}

float4 & float4::operator-=(const float4 & vec)
{
	*this = *this - vec;
	return *this;
}

float GDot(const float4 & vec0, const float4 & vec1)
{
	return vec0.m_x * vec1.m_x + vec0.m_y * vec1.m_y + vec0.m_z * vec1.m_z + vec0.m_w * vec1.m_w;
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

Vector::Vector(const float4 & vec) : 
	m_vec(vec) 
{ 
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

Vector VecComponentwiseMultiply(const Vector & vec1, const Vector & vec2)
{
	return Vector(
				vec1.X() * vec2.X(),
				vec1.Y() * vec2.Y(),
				vec1.Z() * vec2.Z());
}

// Point

Point PosZero() { return Point(0, 0, 0); }

Point::Point(const float4 & vec) :
	m_vec(vec)
{
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

Point PosComponentwiseMultiply(const Point & pos, const Vector & vec)
{
	return Point(
				pos.X() * vec.X(),
				pos.Y() * vec.Y(),
				pos.Z() * vec.Z());
}

// Mat

Mat g_matIdentity = Mat(float4(1, 0, 0, 0), float4(0, 1, 0, 0), float4(0, 0, 1, 0), float4(0, 0, 0, 1));
Vector g_vecXAxis = Vector(1.0f, 0.0f, 0.0f);
Vector g_vecYAxis = Vector(0.0f, 1.0f, 0.0f);
Vector g_vecZAxis = Vector(0.0f, 0.0f, 1.0f);
Vector g_vecZero = Vector(0.0f, 0.0f, 0.0f);
Quat g_quatIdentity = Quat(1.0f, 0.0f, 0.0f, 0.0f);

Mat::Mat()
{
	memset(this, 0, sizeof(Mat));
}

Mat MatTranslate(Point pos)
{
	return Mat(g_vecXAxis.m_vec, g_vecYAxis.m_vec, g_vecZAxis.m_vec, pos.m_vec);
}

Mat MatTranslate(Vector vec)
{
	return Mat(g_vecXAxis.m_vec, g_vecYAxis.m_vec, g_vecZAxis.m_vec, Point(vec).m_vec);
}

Mat MatScale(Vector vec)
{
	return Mat(
			vec.m_vec.m_x * g_vecXAxis.m_vec, 
			vec.m_vec.m_y * g_vecYAxis.m_vec, 
			vec.m_vec.m_z * g_vecZAxis.m_vec, 
			float4(0, 0, 0, 1));
}

Mat MatRotate(Quat quat)
{
	return Mat(
			VecRotate(g_vecXAxis, quat).m_vec,
			VecRotate(g_vecYAxis, quat).m_vec,
			VecRotate(g_vecZAxis, quat).m_vec,
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

float SLength(const Vector & vec)
{
	return GSqrt(vec.X() * vec.X() + vec.Y() * vec.Y() + vec.Z() * vec.Z());
}

Vector VecNormalize(const Vector & vec)
{
	return vec / SLength(vec);
}

float Vector::SLength()
{
	return ::SLength(*this);
}

Vector Vector::VecNormalized()
{
	return VecNormalize(*this);
}

Point operator*(const Point & pos, const Mat & mat)
{
	return pos.m_vec * mat;
}

Mat MatInverse(const Mat & mat)
{
	return mat.MatInverse();
}

Mat Mat::MatInverse() const
{
	// https://numerical.recipes/book.html "Gauss-Jordan elimination"

	Mat matInverse = g_matIdentity;
	Mat matCopy = *this;

	int mpIRowCurrentIRowOriginal[] = { 0, 1, 2, 3 };

	// Take the largest value in the first row, swap it into position zero, divide that row by that value
	//  combine with other rows to zero out that column

	// The idea here is we have AY = 1, where Y is A^-1
	//  We can manipulate this equation to end up with something like 1 * Y' = 1' aka Y' = 1'
	//  Y' because we may have reordered the rows of Y
	//  1' because we may have combined rows of A to eliminate terms
	//  We can then unshuffle Y' & 1' to figure out what Y is

	for (int iDiag = 0; iDiag < 4; iDiag++)
	{
		float gBest = 0.0f;
		int iBest = -1;
		for (int iCol = iDiag; iCol < 4; iCol++)
		{
			float gCur = GAbs(matCopy.m_aVec[iDiag][iCol]);
			if (gCur > gBest)
			{
				gBest = gCur;
				iBest = iCol;
			}
		}

		if (iBest == -1)
		{
			// Degenerate matrix
			
			ASSERT(false);

			return g_matIdentity;
		}

		if (iDiag != iBest)
		{
			// Swap the columns of matCopy

			Swap(matCopy.m_aVec[0][iDiag], matCopy.m_aVec[0][iBest]);
			Swap(matCopy.m_aVec[1][iDiag], matCopy.m_aVec[1][iBest]);
			Swap(matCopy.m_aVec[2][iDiag], matCopy.m_aVec[2][iBest]);
			Swap(matCopy.m_aVec[3][iDiag], matCopy.m_aVec[3][iBest]);

			// Mark the rows of matInverse as having been swapped

			Swap(mpIRowCurrentIRowOriginal[iDiag], mpIRowCurrentIRowOriginal[iBest]);
		}

		// iDiag now contains the largest element, which is also nonzero

		float gDiag = matCopy.m_aVec[iDiag][iDiag];

		matCopy.m_aVec[iDiag] /= gDiag;
		matInverse.m_aVec[iDiag] /= gDiag;

		// Kill off the other rows

		for (int iRow = 0; iRow < 4; iRow++)
		{
			if (iRow == iDiag)
				continue;

			float gFactor = matCopy.m_aVec[iRow][iDiag];
			matCopy.m_aVec[iRow] -= gFactor * matCopy.m_aVec[iDiag];
			matInverse.m_aVec[iRow] -= gFactor * matInverse.m_aVec[iDiag];
		}
	}

	ASSERT(FIsNear(matCopy, g_matIdentity));

	// Swap the rows

	bool mpIRowFSwapHandled[] = { false, false, false, false };

	for (int iRow = 0; iRow < 4; iRow++)
	{
		if (mpIRowFSwapHandled[iRow] || mpIRowCurrentIRowOriginal[iRow] == iRow)
			continue;
		
		int iRowOther = mpIRowCurrentIRowOriginal[iRow];

		Swap(matInverse.m_aVec[iRow], matInverse.m_aVec[iRowOther]);

		mpIRowFSwapHandled[iRow] = true;
		mpIRowFSwapHandled[iRowOther] = true;
	}

	//Swap(matInverse.m_aVec

	return matInverse;
}

bool FIsNear(const float4 & vec0, const float4 & vec1)
{
	return	FIsNear(vec0.m_x, vec1.m_x) &&
			FIsNear(vec0.m_y, vec1.m_y) &&
			FIsNear(vec0.m_z, vec1.m_z) &&
			FIsNear(vec0.m_w, vec1.m_w);
}

bool FIsNear(const Vector & vec0, const Vector & vec1)
{
	return FIsNear(vec0.m_vec, vec1.m_vec);
}

bool FIsNear(const Point & pos0, const Point & pos1)
{
	return FIsNear(pos0.m_vec, pos1.m_vec);
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

bool FIsNear(const Quat & quat1, const Quat & quat2)
{
	return FIsNear(quat1.m_a, quat2.m_a) &&
			FIsNear(quat1.m_b, quat2.m_b) &&
			FIsNear(quat1.m_c, quat2.m_c) &&
			FIsNear(quat1.m_d, quat2.m_d);
}

bool FIsNear(const Transform & transform1, const Transform & transform2)
{
	return FIsNear(transform1.m_pos, transform2.m_pos) &&
			FIsNear(transform1.m_quat, transform2.m_quat) &&
			FIsNear(transform1.m_vecScale, transform2.m_vecScale);
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

Quat Quat::Inverse() const
{
	return Quat(m_a, -m_b, -m_c, -m_d);
}

Quat QuatAxisAngle(const Vector & normal, float radAngle)
{
	ASSERT(FIsNear(SLength(normal), 1.0f));

	// Constructs a quat such that when a vec is rotated using the rotate function the appropriate axis angle will have been applied

	float4 vec = normal.m_vec * GSin(radAngle * .5f);

	return Quat(GCos(radAngle * 0.5f), vec.m_x, vec.m_y, vec.m_z);
}

float4 VecRotate(const float4 & vec, const Quat & quat)
{
	// Convert input point into a quat

	Quat quatPoint = Quat(0.0f, vec.m_x, vec.m_y, vec.m_z);

	Quat quatResult = quat * quatPoint * quat.Inverse();

	return float4(quatResult.m_b, quatResult.m_c, quatResult.m_d, vec.m_w);
}

Vector VecRotate(const Vector & vec, const Quat & quat)
{
	return VecRotate(vec.m_vec, quat);
}

Point PosRotate(const Point & pos, const Quat & quat)
{
	return VecRotate(pos.m_vec, quat);
}

struct Mat Transform::Mat() const
{
	// BB this could almost surely be faster

	return MatScale(m_vecScale) * MatRotate(m_quat) * MatTranslate(m_pos);
}

struct Mat Transform::MatInverse() const
{
	// BB this could almost surely be faster

	return Mat().MatInverse();
}

Mat MatPerspective(float radFovHorizontal, float rAspectWidthOverHeight, float xNearClip, float xFarClip)
{
	// Derived in https://www.wolframcloud.com/env/chasemacmillan/3DPerspectiveProjection.nb

	// Produces a mat which transforms from right handed x forward, z up (y left)
	//  to right handed z forward, y up (x right)

	// NOTE (chasem) Because of the perspective divide, you can actually scale this matrix up by any scalar and get identical results	

	// NOTE (chasem) Some of the structure of this comes from the fact that we're coming from x=foward space, so there's kinda a swizzle matrix here

	float gCotan = 1.0f / GTan(radFovHorizontal / 2.0f);
	float gDXClip = 1.0f / (xFarClip - xNearClip);

	return Mat(
		float4(0.0f, 0.0f, xFarClip * gDXClip, 1.0f),
		float4(-gCotan, 0.0f, 0.0f, 0.0f),
		float4(0.0f, rAspectWidthOverHeight * gCotan, 0.0f, 0.0f),
		float4(0.0f, 0.0f, -xFarClip * xNearClip * gDXClip, 0.0f));
}

Mat Mat::operator*(float g) const
{
	return Mat(
		m_aVec[0]*g, 
		m_aVec[1]*g, 
		m_aVec[2]*g, 
		m_aVec[3]*g);
}

Mat operator*(float g, const Mat & mat)
{
	return mat * g;
}

std::string StrFromPoint(Point pos)
{
	return StrPrintf("[%.2f,%.2f,%.2f]", pos.X(), pos.Y(), pos.Z());
}

std::string StrFromVector(Vector vec)
{
	return StrPrintf("[%.2f,%.2f,%.2f]", vec.X(), vec.Y(), vec.Z());
}

std::string StrFromMat(Mat mat)
{
	return StrPrintf(
		"[%.2f,%.2f,%.2f,%.2f]\n[%.2f,%.2f,%.2f,%.2f]\n[%.2f,%.2f,%.2f,%.2f]\n[%.2f,%.2f,%.2f,%.2f]", 
		mat.m_aVec[0].m_x,mat.m_aVec[0].m_y,mat.m_aVec[0].m_z,mat.m_aVec[0].m_w,
		mat.m_aVec[1].m_x,mat.m_aVec[1].m_y,mat.m_aVec[1].m_z,mat.m_aVec[1].m_w,
		mat.m_aVec[2].m_x,mat.m_aVec[2].m_y,mat.m_aVec[2].m_z,mat.m_aVec[2].m_w,
		mat.m_aVec[3].m_x,mat.m_aVec[3].m_y,mat.m_aVec[3].m_z,mat.m_aVec[3].m_w);
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
	Point posRotateResult = PosRotate(posRotate, QuatAxisAngle(Vector(1, 0, 0), PI / 2.0f));

	ASSERT(FIsNear(posRotateResult, posRotateResultExpected));

	Transform transformA;
	transformA.m_pos = Point(0, 0, 10);
	transformA.m_vecScale = Vector(1, 2, 3);
	transformA.m_quat = QuatAxisAngle(Vector(1,0,0), PI / 2.0f);

	ASSERT(FIsNear(transformA.MatInverse(), transformA.Mat().MatInverse()));
	ASSERT(FIsNear(transformA.Mat() * transformA.MatInverse(), g_matIdentity));

	Vector vecTest = Vector(5, -2, -1);
	Quat quatTest = QuatAxisAngle(VecNormalize(Vector(1, 2, 3)), PI / 5.0f);
	ASSERT(FIsNear(VecRotate(vecTest, quatTest), vecTest * MatRotate(quatTest)));

	ASSERT(FIsNear(MatRotate(quatTest) * MatInverse(MatRotate(quatTest)), g_matIdentity));
}
