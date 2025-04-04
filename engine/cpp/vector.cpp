
#include "vector.h"
#include "string.h" // for memset
#include "util.h"
#include "engine.h"

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

float SLength(float2 vec)
{
	return GSqrt(vec.m_x * vec.m_x + vec.m_y * vec.m_y);
}

// float4

bool float4::FHasNans() const
{
	return isnan(m_x) || isnan(m_y) || isnan(m_z) || isnan(m_w);
}

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

bool float4::operator==(const float4 & vec)
{
	return	m_x == vec.m_x &&
			m_y == vec.m_y &&
			m_z == vec.m_z &&
			m_w == vec.m_w;
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

Vector VecCross(const Vector & vec0, const Vector & vec1)
{
	return Vector(
		vec0.Y() * vec1.Z() - vec0.Z() * vec1.Y(),
		vec0.Z() * vec1.X() - vec0.X() * vec1.Z(),
		vec0.X() * vec1.Y() - vec0.Y() * vec1.X());
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

bool Vector::operator==(const Vector & vec)
{
	return	m_vec.m_x == vec.m_vec.m_x &&
			m_vec.m_y == vec.m_vec.m_y &&
			m_vec.m_z == vec.m_vec.m_z;
}

Vector & Vector::operator+=(const Vector & vec)
{
	*this = *this + vec;
	return *this;
}

Vector & Vector::operator-=(const Vector & vec)
{
	*this = *this - vec;
	return *this;
}

Vector VecComponentwiseMultiply(const Vector & vec1, const Vector & vec2)
{
	return Vector(
				vec1.X() * vec2.X(),
				vec1.Y() * vec2.Y(),
				vec1.Z() * vec2.Z());
}

Vector VecComponentwiseDivide(const Vector & vec1, const Vector & vec2)
{
	return Vector(
				vec1.X() / vec2.X(),
				vec1.Y() / vec2.Y(),
				vec1.Z() / vec2.Z());
}

Vector VecComponentwiseMin(const Vector & vec1, const Vector & vec2)
{
	return Vector(
				GMin(vec1.X(), vec2.X()),
				GMin(vec1.Y(), vec2.Y()),
				GMin(vec1.Z(), vec2.Z()));
}

float GComponentwiseMin(const Vector & vec)
{
	return GMin(GMin(vec.X(), vec.Y()), vec.Z());
}

float GComponentwiseMax(const Vector & vec)
{
	return GMax(GMax(vec.X(), vec.Y()), vec.Z());
}

Vector VecComponentwiseMax(const Vector & vec1, const Vector & vec2)
{
	return Vector(
				GMax(vec1.X(), vec2.X()),
				GMax(vec1.Y(), vec2.Y()),
				GMax(vec1.Z(), vec2.Z()));
}

Point PosComponentwiseMultiply(const Point & pos1, const Point & pos2)
{
	return Point(
				pos1.X() * pos2.X(),
				pos1.Y() * pos2.Y(),
				pos1.Z() * pos2.Z());
}

Point PosComponentwiseDivide(const Point & pos1, const Point & pos2)
{
	return Point(
				pos1.X() / pos2.X(),
				pos1.Y() / pos2.Y(),
				pos1.Z() / pos2.Z());
}

Point PosComponentwiseMin(const Point & pos1, const Point & pos2)
{
	return Point(
				GMin(pos1.X(), pos2.X()),
				GMin(pos1.Y(), pos2.Y()),
				GMin(pos1.Z(), pos2.Z()));
}

Point PosComponentwiseMax(const Point & pos1, const Point & pos2)
{
	return Point(
				GMax(pos1.X(), pos2.X()),
				GMax(pos1.Y(), pos2.Y()),
				GMax(pos1.Z(), pos2.Z()));
}

Vector VectorFromVec4(const float4 & vec4)
{
	return Vector(vec4.m_x,vec4.m_y,vec4.m_z);
}

Point PointFromVec4(const float4 & vec4)
{
	return Point(vec4.m_x, vec4.m_y, vec4.m_z);
}

Vector VecPerpendicular(const Vector & vec)
{
	if (GAbs(GDot(vec, g_vecXAxis)) < 0.9f)
	{
		return VecCross(vec, g_vecXAxis);
	}

	return VecCross(vec, g_vecYAxis);
}

Vector VecCylind(float rad, float sXy, float sZ)
{
	return Vector(GCos(rad) * sXy, GSin(rad) * sXy, sZ);
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

bool Point::FIsZero()
{
	return m_vec.m_x == 0.0f && m_vec.m_y == 0.0f && m_vec.m_z == 0.0f;
}

Point Point::operator+(const Vector & vec) const
{
	return m_vec + vec.m_vec;
}

Point Point::operator-(const Vector & vec) const
{
	return m_vec - vec.m_vec;
}

Point Point::operator-() const
{
	return Point(float4(-m_vec.m_x, -m_vec.m_y, -m_vec.m_z, 1.0f));
}

Vector Point::operator-(const Point & pos) const
{
	return m_vec - pos.m_vec;
}

Point & Point::operator+=(const Vector & vec)
{
	*this = *this + vec;
	return *this;
}

Point & Point::operator-=(const Vector & vec)
{
	*this = *this - vec;
	return *this;
}

bool Point::operator==(const Point & pos)
{
	return	m_vec.m_x == pos.m_vec.m_x &&
			m_vec.m_y == pos.m_vec.m_y &&
			m_vec.m_z == pos.m_vec.m_z;
}

Point PosLerp(const Point & pos1, const Point pos2, float uLerp)
{
	uLerp = GMax(GMin(uLerp, 1.0f), 0.0f);
	return (1.0f - uLerp) * pos1 + uLerp * pos2;
}

Vector VecLerp(const Vector & vec1, const Vector vec2, float uLerp)
{
	uLerp = GMax(GMin(uLerp, 1.0f), 0.0f);
	return (1.0f - uLerp) * vec1 + uLerp * vec2;
}

Vector VecProjectOnNormal(const Vector & vec, const Vector & normal)
{
	return GDot(normal, vec) * normal;
}

Vector VecProjectOnTangent(const Vector & vec, const Vector & normal)
{
	return vec - VecProjectOnNormal(vec, normal);
}

// Mat

const Mat g_matIdentity = Mat(float4(1, 0, 0, 0), float4(0, 1, 0, 0), float4(0, 0, 1, 0), float4(0, 0, 0, 1));
const Vector g_vecXAxis = Vector(1.0f, 0.0f, 0.0f);
const Vector g_vecYAxis = Vector(0.0f, 1.0f, 0.0f);
const Vector g_vecZAxis = Vector(0.0f, 0.0f, 1.0f);
const Vector g_vecZero = Vector(0.0f, 0.0f, 0.0f);
const Vector g_vecOne = Vector(1.0f, 1.0f, 1.0f);
const Point g_posZero = Point(0.0f, 0.0f, 0.0f);
const Quat g_quatIdentity = Quat(1.0f, 0.0f, 0.0f, 0.0f);

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

float SLengthSqr(const Vector & vec)
{
	return vec.X() * vec.X() + vec.Y() * vec.Y() + vec.Z() * vec.Z();
}

Vector VecNormalize(const Vector & vec)
{
	return vec / SLength(vec);
}

Vector VecReflect(const Vector & vecToReflect, const Vector & normal)
{
	Vector vecAlign = VecProjectOnNormal(vecToReflect, normal);
	return (vecToReflect - vecAlign) - vecAlign;
}

Vector VecNormalizeSafe(const Vector & vec)
{
	return VecNormalizeElse(vec, g_vecXAxis);
}

Vector VecNormalizeElse(const Vector & vec, const Vector & vecElse)
{
	if (FIsNear(vec, g_vecZero))
	{
		return VecNormalize(vecElse);
	}

	return VecNormalize(vec);
}

void ClosestPointsOnTwoLines(Point pos0, Vector normal0, Point pos1, Vector normal1, Point * pPos0, Point * pPos1)
{
	float gDot = GDot(normal0, normal1);
	if (FIsNear(1.0f, gDot) || FIsNear(-1.0f, gDot))
	{
		// Parallel, so all points are equally close, choose two close points arbitrarily

		Vector vecProj = VecProjectOnNormal(pos0 - pos1, normal1);
		*pPos0 = pos0;
		*pPos1 = vecProj + pos1;
	}
	else
	{
		Vector normalCross = VecNormalize(VecCross(normal0, normal1));

		// P0(t) = pos0 + normal0*t
		// P1(u) = pos1 + normal1*u
		// P0(t) + normalCross*v = P1(u)
		// pos0 + normal0*t + normalCross*v = pos1 + normal1*u
		// pos0.x + normal0.x*t + normalCross.x*v = pos1.x + normal1.x*u
		// pos0.y + normal0.y*t + normalCross.y*v = pos1.y + normal1.y*u
		// pos0.z + normal0.z*t + normalCross.z*v = pos1.z + normal1.z*u

		// Took above, and derived the following with mathematica
			
		float gDenom = 
			+ normal0.Z() * normal1.Y() * normalCross.X() 
			- normal0.Y() * normal1.Z() * normalCross.X() 
			- normal0.Z() * normal1.X() * normalCross.Y() 
			+ normal0.X() * normal1.Z() * normalCross.Y() 
			+ normal0.Y() * normal1.X() * normalCross.Z() 
			- normal0.X() * normal1.Y() * normalCross.Z();

		float gTNum = 
			+ normal1.Z() * normalCross.Y() * pos0.X() 
			- normal1.Y() * normalCross.Z() * pos0.X() 
			- normal1.Z() * normalCross.X() * pos0.Y() 
			+ normal1.X() * normalCross.Z() * pos0.Y() 
			+ normal1.Y() * normalCross.X() * pos0.Z() 
			- normal1.X() * normalCross.Y() * pos0.Z() 
			- normal1.Z() * normalCross.Y() * pos1.X() 
			+ normal1.Y() * normalCross.Z() * pos1.X() 
			+ normal1.Z() * normalCross.X() * pos1.Y() 
			- normal1.X() * normalCross.Z() * pos1.Y() 
			- normal1.Y() * normalCross.X() * pos1.Z() 
			+ normal1.X() * normalCross.Y() * pos1.Z();

		float gUNum =
			+ normal0.Z() * normalCross.Y() * pos0.X()
			- normal0.Y() * normalCross.Z() * pos0.X()
			- normal0.Z() * normalCross.X() * pos0.Y()
			+ normal0.X() * normalCross.Z() * pos0.Y()
			+ normal0.Y() * normalCross.X() * pos0.Z()
			- normal0.X() * normalCross.Y() * pos0.Z()
			- normal0.Z() * normalCross.Y() * pos1.X()
			+ normal0.Y() * normalCross.Z() * pos1.X()
			+ normal0.Z() * normalCross.X() * pos1.Y()
			- normal0.X() * normalCross.Z() * pos1.Y()
			- normal0.Y() * normalCross.X() * pos1.Z()
			+ normal0.X() * normalCross.Y() * pos1.Z();

		float gVNum =
			+ normal0.Z() * normal1.Y() * pos0.X()
			- normal0.Y() * normal1.Z() * pos0.X()
			- normal0.Z() * normal1.X() * pos0.Y()
			+ normal0.X() * normal1.Z() * pos0.Y()
			+ normal0.Y() * normal1.X() * pos0.Z()
			- normal0.X() * normal1.Y() * pos0.Z()
			- normal0.Z() * normal1.Y() * pos1.X()
			+ normal0.Y() * normal1.Z() * pos1.X()
			+ normal0.Z() * normal1.X() * pos1.Y()
			- normal0.X() * normal1.Z() * pos1.Y()
			- normal0.Y() * normal1.X() * pos1.Z()
			+ normal0.X() * normal1.Y() * pos1.Z();

		float gT = -gTNum / gDenom;
		float gU = -gUNum / gDenom;
		float gV = -gVNum / gDenom;

		ASSERT(!isnan(gT) && !isnan(gU) && !isnan(gV));
		*pPos0 = pos0 + normal0 * gT;
		*pPos1 = pos1 + normal1 * gU;
	}
}

void ClosestPointsOnLineAndLineSegment(Point pos, Vector normal, Point posSegment0, Point posSegment1, Point * pPosLine, Point * pPosSegment)
{
	Vector dPosSegment = posSegment1 - posSegment0;
	float sSegment = SLength(dPosSegment);
	if (FIsNear(0.0f, sSegment))
	{
		*pPosSegment = posSegment0;
		*pPosLine = PosClosestOnLineToPoint(posSegment0, pos, normal);
		return;
	}
	Vector normalSegment = dPosSegment / sSegment;

	Point posLine0;
	Point posLine1;
	ClosestPointsOnTwoLines(pos, normal, posSegment0, normalSegment, &posLine0, &posLine1);

	// Clamp to segment

	float sAlongSegment = GDot(normalSegment, posLine1 - posSegment0);
	sAlongSegment = GClamp(sAlongSegment, 0.0f, sSegment);

	*pPosSegment = posSegment0 + sAlongSegment * normalSegment;
	*pPosLine = PosClosestOnLineToPoint(*pPosSegment, pos, normal);
}

Point PosClosestOnLineToPoint(Point pos, Point posLine, Vector normalLine)
{
	return VecProjectOnNormal(pos - posLine, normalLine) + posLine;
}

Point PosClosestOnLineSegmentToPoint(Point pos, Point posLineSeg0, Point posLineSeg1)
{
	Vector dPosSegment = posLineSeg1 - posLineSeg0;
	float sSegment = SLength(dPosSegment);
	if (FIsNear(0.0f, sSegment))
	{
		return posLineSeg0;
	}
	Vector normalSegment = dPosSegment / sSegment;

	Point posOnLine = PosClosestOnLineToPoint(pos, posLineSeg0, normalSegment);

	// Clamp to range

	float s = GDot(posOnLine-posLineSeg0, normalSegment);
	s = GClamp(s,0.0f,sSegment);
	return posLineSeg0 + s * normalSegment;
}

Point PosClosestInQuadToPoint(Point pos, Point posPlane0, Point posPlane1, Point posPlane2, Point posPlane3)
{
	// BB not ensuring there's no bow-tie or that the polygon is convex, this will return junk in those conditions

	TWEAKABLE float s_gEpsilon = .01f;
	ASSERT(SLength(posPlane1 - posPlane0) > s_gEpsilon && SLength(posPlane1 - posPlane2) > s_gEpsilon && SLength(posPlane2 - posPlane3) > s_gEpsilon && SLength(posPlane0 - posPlane3) > s_gEpsilon);

	Vector normal = VecNormalize(VecCross(posPlane2 - posPlane0, posPlane1 - posPlane0));

	ASSERT(GAbs(GDot(VecNormalize(posPlane3 - posPlane0), normal)) < s_gEpsilon);

	bool fOutside = false;
	float sBest = FLT_MAX;
	Point posBest = g_posZero;

	Point * apPos[4] = { &posPlane0, &posPlane1, &posPlane2,&posPlane3 };
	for (int ipPos = 0; ipPos < 4; ipPos++)
	{
		const Point & posFrom = *apPos[ipPos];
		const Point & posTo = *apPos[(ipPos + 1) % 4];
		Point posOnSeg = PosClosestOnLineSegmentToPoint(pos, posFrom, posTo);
		Vector dPos = pos - posOnSeg;

		const Point & posIn = *apPos[(ipPos + 2) % 4];
		Vector dPosIn = VecProjectOnTangent(posIn - posFrom, VecNormalize(posTo - posFrom));
		float gDot = GDot(dPosIn, dPos);
		if (gDot < 0.0f)
		{
			fOutside = true;
			float s = SLength(dPos);
			if (s < sBest)
			{
				sBest = s;
				posBest = posOnSeg;
			}
		}
	}

	if (fOutside)
	{
		return posBest;
	}

	return VecProjectOnTangent(pos - posPlane0, normal) + posPlane0;
}

float Vector::SLength() const
{
	return ::SLength(*this);
}

Vector Vector::VecNormalized() const
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
	//  Basically just the algorithm you learn in math class

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

bool FIsExactly(const Point & pos0, const Point & pos1)
{
	return false;
}

bool FIsExactly(const Vector & vec0, const Point & vec1)
{
	return false;
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

bool Mat::FHasNans() const
{
	return m_aVec[0].FHasNans() || m_aVec[1].FHasNans() || m_aVec[2].FHasNans() || m_aVec[3].FHasNans();
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

Quat QuatMulRaw(const Quat & quatA, const Quat & quatB)
{
	// Multiply quats without renormalizing

	// i^2 = j^2 = k^2 = ijk = -1

	// By applying the above rule, keeping in mind that Quaternions are of the form Q = a + bi + cj + dk
	//  and then distributing you arrive at the following:

	return Quat(
				quatA.m_a * quatB.m_a - quatA.m_b * quatB.m_b - quatA.m_c * quatB.m_c - quatA.m_d * quatB.m_d,
				quatA.m_a * quatB.m_b + quatA.m_b * quatB.m_a + quatA.m_c * quatB.m_d - quatA.m_d * quatB.m_c,
				quatA.m_a * quatB.m_c - quatA.m_b * quatB.m_d + quatA.m_c * quatB.m_a + quatA.m_d * quatB.m_b,
				quatA.m_a * quatB.m_d + quatA.m_b * quatB.m_c - quatA.m_c * quatB.m_b + quatA.m_d * quatB.m_a);
}

Quat Quat::operator*(const Quat & quat) const
{
	Quat quatRetr = QuatMulRaw(*this, quat);
	float sLength = quatRetr.SLength();
	return Quat(quatRetr.m_a / sLength, quatRetr.m_b / sLength, quatRetr.m_c / sLength, quatRetr.m_d / sLength);
}

Quat Quat::Inverse() const
{
	return Quat(m_a, -m_b, -m_c, -m_d);
}


float Quat::SLength() const
{
	return GSqrt(m_a * m_a + m_b * m_b + m_c * m_c + m_d * m_d);
}

bool Quat::FIsIdentity() const
{
	return m_a == 1.0f && m_b == 0.0f && m_c == 0.0f && m_d == 0.0f;
}

Quat QuatAxisAngle(const Vector & normal, float radAngle)
{
	// NOTE a positive radAngle = clockwise rotation about the normal since our coordinate system is right handed

	ASSERT(FIsNear(SLength(normal), 1.0f, .0001f));

	// Constructs a quat such that when a vec is rotated using the rotate function the appropriate axis angle will have been applied

	float4 vec = normal.m_vec * GSin(radAngle * .5f);

	return Quat(GCos(radAngle * 0.5f), vec.m_x, vec.m_y, vec.m_z);
}

Quat QuatFromTo(const Vector & vecFrom, const Vector & vecTo)
{
	ASSERT(FIsNear(SLength(vecFrom), 1.0f, .0001f));
	ASSERT(FIsNear(SLength(vecTo), 1.0f, .0001f));

	float gDot = GDot(vecFrom, vecTo);

	const float gEpsilon = .00001f;

	Quat quat;
	if (gDot > 1.0f - gEpsilon)
	{
		// Already match

		quat = g_quatIdentity;
	}
	else if (gDot < -1.0f + gEpsilon)
	{
		quat = QuatAxisAngle(VecNormalize(VecPerpendicular(vecFrom)), PI);
	}
	else
	{
		Vector vecCrossRotateForward = VecCross(vecFrom, vecTo);
		float s = vecCrossRotateForward.SLength();
		vecCrossRotateForward = vecCrossRotateForward / s;
		float rad = RadAsin(GClamp(s,-1.0f,1.0f));
		if (gDot < 0)
		{
			rad = PI - rad;
		}
		quat = QuatAxisAngle(vecCrossRotateForward, rad);
	}

	return quat;
}

Quat QuatFromMatRot(const Mat & matRot)
{
	ASSERT(FIsNear(SLength(matRot.m_aVec[0]), 1.0f));
	ASSERT(FIsNear(SLength(matRot.m_aVec[1]), 1.0f));
	ASSERT(FIsNear(SLength(matRot.m_aVec[2]), 1.0f));

	// From https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/

	Vector vec0 = matRot.m_aVec[0];
	Vector vec1 = matRot.m_aVec[1];
	Vector vec2 = matRot.m_aVec[2];

	float g00 = vec0.X();
	float g10 = vec0.Y();
	float g20 = vec0.Z();
	float g01 = vec1.X();
	float g11 = vec1.Y();
	float g21 = vec1.Z();
	float g02 = vec2.X();
	float g12 = vec2.Y();
	float g22 = vec2.Z();

	float gTr = g00 + g11 + g22;

	if (gTr > 0.0f)
	{
		float gS = GSqrt(gTr + 1.0f) * 2.0f;
		return Quat(
			.25f * gS, 
			(g21 - g12) / gS, 
			(g02 - g20) / gS,
			(g10 - g01) / gS);
	}
	else if ((g00 > g11) && (g00 > g22))
	{
		float gS = GSqrt(1.0f + g00 - g11 - g22) * 2.0f;
		return Quat(
			(g21 - g12) / gS,
			.25f * gS,
			(g01 + g10) / gS,
			(g02 + g20) / gS);
	}
	else if (g11 > g22)
	{
		float gS = GSqrt(1.0f + g11 - g00 - g22) * 2.0f;
		return Quat(
			(g02 - g20) / gS,
			(g01 + g10) / gS,
			.25f * gS,
			(g12 + g21) / gS);
	}
	else
	{
		float gS = GSqrt(1.0f + g22 - g00 - g11) * 2.0f;
		return Quat(
			(g10 - g01) / gS,
			(g02 + g20) / gS,
			(g12 + g21) / gS,
			.25f * gS);
	}
}

Quat QuatLookAt(const Vector & vecForward, const Vector & vecUp)
{
	ASSERT(FIsNear(SLength(vecForward), 1.0f));
	ASSERT(FIsNear(SLength(vecUp), 1.0f));

	// NOTE My initial implementation of this attempted to first rotate x to forward, then rotate the rotated z to up
	//  This mostly worked but had some precision issues. So just using this implementation instead, which essentially just constructs
	//  A quat from a rotation matrix. Should probably factor this into a QuatFromMat function or something

	Vector vecRight = VecNormalize(VecCross(vecForward, vecUp));
	Vector vecUpAdjusted = VecNormalize(VecCross(vecRight, vecForward));
	Vector vecLeft = -vecRight;

	return QuatFromMatRot(Mat(vecForward, vecLeft, vecUpAdjusted, g_posZero));
}

float4 VecRotate(const float4 & vec, const Quat & quat)
{
	// Convert input point into a quat

	Quat quatPoint = Quat(0.0f, vec.m_x, vec.m_y, vec.m_z);

	Quat quatResult = QuatMulRaw(QuatMulRaw(quat, quatPoint), quat.Inverse());

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

	// inverted z

	return Mat(
		float4(0.0f, 0.0f, -xNearClip * gDXClip, 1.0f),
		float4(-gCotan, 0.0f, 0.0f, 0.0f),
		float4(0.0f, rAspectWidthOverHeight * gCotan, 0.0f, 0.0f),
		float4(0.0f, 0.0f, xFarClip * xNearClip * gDXClip, 0.0f));
}

Mat MatOrthographic(float gScale, float rAspectWidthOverHeight, float xNearClip, float xFarClip)
{
	// Derived in a simliar fashion to MatPerspective

	// Where gScale is the horizontal width  (not 2*gScale)

	float dX = xFarClip - xNearClip;

	return Mat(
		float4(0.0f, 0.0f, -1.0f / xFarClip, 0.0f),
		float4(-2 * dX / (xFarClip * gScale), 0.0f, 0.0f, 0.0f),
		float4(0.0f, 2.0f * rAspectWidthOverHeight * dX / (xFarClip * gScale), 0.0f, 0.0f),
		float4(0.0f, 0.0f, 1.0f, dX / xFarClip));
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

std::string StrFromQuat(Quat quat)
{
	return StrPrintf("[%.4f,%.4f,%.4f,%.4f]", quat.m_a, quat.m_b, quat.m_c, quat.m_d);
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

	ASSERT(FIsNear(VecCross(g_vecXAxis, g_vecYAxis), g_vecZAxis));
	ASSERT(FIsNear(VecCross(-g_vecYAxis, g_vecXAxis), g_vecZAxis));
	ASSERT(FIsNear(VecRotate(g_vecXAxis, QuatFromTo(g_vecXAxis, g_vecYAxis)), g_vecYAxis));

	ASSERT(FIsNear(VecRotate(g_vecZAxis, QuatAxisAngle(g_vecXAxis, -PI / 2.0f)), g_vecYAxis));
	ASSERT(FIsNear(VecRotate(g_vecXAxis, QuatAxisAngle(g_vecZAxis, PI / 2.0f)), g_vecYAxis));
}

float GScaleMaxFromMat(const Mat & mat)
{
	float gScaleMax = SLength(Vector(mat.m_aVec[0].m_x, mat.m_aVec[0].m_y, mat.m_aVec[0].m_z));
	gScaleMax = GMax(gScaleMax,SLength(Vector(mat.m_aVec[1].m_x, mat.m_aVec[1].m_y, mat.m_aVec[1].m_z)));
	gScaleMax = GMax(gScaleMax,SLength(Vector(mat.m_aVec[2].m_x, mat.m_aVec[2].m_y, mat.m_aVec[2].m_z)));
	return gScaleMax;
}
