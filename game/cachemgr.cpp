#include "ht.h"

namespace wi {

CacheMgr gcam;

CacheMgr::CacheMgr()
{
	m_pceList = NULL;
	m_pceFirst = NULL;
	m_pceFree = NULL;
	m_cbTotalSize = 0;
	m_cbLimit = 0;
}

CacheMgr::~CacheMgr()
{
	Assert(m_pceList == NULL);
}

void CacheMgr::Exit()
{
	// Free cache entries

	while (m_pceFirst != NULL)
		Discard(m_pceFirst);
	m_pceFirst = NULL;

	// Free CacheEntry list

	delete m_pceList;
	m_pceList = NULL;
	m_pceFree = NULL;
}

bool CacheMgr::Init()
{
	// Alloc cache entries

	m_pceList = new CacheEntry[kcCacheEntries];
	Assert(m_pceList != NULL, "out of memory!");
	if (m_pceList == NULL)
		return false;
	memset(m_pceList, 0, sizeof(CacheEntry) * kcCacheEntries);

	// Add all entries to free list

	for (CacheEntry *pce = m_pceList; pce < &m_pceList[kcCacheEntries]; pce++) {
		pce->wUniqueLock = kwIncUnique;
		AddToFreeList(pce);
	}

	return true;
}

dword CacheMgr::GetLimit()
{
	return m_cbLimit;
}

void CacheMgr::SetLimit(dword cbLimit)
{
	if (cbLimit == 0) {
		m_cbLimit = 0;
		return;
	}
	gcam.MakeSpace((dword)-1);
	dword cbMin = m_cbTotalSize + ((dword)32 * 1024);
	if (cbLimit < cbMin)
		cbLimit = cbMin;
	m_cbLimit = cbLimit;
}

dword CacheMgr::GetTotalSize()
{
	return m_cbTotalSize;
}

void *CacheMgr::GetPtr(CacheHandle hc)
{
	CacheEntry *pce = ValidateHandle(hc);
	if (pce == NULL)
		return NULL;
	Assert(pce->hmem != NULL);
	Remove(pce);
	Add(pce);
	return gmmgr.GetPtr(pce->hmem);
}

void *CacheMgr::Lock(CacheHandle hc)
{
	CacheEntry *pce = ValidateHandle(hc);
	if (pce == NULL)
		return NULL;
	Assert(pce->hmem != NULL);
	Remove(pce);
	Add(pce);
	Assert(((pce->wUniqueLock + kwIncLock) & kwLockMask) != 0);
	pce->wUniqueLock += kwIncLock;
	if ((pce->wUniqueLock & kwLockMask) == kwIncLock)
		gmmgr.SetLocked(pce->hmem);
	return gmmgr.GetPtr(pce->hmem);
}

void CacheMgr::Unlock(CacheHandle hc)
{
	CacheEntry *pce = ValidateHandle(hc);
	if (pce == NULL)
		return;
	Assert(pce->hmem != NULL);
	Assert((pce->wUniqueLock & kwLockMask) != 0);
	pce->wUniqueLock -= kwIncLock;
	if ((pce->wUniqueLock & kwLockMask) == 0)
		gmmgr.ClearLocked(pce->hmem);
}

bool CacheMgr::IsValid(CacheHandle hc)
{
	CacheEntry *pce = ValidateHandle(hc);
	return pce != NULL;
}

CacheHandle CacheMgr::NewObject(void *pv, word cb, word wfHints)
{
	// Apply limits if asked

	if (m_cbLimit != 0) {
		while (m_cbTotalSize + cb > m_cbLimit) {
			if (!MakeSpace(m_cbTotalSize + cb - m_cbLimit))
				return NULL;
		}
	}

	// Free up an entry if we need to

	if (m_pceFree == NULL) {
		// No free slots available. Discard the oldest entry for reuse.

		for (CacheEntry *pceT = m_pceFirst->pcePrev; pceT != NULL; pceT = pceT->pcePrev) {
			// If we loop back to m_pceFirst, then all CacheEntries are locked. No way!

			Assert(pceT != m_pceFirst);
			if ((pceT->wUniqueLock & kwLockMask) == 0) {
				Discard(pceT);
				break;
			}
		}
	}
	CacheEntry *pce = m_pceFree->pcePrev;
	Assert(pce != NULL);
	if (pce == NULL)
		return NULL;

	// Alloc the object

	pce->hmem = gmmgr.AllocHandle(cb, wfHints);
	Assert(pce->hmem != NULL);
	if (pce->hmem == NULL)
		return NULL;
	pce->cbSize = cb;

	// Write in data

	if (pv != NULL)
		gmmgr.WriteHandle(pce->hmem, 0, pv, cb);

	// Take off free list, put at start of the alloced list

	RemoveFromFreeList(pce);
	Add(pce);
	m_cbTotalSize += cb;
	return MakeHandle(pce);
}

void CacheMgr::Write(CacheHandle hc, word ib, void *pvSrc, word cb)
{
	CacheEntry *pce = ValidateHandle(hc);
	Assert(pce != NULL);
	if (pce == NULL)
		return;
	gmmgr.WriteHandle(pce->hmem, ib, pvSrc, cb);
}

word CacheMgr::GetSize(CacheHandle hc)
{
	CacheEntry *pce = ValidateHandle(hc);
	Assert(pce != NULL);
	if (pce == NULL)
		return 0;
	return pce->cbSize;
}

void CacheMgr::Discard(CacheEntry *pce)
{
	Assert((pce->wUniqueLock & kwLockMask) == 0);
	Assert(pce->hmem != NULL);
	Remove(pce);
	AddToFreeList(pce);
	gmmgr.FreeHandle(pce->hmem);
	pce->hmem = NULL;
	pce->wUniqueLock += kwIncUnique;
	if ((pce->wUniqueLock & kwUniqueMask) == 0)
		pce->wUniqueLock += kwIncUnique;
	m_cbTotalSize -= pce->cbSize;
}

bool CacheMgr::MakeSpace(dword cb)
{
	if (m_pceFirst == NULL)
		return false;
	CacheEntry *pceT = m_pceFirst->pcePrev;
	while (pceT != m_pceFirst) {
		if ((pceT->wUniqueLock & kwLockMask) != 0) {
			pceT = pceT->pcePrev;
			continue;
		}
		CacheEntry *pcePrev = pceT->pcePrev;
		Discard(pceT);
		word cbLargestFree;
		gmmgr.GetFreeSize(&cbLargestFree);
		if (cbLargestFree >= cb)
			return true;
		pceT = pcePrev;
	}
	return false;
}

} // namespace wi