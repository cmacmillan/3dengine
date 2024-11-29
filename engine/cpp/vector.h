#pragma once

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
	float4() : m_x(0.0f), m_y(0.0f), m_z(0.0f), m_w(0.0f) {}
	float4(const float4 & vec) : m_x(vec.m_x), m_y(vec.m_y), m_z(vec.m_z), m_w(vec.m_w) { }
	float4(float x, float y, float z, float w) : m_x(x), m_y(y), m_z(z), m_w(w) {}
	float m_x, m_y, m_z, m_w;
	float4 operator/(float g) const;
	float4 operator*(float g) const;
	float4 operator/(float4 vec) const;
	float4 operator*(float4 vec) const;
	float4 operator+(float4 vec) const;
	float4 operator-(float4 vec) const;
	float4 operator-() const;
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
	float	X() { return m_vec.m_x; }
	float	Y() { return m_vec.m_y; }
	float	Z() { return m_vec.m_z; }
	float4  m_vec;
	Vector operator+(const Vector & vec) const;
	Vector operator-(const Vector & vec) const;
	Vector operator*(float g) const;
	Vector operator/(float g) const;
	Point operator+(const Point & pos) const;
	Vector operator-() const;
};

Vector VecUnitX();
Vector VecUnitY();
Vector VecUnitZ();
Vector VecZero();

Vector operator*(float g, const Vector & vec);
Vector operator/(float g, const Vector & vec);
float GDot(const Vector & vec0, const Vector & vec1);
bool FIsNear(const Vector & vec0, const Vector & vec1);

struct Point
{
	Point() : m_vec() {}
	Point(const float4 & vec);
	Point(const Vector & vec);
	Point(float x, float y, float z) : m_vec(x, y, z, 1.0f) {}
	float	X() { return m_vec.m_x; }
	float	Y() { return m_vec.m_y; }
	float	Z() { return m_vec.m_z; }
	float4	m_vec;
	Point operator+(const Vector & vec) const;
	Point operator-(const Vector & vec) const;
	Vector operator-(const Point & pos) const;
};

Point PosZero();

bool FIsNear(const Point & pos0, const Point & pos1);

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
	float4 m_aVec[4];
	Mat operator*(const Mat & mat) const;
	Mat MatTranspose() const;
};

Mat MatTranslate(Point pos);
Mat MatTranslate(Vector vec);
Mat MatScale(Vector vec);

float4 operator*(const float4 & vec, const Mat & mat);
Vector operator*(const Vector & vec, const Mat & mat);
Point operator*(const Point & pos, const Mat & mat);

bool FIsNear(const Mat & mat0, const Mat & mat1);

struct Quat
{
	Quat(float a, float b, float c, float d) : m_a(a), m_b(b), m_c(c), m_d(d) {}

	// Quaternion = a + bi + cj + dk

	float m_a, m_b, m_c, m_d;

	Quat operator*(const Quat & quat) const;
};

// NOTE a positive radAngle = clockwise rotation about the normal since our coordinate system is right handed

Quat QuatAxisAngle(const Vector & normal, float radAngle);

float4 Rotate(const float4 & vec, const Quat & quat);
Vector Rotate(const Vector & vec, const Quat & quat);
Point Rotate(const Point & pos, const Quat & quat);

void AuditVectors();
