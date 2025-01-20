#pragma once

template<typename K, typename V, size_t C>
struct SLinearMap
{
	struct SKv
	{
		K m_key;
		V m_value;
	};

	bool FTryGetValueFromKey(bool (*FCompare) (const K & key1, const K & key2), const K & key, V * pValue)
	{
		for (int i = 0; i < C; i++)
		{
			if (FCompare(m_aKv[i].m_key, key))
			{
				*pValue = m_aKv[i].m_value;
				return true;
			}
		}

		return false;
	}

	SKv m_aKv[C] = {};
};
