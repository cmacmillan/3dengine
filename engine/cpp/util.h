#pragma once

#define NOMINMAX

#include <vector>
#include <locale>
#include <codecvt>
#include <string>
#include <assert.h>

#define ASSERT assert
#define CASSERT(arg) static_assert(arg, "static assert failed!\n");
#define DIM(a) sizeof(a)/sizeof(a[0])

// some stuff is inlined so the Scrabble (data structure generator project) can use it

inline float min(float g1, float g2)
{
	if (g1 < g2)
		return g1;
	return g2;
}

inline float max(float g1, float g2)
{
	if (g1 > g2)
		return g1;
	return g2;
}

template<typename T>
int IFind(const std::vector<T> & aryT, const T & t)
{
	for (int i = 0; i < aryT.size(); i++)
	{
		const T & l = aryT[i];
		if (l == t)
			return i;
	}
	return -1;
}

template <typename T>
T lerp(const T & t1, const T & t2, float lerp)
{
	lerp = max(min(lerp, 1.0f), 0.0f);
	return (t2 - t1) * lerp + t1;
}

float maprange(float a1, float a2, float b1, float b2, float g);

inline char ToLower(char ch)
{
	if (ch >= 'A' && ch <= 'Z')
	{
		return 'a' + ch - 'A';
	}
	return ch;
}

inline char ToUpper(char ch)
{
	if (ch >= 'a' && ch <= 'z')
	{
		return 'A' + ch - 'a';
	}
	return ch;
}

bool FIsUpper(char ch);
bool FIsLower(char ch);

std::wstring wstrFromStr(std::string str);

std::string strFromWstr(std::wstring wstr);

template<typename T, size_t C>
struct SFixArray
{
	void Append(const T & t);
	void Remove(int i);
	void RemoveSwap(int i);
	T operator [](int i) const;
	T & operator [](int i);
	T	m_a[C];
	int m_c = 0;
};

template<typename T, size_t C>
void SFixArray<T, C>::Append(const T & t)
{
	ASSERT(m_c < C);
	m_a[m_c] = t;
	m_c++;
}

template<typename T, size_t C>
void SFixArray<T, C>::Remove(int i)
{
	ASSERT(i >= 0);
	ASSERT(i < m_c);
	for (int iShift = i; iShift < m_c-1; iShift++)
	{
		m_a[iShift] = m_a[iShift + 1];
	}
	m_c--;
}

template<typename T, size_t C>
void SFixArray<T, C>::RemoveSwap(int i)
{
	ASSERT(i >= 0);
	ASSERT(i < m_c);
	m_a[i] = m_a[m_c - 1];
	m_c--;
}

template<typename T, size_t C>
T SFixArray<T, C>::operator[](int i) const
{
	ASSERT(i >= 0);
	ASSERT(i < m_c);
	return m_a[i];
}

template<typename T, size_t C>
T & SFixArray<T, C>::operator[](int i)
{
	ASSERT(i >= 0);
	ASSERT(i < m_c);
	return m_a[i];
}

template<typename T, size_t C>
int IFind(const SFixArray<T, C> & aryT, const T & t)
{
	for (int i = 0; i < aryT.m_c; i++)
	{
		const T & l = aryT[i];
		if (l == t)
			return i;
	}
	return -1;
}

void AuditFixArray();
