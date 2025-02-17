#pragma once

#include <string>

struct float2
{
	float2() : m_x(0.0f), m_y(0.0f) {}
	float2(float x, float y) : m_x(x), m_y(y) {}
	float m_x, m_y;
	float2 operator/(float g) const;
	float2 operator*(float g) const;
	float2 operator/(float2 vec) const;
	float2 operator*(float2 vec) const;
	float2 operator+(float2 vec) const;
	float2 operator-(float2 vec) const;
};

float2 operator*(float g, const float2 & vec);
float2 operator/(float g, const float2 & vec);

struct float4
{
				float4() : 
					m_x(0.0f), 
					m_y(0.0f), 
					m_z(0.0f), 
					m_w(0.0f) 
					{}

				float4(const float4 & vec) : 
					m_x(vec.m_x), 
					m_y(vec.m_y), 
					m_z(vec.m_z), 
					m_w(vec.m_w) 
					{}

				float4(float x, float y, float z, float w) : 
					m_x(x), 
					m_y(y), 
					m_z(z), 
					m_w(w) 
					{}

	float4		operator/(float g) const;
	float4		operator*(float g) const;
	float4		operator/(float4 vec) const;
	float4		operator*(float4 vec) const;
	float4		operator+(float4 vec) const;
	float4		operator-(float4 vec) const;
	float4		operator-() const;
	float &		operator[](int i);

	float4 &	operator*=(float g);
	float4 &	operator/=(float g);
	float4 &	operator+=(const float4 & vec);
	float4 &	operator-=(const float4 & vec);

	float m_x;
	float m_y; 
	float m_z;
	float m_w;
};

float4 operator*(float g, const float4 & vec);
float4 operator/(float g, const float4 & vec);
float GDot(const float4 & vec0, const float4 & vec1);

bool FIsNear(const float4 & vec0, const float4 & vec1);

struct Point; 

struct Vector
{
	Vector() : m_vec() {}
	Vector(const float4 & vec);
	Vector(const Point & pos);
	Vector(float x, float y, float z) : m_vec(x, y, z, 0.0f) {}
	float	SLength() const;
	Vector	VecNormalized() const;
	float	X() const { return m_vec.m_x; }
	float	Y() const { return m_vec.m_y; }
	float	Z() const { return m_vec.m_z; }
	float4  m_vec;
	Vector operator+(const Vector & vec) const;
	Vector operator-(const Vector & vec) const;
	Vector operator*(float g) const;
	Vector operator/(float g) const;
	Point operator+(const Point & pos) const;
	Vector operator-() const;
};

Vector VecPerpendicular(const Vector & vec);

Vector operator*(float g, const Vector & vec);
Vector operator/(float g, const Vector & vec);
float GDot(const Vector & vec0, const Vector & vec1);
Vector VecCross(const Vector & vec0, const Vector & vec1);
bool FIsNear(const Vector & vec0, const Vector & vec1);

struct Point
{
	Point() : m_vec() {}
	Point(const float4 & vec);
	Point(const Vector & vec);
	Point(float x, float y, float z) : m_vec(x, y, z, 1.0f) {}
	float	X() const { return m_vec.m_x; }
	float	Y() const { return m_vec.m_y; }
	float	Z() const { return m_vec.m_z; }
	float4	m_vec;
	Point operator+(const Vector & vec) const;
	Point operator-(const Vector & vec) const;
	Vector operator-(const Point & pos) const;
};

Point PosLerp(const Point & pos1, const Point pos2, float uLerp);
Vector VecLerp(const Vector & vec1, const Vector vec2, float uLerp);

Point PosZero();

bool FIsNear(const Point & pos0, const Point & pos1);

Vector VecComponentwiseMultiply(const Vector & vec1, const Vector & vec2);
Vector VecComponentwiseDivide(const Vector & vec1, const Vector & vec2);
Vector VecComponentwiseMin(const Vector & vec1, const Vector & vec2);
Vector VecComponentwiseMax(const Vector & vec1, const Vector & vec2);
Point PosComponentwiseMultiply(const Point & pos1, const Point & pos2);
Point PosComponentwiseDivide(const Point & pos1, const Point & pos2);
Point PosComponentwiseMin(const Point & pos1, const Point & pos2);
Point PosComponentwiseMax(const Point & pos1, const Point & pos2);

float SLength(const Vector & vec);
Vector VecNormalize(const Vector & vec);

