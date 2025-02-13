#pragma once

template<typename K, typename V>
struct SKv
{
	K m_key;
	V m_value;
};

template<typename K, typename V>
bool FTryGetValueFromKey(SKv<K, V> * aKv, int cKv, bool (*FCompare) (const K & key1, const K & key2), const K & key, V * pValue)
{
	for (int i = 0; i < cKv; i++)
	{
		if (FCompare(aKv[i].m_key, key))
		{
			*pValue = aKv[i].m_value;
			return true;
		}
	}

	return false;
}
