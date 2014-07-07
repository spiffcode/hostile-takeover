#include "ht.h"

HashTableCode ghstCode;

// CacheEntry for TBitmapCode code that also understands how to be part of a hash table

void HashEntryCode::Discard()
{
	CacheEntry::Discard();
	ghstCode.FreeEntry(this);
}

#define kcHashEntries 1024 // must be power of 2
#define kwIndexMask (kcHashEntries - 1)

HashTableCode::HashTableCode()
{
	m_aphec = NULL;
	m_phecFree = NULL;
}

HashTableCode::~HashTableCode()
{
	Assert(m_aphec == NULL);
	Assert(m_phecFree == NULL);
}

bool HashTableCode::Init()
{
	m_aphec = new HashEntryCode *[kcHashEntries];
	Assert(m_aphec != NULL);
	if (m_aphec == NULL)
		return false;
	memset(m_aphec, 0, sizeof(HashEntryCode *) * kcHashEntries);
	return true;
}

void HashTableCode::Exit()
{
	DiscardHashEntries();
	delete m_aphec;
	m_aphec = NULL;

	// Free free list entries

	HashEntryCode *phec = m_phecFree;
	while (phec != NULL) {
		HashEntryCode *phecNext = phec->m_phecNext;
		delete phec;
		phec = phecNext;
	}
	m_phecFree = NULL;
}

void HashTableCode::DiscardHashEntries()
{
	// Free hash table entries

	for (int i = 0; i < kcHashEntries; i++) {
		while (m_aphec[i] != NULL)
			m_aphec[i]->Discard();
	}
}

void HashTableCode::FreeEntry(HashEntryCode *phec)
{
	HashEntryCode **pphec = &m_aphec[(int)(phec->m_hk & kwIndexMask)];
	for (; (*pphec) != NULL; pphec = &(*pphec)->m_phecNext) {
		if (*pphec == phec) {
			*pphec = phec->m_phecNext;
			phec->m_phecNext = m_phecFree;
			m_phecFree = phec;
			return;
		}
	}
	Assert(false);
}

HashEntryCode *HashTableCode::FindEntry(dword hkName, int x, int y, int cx, int cy, bool fOdd)
{
	// Try to find it

	//dword hk = hkName ^ (cy << (y & 7)) + (y << (cy & 7)) + (cx << (x & 7)) + (x << (cx & 7)) + (fEven ? 0x5050 : 0);
	dword hk = hkName ^ ((y << 8) + cy) + ((x << 8) + cx) + (fOdd ? 0x5050 : 0);
	HashEntryCode *phec = m_aphec[(int)(hk & kwIndexMask)];
	for (; phec != NULL; phec = phec->m_phecNext) {
		if (phec->m_hk != hk)
			continue;
		if (fOdd == phec->m_fOdd && x == phec->m_x && y == phec->m_y && cx == phec->m_cx && cy == phec->m_cy)
			return phec;
		// This is a legal condition, but the assert is to notify me of collision.
		//Assert(false);
	}

	// Can't find it, so create it
	// Alloc entry

	if (m_phecFree != NULL) {
		phec = m_phecFree;
		m_phecFree = phec->m_phecNext;
	} else {
		phec = new HashEntryCode;
		if (phec == NULL)
			return NULL;
	}

	// Init

	phec->m_x = x;
	phec->m_y = y;
	phec->m_cx = cx;
	phec->m_cy = cy;
	phec->m_fOdd = fOdd;
	phec->m_hk = hk;

	// Add to table

	HashEntryCode **pphec = &m_aphec[(int)(phec->m_hk & kwIndexMask)];
	if (*pphec != NULL) {
		phec->m_phecNext = *pphec;
	} else {
		phec->m_phecNext = NULL;
	}
	*pphec = phec;

	// Add to cache

	gcam.Add((CacheEntry *)phec);
	return phec;
}