// Row major, so vectors are horizontal
//  so multiplication goes left to right, like at work

// 4x4 matrix

struct Mat
{
	Mat();
	Mat(float4 r0, float4 r1, float4 r2, float4 r3) 
	{ 
		m_aVec[0] = r0; 
		m_aVec[1] = r1;
		m_aVec[2] = r2; 
		m_aVec[3] = r3; 
	}
	Mat(Vector vecX, Vector vecY, Vector vecZ, Point pos)
	{
		m_aVec[0] = vecX.m_vec;
		m_aVec[1] = vecY.m_vec;
		m_aVec[2] = vecZ.m_vec; 
		m_aVec[3] = pos.m_vec; 
	}
	Vector VecX() { return m_aVec[0]; }
	Vector VecY() { return m_aVec[1]; }
	Vector VecZ() { return m_aVec[2]; }
	Point Pos() { return m_aVec[3]; }
	float4 m_aVec[4];
	Mat operator*(const Mat & mat) const;
	Mat operator*(float g) const;
	Mat MatTranspose() const;
	Mat MatInverse() const;
};

const extern Mat g_matIdentity;
const extern Vector g_vecXAxis;
const extern Vector g_vecYAxis;
const extern Vector g_vecZAxis;
const extern Vector g_vecZero;
const extern Vector g_vecOne;
const extern Point g_posZero;

Mat operator*(float g, const Mat & mat);

float4 operator*(const float4 & vec, const Mat & mat);
Vector operator*(const Vector & vec, const Mat & mat);
Point operator*(const Point & pos, const Mat & mat);

bool FIsNear(const Mat & mat0, const Mat & mat1);

struct Quat
{
	Quat() : m_a(0.0f), m_b(0.0f), m_c(0.0f), m_d(0.0f) {}
	Quat(float a, float b, float c, float d) : m_a(a), m_b(b), m_c(c), m_d(d) {}

	Quat Inverse() const;
	float SLength() const;

	// Quaternion = a + bi + cj + dk

	float m_a, m_b, m_c, m_d;

	Quat operator*(const Quat & quat) const;
};

const extern Quat g_quatIdentity;

// NOTE a positive radAngle = clockwise rotation about the normal since our coordinate system is right handed

Quat QuatAxisAngle(const Vector & normal, float radAngle);
Quat QuatFromTo(const Vector & vecFrom, const Vector & vecTo);
Quat QuatLookAt(const Vector & vecForward, const Vector & vecUp);
Quat QuatMulRaw(const Quat & quatA, const Quat & quatB);

bool FIsNear(const Quat & quat1, const Quat & quat2);

float4 VecRotate(const float4 & vec, const Quat & quat);
Vector VecRotate(const Vector & vec, const Quat & quat);
Point PosRotate(const Point & pos, const Quat & quat);
Vector VecNormalize(const Vector & vec);
Vector VecNormalizeElse(const Vector & vec, const Vector & vecElse);

Vector VecProjectOnNormal(const Vector & vec, const Vector & normal);
Vector VecProjectOnTangent(const Vector & vec, const Vector & normal);
Vector VecReflect(const Vector & vecToReflect, const Vector & normal);

Mat MatTranslate(Point pos);
Mat MatTranslate(Vector vec);
Mat MatScale(Vector vec);
Mat MatRotate(Quat quat);
Mat MatInverse(const Mat & mat);

Quat QuatFromMatRot(const Mat & matRot);

Mat MatPerspective(float radFovHorizontal, float rAspectWidthOverHeight, float dXNearClip, float dXFarClip);
Mat MatOrthographic(float gScale, float rAspectWidthOverHeight, float xNearClip, float xFarClip);

struct Transform // tag = transform
{
	Transform() : m_pos(Point(0.0f, 0.0f, 0.0f)), m_quat(Quat(1.0f, 0.0f, 0.0f, 0.0f)), m_vecScale(Vector(1.0f, 1.0f, 1.0f)) {}

	struct Mat		Mat() const;
	struct Mat		MatInverse() const;

	Point	m_pos;
	Quat	m_quat;
	Vector	m_vecScale;
};

bool FIsNear(const Transform & transform1, const Transform & transform2);

std::string StrFromPoint(Point pos);
std::string StrFromVector(Vector vec);
std::string StrFromQuat(Quat quat);
std::string StrFromMat(Mat mat);

void AuditVectors();
