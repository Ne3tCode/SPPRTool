#ifndef UTLBUFFER_MINI_H
#define UTLBUFFER_MINI_H

#ifdef _WIN32
#pragma once
#endif

class CUtlBuffer
{
public:
	typedef bool (CUtlBuffer::*OverflowFunc_t)(int nSize);

	CUtlBuffer(int growSize = 0, int initSize = 0)
		: m_pMemory(nullptr), m_nAllocationCount(0), m_nGrowSize(growSize)
	{
		EnsureCapacity(initSize);
		m_GetOverflowFunc = &EnsureCapacity;
		m_PutOverflowFunc = &EnsureCapacity;
		m_dummy1 = m_dummy2 = m_dummy3 = m_dummy4 = m_dummy5 = m_dummy6 = m_Byteswap = 0;
	}

	~CUtlBuffer()
	{
		Purge();
	}

	unsigned char *Base()
	{
		return m_pMemory;
	}

	int Size()
	{
		return m_nAllocationCount;
	}

	bool EnsureCapacity(int num)
	{
		if (m_nAllocationCount >= num)
			return true;

		if (m_nGrowSize < 0)
			return false;

		m_nAllocationCount = num;

		if (m_pMemory)
		{
			m_pMemory = (unsigned char *)realloc(m_pMemory, m_nAllocationCount * sizeof(unsigned char));
		}
		else
		{
			m_pMemory = (unsigned char *)malloc(m_nAllocationCount * sizeof(unsigned char));
		}
		return true;
	}

	void Purge()
	{
		if (m_nGrowSize >= 0)
		{
			if (m_pMemory)
			{
				free(m_pMemory);
				m_pMemory = nullptr;
			}
			m_nAllocationCount = 0;
		}
		m_dummy1 = m_dummy2 = m_dummy3 = m_dummy4 = m_dummy5 = m_dummy6 = m_Byteswap = 0;
	}

private:
	int m_dummy1;
	unsigned char *m_pMemory;
	int m_nAllocationCount;
	int m_nGrowSize;

	int m_dummy2; // 2
	int m_dummy3; // dummy
	int m_dummy4; // 4
	int m_dummy5; // me
	int m_dummy6; // ¯\_(ツ)_/¯

	OverflowFunc_t m_GetOverflowFunc;
	OverflowFunc_t m_PutOverflowFunc;

	int m_Byteswap;
};

#endif // UTLBUFFER_MINI_H
