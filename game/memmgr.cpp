// Heap support. Chunks are allocated by MemMgr, heaps are initialized into these chunks (can be in
// dyn ram or db ram). The MemMgr delegates requests to the appropriate heap. Allocs out of dyn ram first.

// "chunk" are the chunks of memory allocated from the host os in which we create heaps
// "block" are the blocks of memory we reserve in our heaps for allocs
// "alloc" are the allocs requested. cbBlock == ((cbAlloc + 1) & ~1) + sizeof(MemHeader)

#if defined(DEBUG) && !defined(PIL)
#define INCL_FORCECOMPACTION
#endif

#include "ht.h"

namespace wi {

MemMgr gmmgr;
dword gcbDynMemAtStart;
dword gcbDbMemAtStart;

// MemHeader w field
// bit 0: busy flag. 1 if busy, 0 if free. If free, bits 1-15 are the offset to the next free.
// bit 1: locked flag. 1 if locked, 0 if unlocked. Valid only if busy is 1
// bits 2-7: handle table entry index
// bits 8-15: handle table index

#define kfMhdrBusy 0x01
#define kfMhdrLocked 0x02

#define IsFree(pmhdr) ((pmhdr->w & kfMhdrBusy) == 0)
#define IsLocked(pmhdr) ((pmhdr->w & kfMhdrLocked) != 0)
#define GetNextFree(pmhdr) (pmhdr->w == 0 ? NULL : (MemHeader *)(pheap->pbHeap + pmhdr->w))
#define GetNext(pmhdr) ((MemHeader *)((byte *)pmhdr + pmhdr->cbBlock))
#define GetHandleTableIndex(her) (her >> 8)
#define GetHandleEntryIndex(her) ((her & 0xfc) >> 2)
#define GetHandleEntryPtr(her) (&m_aphtab[GetHandleTableIndex(her)]->ahe[GetHandleEntryIndex(her)])
#define GetHandleEntryRef(ihtab, ihe) ((ihtab << 8) | (ihe << 2))
#define UpdateHandleEntry(pheap, her, pmhdr) *GetHandleEntryPtr(her) = (word)((byte *)pmhdr - pheap->pbHeap)
#define GetHeap(hmem) m_apheap[((HMemStruct *)&hmem)->iheap]
#define GetMemHeader(hmem) (MemHeader *)(*GetHandleEntryPtr(((HMemStruct *)&hmem)->her) + GetHeap(hmem)->pbHeap)

// Heap flags

#define kfHeapDbRam 1
#define kfHeapLargestFreeBlockSizeValid 2
#define kfHeapFullyCompacted 4

// MemMgr

#define kcbChunkAllocBig ((word)63 * 1024)
#define kcbChunkAllocSmall ((word)32 * 1024)
#define kcbChunkTooSmall ((word)4 * 1024)

#define kszDbMemMgr "HtDbMemMgr"
#define kdwType 'temp'

MemMgr::MemMgr()
{
	m_cheap = 0;
	m_chtab = 0;
	m_chtabAlloced = 0;
	m_aphtab = NULL;
	m_cbTotalFreeBlocks = 0;
#ifdef PIL
	m_lid = 0;
	m_pdb = NULL;
#endif
}

MemMgr::~MemMgr()
{
	Assert(m_cheap == 0);
	Assert(m_chtab == 0);
#ifdef PIL
	Assert(m_pdb == NULL);
#endif
}

void MemMgr::OnRebootFreeStorageDatabase()
{
#ifdef PIL
	LocalID lid = DmFindDatabase(0, kszDbMemMgr);
	if (lid != 0)
		DmDeleteDatabase(0, lid);
#endif
}

#if defined(WIN) || defined(IPHONE) || defined(SDL)
#define kcbDynMemWindows (3 * 1024 * 1024)
#endif

void MemMgr::GetInitSize(dword cbDynReserve, dword *pcbDyn, dword *pcbStorage)
{
#ifdef PIL
	UInt32 cbDyn, cbStorage;
	UInt32 cbMaxT;
	MemHeapFreeBytes(0, (UInt32 *)&cbDyn, &cbMaxT);
	MemHeapFreeBytes(1, (UInt32 *)&cbStorage, &cbMaxT);

	dword cbDynRemaining = 0;
	if (cbDynReserve < cbDyn)
		cbDynRemaining = cbDyn - cbDynReserve;

	*pcbDyn = cbDynRemaining;
	*pcbStorage = cbStorage;
#else
	*pcbDyn = kcbDynMemWindows;
	*pcbStorage = 0;
#endif
}

void MemMgr::Init(dword cbDynReserve, dword cbMaxNeeded, dword *pcbTotal)
{
	// Remember how much memory is available in dyn, db heaps before doing anything.

#ifdef PIL
	UInt32 cbMaxT;
	MemHeapFreeBytes(0, (UInt32 *)&gcbDynMemAtStart, &cbMaxT);
	MemHeapFreeBytes(1, (UInt32 *)&gcbDbMemAtStart, &cbMaxT);
#else
	gcbDynMemAtStart = kcbDynMemWindows;
	gcbDbMemAtStart = 0;
#endif

	// First, dyn allocs. Grab everything we can, in the largest chunks we can.

	m_cheap = 0;
	*pcbTotal = 0;
	while (*pcbTotal < cbMaxNeeded) {
		// Bounds check

		if (m_cheap == ARRAYSIZE(m_apheap))
			break;

		// Find out how much dyn ram we have to alloc. If we're at our
		// reserve limit, break out.

#ifdef PIL
		// Call the mem api for Palm

		UInt32 cbFree;
		MemHeapFreeBytes(0, &cbFree, &cbMaxT);
#else
		// For non-Palm, just assume a big size

		dword cbFree = gcbDynMemAtStart - *pcbTotal;
#endif
		if (cbFree <= cbDynReserve)
			break;

		// If the chunk is too small to bother, break out

		dword cbAlloc = cbFree - cbDynReserve;
		if (cbAlloc < kcbChunkTooSmall)
			break;

		// Hack: On simulator, MemHeapFreeBytes() lies and doesn't change across
		// successive allocs. If we can't alloc the size we want, skip the dyn heap
		// and go for the storage heap

		if (m_cheap == 0) {
			// Don't waste a heap if marginal space is available

			if (cbAlloc < kcbChunkAllocBig)
				break;
		} else {
			// If the last heap was fractional, skip to storage mem

			if (m_apheap[m_cheap - 1]->cbHeap < kcbChunkAllocBig)
				break;
		}

		// Palm MemMgr is funky. Try big chunk size first (bigger the better). If it fails,
		// try the small size. If that fails, break out.

		if (cbAlloc > kcbChunkAllocBig)
			cbAlloc = kcbChunkAllocBig;
		void *pv = (void *)new byte[cbAlloc];
		if (pv == NULL && cbAlloc > kcbChunkAllocSmall) {
			cbAlloc = kcbChunkAllocSmall;
			pv = (void *)new byte[cbAlloc];
		}
		if (pv == NULL)
			break;

		// Create a heap in this chunk and continue

		Heap *pheap = NewHeap(pv, (word)cbAlloc, false, 0, NULL);
		if (pheap == NULL) {
			delete[] (byte *)pv;
			break;
		}
		m_apheap[m_cheap++] = pheap;
		*pcbTotal += cbAlloc;
	}
	
	// If our heap space is big enough, we're done

	if (*pcbTotal >= cbMaxNeeded)
		return;

#ifdef PIL
	// Start allocing out of db ram (Palm only)

	LocalID lid = DmFindDatabase(0, kszDbMemMgr);
	if (lid != 0)
		DmDeleteDatabase(0, lid);
	if (DmCreateDatabase(0, kszDbMemMgr, kdwCreator, kdwType, false) != 0)
		return;
	m_lid = DmFindDatabase(0, kszDbMemMgr);
	if (m_lid == 0)
		return;
	m_pdb = DmOpenDatabase(0, m_lid, dmModeReadWrite);
	if (m_pdb == NULL) {
		DmDeleteDatabase(0, m_lid);
		return;
	}

	// Start creating heaps

	while (*pcbTotal < cbMaxNeeded) {
		// Bounds check

		if (m_cheap == ARRAYSIZE(m_apheap))
			break;

		// Try to allocate a chunk

		UInt32 cbAlloc = kcbChunkAllocBig;
		UInt16 irec = dmMaxRecordIndex;
		MemHandle hNew = DmNewRecord(m_pdb, &irec, cbAlloc);
		if (hNew == NULL) {
			cbAlloc = kcbChunkAllocSmall;
			hNew = DmNewRecord(m_pdb, &irec, cbAlloc);
		}
		if (hNew == NULL)
			break;
		void *pv = MemHandleLock(hNew);

		// Make a heap in this chunk. If this fails, break out. The db records
		// will get removed in later cleanup

		Heap *pheap = NewHeap(pv, (word)cbAlloc, true, irec, hNew);
		if (pheap == NULL)
			break;
		m_apheap[m_cheap++] = pheap;
		*pcbTotal += cbAlloc;
	}
#endif
}

void MemMgr::Exit()
{
	// Free heaps first

	for (int iheap = 0; iheap < m_cheap; iheap++) {
		Heap *pheap = m_apheap[iheap];

		// Check for one free block the size of the heap. If anything less, something hasn't been freed

		Assert(((MemHeader *)pheap->pbHeap)->cbBlock == pheap->cbHeap && IsFree(((MemHeader *)pheap->pbHeap)));

		// dyn heap or db heap?

		if (pheap->wf & kfHeapDbRam) {
#ifdef PIL
			// Just unlock the record; it'll get freed below all at once

			MemHandleUnlock((MemHandle)pheap->hMem);
			DmReleaseRecord(m_pdb, pheap->nRec, false);
#else
			Assert(false);
#endif
		} else {
			// Free the dyn chunk

			delete[] pheap->pbHeap;
		}
		delete pheap;
	}
	m_cheap = 0;

#ifdef PIL
	// Now free database which'll free all db chunks at once.

	if (m_pdb != NULL) {
		Err err = DmCloseDatabase(m_pdb);
		Assert(err == 0);
		DmDeleteDatabase(0, m_lid);
		m_pdb = NULL;
		m_lid = 0;
	}
#endif

	// Free handle tables

	for (int ihtab = 0; ihtab < m_chtab; ihtab++)
		delete m_aphtab[ihtab];
	delete[] m_aphtab;
	m_chtab = 0;
	m_chtabAlloced = 0;
	m_aphtab = NULL;
	m_cbTotalFreeBlocks = 0;
}

Heap *MemMgr::NewHeap(void *pvHeap, word cbHeap, bool fDbRam, word nRec, void *hMem)
{
	Heap *pheap = new Heap;
	if (pheap == NULL)
		return NULL;

	pheap->pbHeap = (byte *)pvHeap;
	pheap->cbHeap = cbHeap;
	pheap->wf = fDbRam ? kfHeapDbRam : 0;
	pheap->nRec = nRec;
	pheap->hMem = hMem;

	MemHeader mhdr;
	mhdr.w = 0;
	mhdr.cbBlock = cbHeap;
	WriteHeader(pheap, (MemHeader *)pvHeap, &mhdr);
	pheap->pmhdrFreeFirst = (MemHeader *)pvHeap;

	m_cbTotalFreeBlocks += cbHeap;

	return pheap;
}

#define kchtabGrow 32

HandleTable *MemMgr::NewHandleTable()
{
	// Make space in the list

	if (m_chtab == m_chtabAlloced) {
		HandleTable **aphtabT = new HandleTable *[m_chtabAlloced + kchtabGrow];
		if (aphtabT == NULL)
			return NULL;
		if (m_aphtab != NULL) {
			memcpy(aphtabT, m_aphtab, sizeof(HandleTable *) * m_chtabAlloced);
			delete[] m_aphtab;
		}
		m_aphtab = aphtabT;
		m_chtabAlloced += kchtabGrow;
	}

	// Alloc new

	HandleTable *phtab = new HandleTable;
	if (phtab == NULL)
		return NULL;
	memset(phtab, 0, sizeof(HandleTable));

	// Link into free list

	for (int ihe = 0; ihe < kcHandlesPerTable; ihe++) {
		if (ihe < kcHandlesPerTable - 1) {
			phtab->ahe[ihe] = (HandleEntry)(ihe + 1);
		} else {
			phtab->ahe[ihe]= (HandleEntry)-1;
		}
	}

	// Done

	m_aphtab[m_chtab++] = phtab;
	return phtab;
}

bool MemMgr::AllocHandleEntry(word *pher)
{
#ifdef INCL_VALIDATEHEAP
	ValidateHandleTableFreeEntries();
#endif

	// First find a handle table with a free entry

	int ihtab;
	for (ihtab = 0; ihtab < m_chtab; ihtab++) {
		if (m_aphtab[ihtab]->iheFree != -1)
			break;
	}

	// If we couldn't find a suitable handle table, make a new one

	if (ihtab >= m_chtab) {
		if (NewHandleTable() == NULL)
			return false;
		ihtab = m_chtab - 1;
	}

	// Grab the next free handle entry

	HandleTable *phtab = m_aphtab[ihtab];
	Assert(phtab->iheFree >= 0 && phtab->iheFree < ARRAYSIZE(phtab->ahe));
	int ihe = phtab->iheFree;
	int iheFree = (int)(short)phtab->ahe[ihe];
	Assert(iheFree == -1 || iheFree < ARRAYSIZE(phtab->ahe));
	Assert(phtab->ahe[iheFree] == (word)-1 || phtab->ahe[iheFree] < ARRAYSIZE(phtab->ahe));
	phtab->iheFree = iheFree;

	// Return a cookie reference

	*pher = GetHandleEntryRef(ihtab, ihe);

#ifdef INCL_VALIDATEHEAP
	phtab->ahe[ihe] = 0;
	ValidateHandleTableFreeEntries();
#endif

	return true;
}

void MemMgr::FreeHandleEntry(word her)
{
#ifdef INCL_VALIDATEHEAP
	ValidateHandleTableFreeEntries();
#endif

	int ihtab = GetHandleTableIndex(her);
	int ihe = GetHandleEntryIndex(her);
	Assert(ihtab < m_chtab);
	HandleTable *phtab = m_aphtab[ihtab];
	Assert(ihe < ARRAYSIZE(phtab->ahe)); // should be in table
	Assert((phtab->ahe[ihe] & 1) == 0); // hack to check it's not allocated
	Assert(phtab->iheFree == -1 || phtab->iheFree < ARRAYSIZE(phtab->ahe)); // validate free
	Assert(phtab->ahe[phtab->iheFree] == (word)-1 || phtab->ahe[phtab->iheFree] < ARRAYSIZE(phtab->ahe)); // validate free points to free
	phtab->ahe[ihe] = (HandleEntry)phtab->iheFree;
	phtab->iheFree = ihe;

#ifdef INCL_VALIDATEHEAP
	ValidateHandleTableFreeEntries();
#endif
}

dword MemMgr::GetFreeSize(word *pcbLargestFree)
{
	if (pcbLargestFree != NULL) {
		word cbLargestFreeBlock = 0;
		for (int iheap = 0; iheap < m_cheap; iheap++) {
			Heap *pheap = m_apheap[iheap];
			word cbT = GetLargestFreeBlockSize(pheap);
			if (cbT > cbLargestFreeBlock)
				cbLargestFreeBlock = cbT;
		}
		if (cbLargestFreeBlock > sizeof(MemHeader)) {
			*pcbLargestFree = cbLargestFreeBlock - sizeof(MemHeader);
		} else {
			*pcbLargestFree = 0;
		}
	}

	return m_cbTotalFreeBlocks - sizeof(MemHeader);
}

word MemMgr::GetLargestFreeBlockSize(Heap *pheap)
{
	// Compact to determine the largest free block

	if ((pheap->wf & (kfHeapFullyCompacted | kfHeapLargestFreeBlockSizeValid)) != (kfHeapFullyCompacted | kfHeapLargestFreeBlockSizeValid)) {
		// We want to get the large free block size which we'll do by
		// compaction even if the heap is fully compacted already (rare).

		pheap->wf &= ~kfHeapFullyCompacted;
		Compact(pheap, 0xffff, NULL, 0);
	}

	// Now these bits should be valid

	Assert((pheap->wf & (kfHeapFullyCompacted | kfHeapLargestFreeBlockSizeValid)) == (kfHeapFullyCompacted | kfHeapLargestFreeBlockSizeValid));
	return pheap->cbLargestFreeBlock;
}

dword MemMgr::GetTotalSize(dword *pcbDyn, dword *pcbDb)
{
	*pcbDyn = 0;
	*pcbDb = 0;
	for (int iheap = 0; iheap < m_cheap; iheap++) {
		Heap *pheap = m_apheap[iheap];
		if (pheap->wf & kfHeapDbRam) {
			*pcbDb += pheap->cbHeap;
		} else {
			*pcbDyn += pheap->cbHeap;
		}
	}
	return *pcbDb + *pcbDyn;
}

HMem MemMgr::AllocHandle(word cbAlloc, word wfHints)
{
	// CE is ARM which wants dword aligned allocs because of alignment issues

#ifdef __CPU_68K
	word cbBlock = ((cbAlloc + 1) & ~1) + sizeof(MemHeader);
#else
	word cbBlock = ((cbAlloc + 3) & ~3) + sizeof(MemHeader);
#endif
	bool fLastCacheDiscardFailed = false;

	// Try to alloc from one of our heaps. If this fails, free space from the cache and try again

	while (true) {
		// Try each heap

		for (int iheap = 0; iheap < m_cheap; iheap++) {
			Heap *pheap = m_apheap[iheap];

			// Do we have a block large enough to contain this alloc? We may be able to know
			// that up front and save some time.

			if ((pheap->wf & (kfHeapFullyCompacted | kfHeapLargestFreeBlockSizeValid)) == (kfHeapFullyCompacted | kfHeapLargestFreeBlockSizeValid)) {
				if (cbBlock > pheap->cbLargestFreeBlock)
					continue;
			}
			
			// Find any that fits.

			word cbLargestFreeBlock = 0;
			MemHeader *pmhdrFreeLast = NULL;
			MemHeader *pmhdrFree = pheap->pmhdrFreeFirst;
			MemHeader *pmhdrUse = NULL;
			MemHeader *pmhdrUseFreeLast = NULL;
			while (pmhdrFree != NULL) {
				// This block large enough?

				Assert(IsFree(pmhdrFree));
				if (pmhdrFree->cbBlock >= cbBlock) {
					// If this block will get locked, alloc it at the end of the heap to help
					// fragmentation

					pmhdrUse = pmhdrFree;
					pmhdrUseFreeLast = pmhdrFreeLast;
					if (!(wfHints & kfHintWillLock))
						break;
				}

				// Track largest free

				if (pmhdrFree->cbBlock > cbLargestFreeBlock)
					cbLargestFreeBlock = pmhdrFree->cbBlock;

				// Next free

				pmhdrFreeLast = pmhdrFree;
				pmhdrFree = GetNextFree(pmhdrFree);
			}

			// If we scanned the whole list, update largest free

			if (pmhdrFree == NULL) {
				pheap->cbLargestFreeBlock = cbLargestFreeBlock;
				pheap->wf |= kfHeapLargestFreeBlockSizeValid;
			}
			
			// If we found something, use it

			if (pmhdrUse != NULL)
				return AllocBlock(iheap, pmhdrUse, pmhdrUseFreeLast, cbBlock, wfHints);

			// If already compacted, nothing more to do for this heap.

			if (pheap->wf & kfHeapFullyCompacted)
				continue;

			// Compact to try to create the space

			pmhdrFree = Compact(pheap, cbBlock, &pmhdrFreeLast, wfHints);
			if (pmhdrFree != NULL)
				return AllocBlock(iheap, pmhdrFree, pmhdrFreeLast, cbBlock, wfHints);
		}

		// No space in any of the heaps. Free space from the cache.
		// If the last time we tried to make space it failed, there is nothing left
		// to discard, so we have a total failure.

		if (fLastCacheDiscardFailed)
			return NULL;
		fLastCacheDiscardFailed = !gcam.MakeSpace(cbAlloc);
	}
}

HMem MemMgr::AllocBlock(int iheap, MemHeader *pmhdr, MemHeader *pmhdrFreeLast, word cbBlock, word wfHints)
{
#ifdef INCL_VALIDATEHEAP
	Validate();
#endif

	// First need to alloc a new handle. This can fail.

	word her;
	if (!AllocHandleEntry(&her))
		return NULL;

	MemHeader mhdr;
	mhdr.w = her | kfMhdrBusy;
	mhdr.cbBlock = cbBlock;

	// Have a block to use, now officially alloc it for use.
	// Should we split this block or use it as is?

	Heap *pheap = m_apheap[iheap];
	if (pmhdr->cbBlock - cbBlock > sizeof(MemHeader)) {
		// Enough room to split the existing block

		pmhdr = SplitFreeBlock(pheap, pmhdr, pmhdrFreeLast, &mhdr, wfHints);
	} else {
		// Not enough room to split so use this block as is. May have up to
		// MemHeader+1 extra bytes. Remove this block from the free list.

		if (pmhdrFreeLast != NULL) {
			MemHeader mhdrFreeLast = *pmhdrFreeLast;
			mhdrFreeLast.w = pmhdr->w;
			WriteHeader(pheap, pmhdrFreeLast, &mhdrFreeLast);
		} else {
			pheap->pmhdrFreeFirst = GetNextFree(pmhdr);
		}

		// Init the new block header

		mhdr.cbBlock = pmhdr->cbBlock;
		WriteHeader(m_apheap[iheap], pmhdr, &mhdr);

		// If we're using the largest block, we don't know what the next largest block is

		if (mhdr.cbBlock == pheap->cbLargestFreeBlock)
			pheap->wf &= ~kfHeapLargestFreeBlockSizeValid;
	}

	// Update the handle to this object

	UpdateHandleEntry(pheap, pmhdr->w, pmhdr);

	// We just used this much space

	m_cbTotalFreeBlocks -= pmhdr->cbBlock;

#ifdef INCL_VALIDATEHEAP
	Validate();
#endif

	// We're done.

	HMemStruct hms;
	hms.her = mhdr.w;
	hms.iheap = iheap;

	// Easiest way to handle alignment

	HMem hMem;
	memcpy(&hMem, &hms, _min(sizeof(hMem), sizeof(hms)));
	return hMem;
}

MemHeader *MemMgr::SplitFreeBlock(Heap *pheap, MemHeader *pmhdr, MemHeader *pmhdrFreeLast, MemHeader *pmhdrNew, word wfHints)
{
	Assert(pmhdr->cbBlock - pmhdrNew->cbBlock > sizeof(MemHeader));
	Assert(IsFree(pmhdr));

	// If we're splitting the largest block, we don't know what the next
	// biggest block is

	if (pmhdr->cbBlock == pheap->cbLargestFreeBlock)
		pheap->wf &= ~kfHeapLargestFreeBlockSizeValid;

	if (wfHints & kfHintWillLock) {
		// Locked objects go at the end of the heap. So when splitting a free block, place the new object
		// at the end of the block, new free block at the start. This method doesn't require adjusting
		// the free list since the free block doesn't move.

		MemHeader mhdrFreeNew = *pmhdr;
		mhdrFreeNew.cbBlock -= pmhdrNew->cbBlock;
		WriteHeader(pheap, pmhdr, &mhdrFreeNew);
		pmhdr = GetNext(pmhdr);
		WriteHeader(pheap, pmhdr, pmhdrNew);
	} else {
		// Split the block so the new free chunk goes at the end. This tends to alloc blocks
		// towards the front of the heap (at the penalty of having to update the free list)

		MemHeader mhdrFreeNew = *pmhdr;
		WriteHeader(pheap, pmhdr, pmhdrNew);
		mhdrFreeNew.cbBlock -= pmhdrNew->cbBlock;
		MemHeader *pmhdrFreeNew = GetNext(pmhdr);
		WriteHeader(pheap, pmhdrFreeNew, &mhdrFreeNew);

		// Update the free list

		if (pmhdrFreeLast != NULL) {
			MemHeader mhdrFreeLast = *pmhdrFreeLast;
			mhdrFreeLast.w = (word)((byte *)pmhdrFreeNew - pheap->pbHeap);
			WriteHeader(pheap, pmhdrFreeLast, &mhdrFreeLast);
		} else {
			pheap->pmhdrFreeFirst = pmhdrFreeNew;
		}
		Assert(IsFree(pmhdrFreeNew));
	}

	// Return the new block

	return pmhdr;
}

MemHeader *MemMgr::Compact(Heap *pheap, word cbBlock, MemHeader **ppmhdrFreeLast, word wfHints)
{
#ifdef INCL_VALIDATEHEAP
	Validate();
#endif

	// We're here to see if a block at least as big as cbBlock can be created by heap compaction.

	Assert(!(pheap->wf & kfHeapFullyCompacted));

	// Use a bubble up approach. Try to move free space upwards to create larger free chunks towards the middle
	// of the heap. Since everything is handle based, works great on everything except locked objects.

	word cbLargestFreeBlock = 0;
	MemHeader *pmhdrUse = NULL;
	MemHeader *pmhdr = (MemHeader *)pheap->pbHeap;
	MemHeader *pmhdrHeapEnd = (MemHeader *)&pheap->pbHeap[pheap->cbHeap];
	MemHeader *pmhdrLast = NULL;
	MemHeader *pmhdrFreeLast = NULL;
	MemHeader *pmhdrFreeLastNext = NULL;
	while (pmhdr != pmhdrHeapEnd) {
		// We do compacting by comparing the current and last blocks

		if (pmhdrLast != NULL) {
			// Remember the last free block so the free list can be maintained

			if (IsFree(pmhdrLast))
				pmhdrFreeLastNext = pmhdrLast;

			// If this block is free, we can merge two free blocks, or if not free, bubble up a
			// free block

			if (IsFree(pmhdr)) {
				// This block is free.

				if (IsFree(pmhdrLast)) {
					// Last block is free too; coalesce these two

					MemHeader mhdr = *pmhdr;
					mhdr.cbBlock += pmhdrLast->cbBlock;
					WriteHeader(pheap, pmhdrLast, &mhdr);

					// pmhdr has disappeared. It's now the previous block.

					pmhdr = pmhdrLast;

					// Update the largest free if necessary

					if (pmhdr->cbBlock > pheap->cbLargestFreeBlock)
						pheap->cbLargestFreeBlock = pmhdr->cbBlock;

					// Keep the last "free last" since we just merged pmhdrLast

					pmhdrFreeLastNext = pmhdrFreeLast;
				}
			} else {
				// This block is alloced. If it is locked, we really can't do anything
				// with this block and the last block.

				if (!IsLocked(pmhdr)) {
					// This block is unlocked.

					if (IsFree(pmhdrLast)) {
						// Previous block is free. Move the current block into that free block,
						// thereby 'bubbling up' the free block.
						// Save the free header

						MemHeader mhdrFree = *pmhdrLast;

						// First update handle table for this block about to move

						UpdateHandleEntry(pheap, pmhdr->w, pmhdrLast);

						// Now move it

						WriteHeader(pheap, pmhdrLast, pmhdr, pmhdr->cbBlock);
						pmhdr = GetNext(pmhdrLast);

						// Now write the new free block

						WriteHeader(pheap, pmhdr, &mhdrFree);

						// Update the free list link

						if (pmhdrFreeLast != NULL) {
							MemHeader mhdr = *pmhdrFreeLast;
							mhdr.w = (word)((byte *)pmhdr - pheap->pbHeap);
							WriteHeader(pheap, pmhdrFreeLast, &mhdr);
						} else {
							pheap->pmhdrFreeFirst = pmhdr;
						}

						// Keep the last "free last" since we just moved pmhdrLast

						pmhdrFreeLastNext = pmhdrFreeLast;
					}
				}
			}
		}

		// Track largest free

		if (IsFree(pmhdr)) {
			if (pmhdr->cbBlock > cbLargestFreeBlock)
				cbLargestFreeBlock = pmhdr->cbBlock;
		}

		// If this block is free and is large enough, return with it

		if (IsFree(pmhdr) && pmhdr->cbBlock >= cbBlock) {
			Assert((pmhdrFreeLast == NULL && pheap->pmhdrFreeFirst == pmhdr) || GetNextFree(pmhdrFreeLast) == pmhdr);
			pmhdrUse = pmhdr;
			*ppmhdrFreeLast = pmhdrFreeLast;

			// If this object will get not get locked, then break now to alloc it towards the front

			if (!(wfHints & kfHintWillLock))
				break;
		}

		// Next block...

		pmhdrFreeLast = pmhdrFreeLastNext;
		pmhdrLast = pmhdr;
		pmhdr = GetNext(pmhdr);
		Assert(pmhdr > pmhdrLast);
	}

	// If we completed, this info is up to date

	if (pmhdr == pmhdrHeapEnd) {
		pheap->cbLargestFreeBlock = cbLargestFreeBlock;
		pheap->wf |= kfHeapFullyCompacted | kfHeapLargestFreeBlockSizeValid;
	}

#ifdef INCL_VALIDATEHEAP
	Validate();
#endif

	// Return what we found

	return pmhdrUse;
}

void MemMgr::FreeHandle(HMem hmem)
{
#ifdef INCL_VALIDATEHEAP
	ValidateHandle(hmem);
	Validate();
#endif

	Heap *pheap = GetHeap(hmem);
	MemHeader *pmhdr = GetMemHeader(hmem);
	Assert(!IsFree(pmhdr));

	// Free this handle

	FreeHandleEntry(pmhdr->w);

	// The free list is sorted. This makes it possible to compact easily, as well as
	// coalesce free blocks during freeing. Find the insertion point.

	MemHeader *pmhdrInsertAfter = NULL;
	if (pmhdr > pheap->pmhdrFreeFirst && pheap->pmhdrFreeFirst != NULL) {
		pmhdrInsertAfter = pheap->pmhdrFreeFirst;
		while (pmhdrInsertAfter != NULL) {
			Assert(IsFree(pmhdrInsertAfter));
			MemHeader *pmhdrT = GetNextFree(pmhdrInsertAfter);
			if (pmhdrT == NULL || pmhdrT > pmhdr)
				break;
			pmhdrInsertAfter = pmhdrT;
		}
	}

	// See if we have a free block immediately before

	MemHeader *pmhdrFreeBefore = NULL;
	if (pmhdrInsertAfter != NULL && GetNext(pmhdrInsertAfter) == pmhdr)
		pmhdrFreeBefore = pmhdrInsertAfter;

	// Now check to see if we have a free block immediately after

	MemHeader *pmhdrFreeAfter = NULL;
	MemHeader *pmhdrT = GetNext(pmhdr);
	if (pmhdrT != NULL && (byte *)pmhdrT < &pheap->pbHeap[pheap->cbHeap] && IsFree(pmhdrT)) {
		pmhdrFreeAfter = pmhdrT;
		Assert(pmhdrFreeAfter > pmhdrFreeBefore);
	}

	// About to free this much space

	m_cbTotalFreeBlocks += pmhdr->cbBlock;

	// Case out coalescing

	if (pmhdrFreeBefore == NULL) {
		if (pmhdrFreeAfter == NULL) {
			// No empty space surrounding

			if (pmhdrInsertAfter == NULL) {
				// This is the first sorted free block

				MemHeader mhdr;
				mhdr.w = 0;
				if (pheap->pmhdrFreeFirst != NULL)
					mhdr.w = (word)((byte *)pheap->pmhdrFreeFirst - pheap->pbHeap);
				mhdr.cbBlock = pmhdr->cbBlock;
				WriteHeader(pheap, pmhdr, &mhdr);
				pheap->pmhdrFreeFirst = pmhdr;
			} else {
				// This is not the first free block, but somewhere in the middle

				MemHeader mhdr;
				mhdr.w = pmhdrInsertAfter->w;
				mhdr.cbBlock = pmhdr->cbBlock;
				WriteHeader(pheap, pmhdr, &mhdr);
				mhdr.w = (word)((byte *)pmhdr - pheap->pbHeap);
				mhdr.cbBlock = pmhdrInsertAfter->cbBlock;
				WriteHeader(pheap, pmhdrInsertAfter, &mhdr);
			}

			// Update the largest free block if this block is even larger

			if (pmhdr->cbBlock > pheap->cbLargestFreeBlock)
				pheap->cbLargestFreeBlock = pmhdr->cbBlock;
		} else {
			// No empty space before, but have empty space after
			// First merge block after

			MemHeader mhdr = *pmhdrFreeAfter;
			mhdr.cbBlock += pmhdr->cbBlock;
			WriteHeader(pheap, pmhdr, &mhdr);

			// Now update the pmhdrInsertAfter

			if (pmhdrInsertAfter == NULL) {
				pheap->pmhdrFreeFirst = pmhdr;
			} else {
				mhdr = *pmhdrInsertAfter;
				mhdr.w = (word)((byte *)pmhdr - pheap->pbHeap);
				WriteHeader(pheap, pmhdrInsertAfter, &mhdr);
			}

			// Update largest block if this new block is larger

			if (pmhdr->cbBlock > pheap->cbLargestFreeBlock)
				pheap->cbLargestFreeBlock = pmhdr->cbBlock;
		}
	} else {
		if (pmhdrFreeAfter == NULL) {
			// Empty space before but none after
			// No free list fix up required

			MemHeader mhdrBefore = *pmhdrFreeBefore;
			mhdrBefore.cbBlock += pmhdr->cbBlock;
			WriteHeader(pheap, pmhdrFreeBefore, &mhdrBefore);

			// Update largest block if this new block is larger

			if (mhdrBefore.cbBlock > pheap->cbLargestFreeBlock)
				pheap->cbLargestFreeBlock = mhdrBefore.cbBlock;
		} else {
			// Empty space both behind and forwards
			// Minor free list fix up required

			MemHeader mhdrBefore = *pmhdrFreeBefore;
			mhdrBefore.w = pmhdrFreeAfter->w;
			mhdrBefore.cbBlock += pmhdr->cbBlock + pmhdrFreeAfter->cbBlock;
			WriteHeader(pheap, pmhdrFreeBefore, &mhdrBefore);

			// Update largest block if this new block is larger

			if (mhdrBefore.cbBlock > pheap->cbLargestFreeBlock)
				pheap->cbLargestFreeBlock = mhdrBefore.cbBlock;
		}
	}

	// The heap isn't fully compacted now

	pheap->wf &= ~kfHeapFullyCompacted;

#ifdef INCL_VALIDATEHEAP
	Validate();
#endif

#ifdef INCL_FORCECOMPACTION
	// Force compaction for validation purposes
	// This will validate again after compaction.

	Compact(pheap, 0xffff, NULL, 0);
#endif
}

void MemMgr::SetLocked(HMem hmem)
{
#ifdef INCL_VALIDATEHEAP
	ValidateHandle(hmem);
#endif
	// Set the locked bit

	MemHeader *pmhdr = GetMemHeader(hmem);
	Assert(!IsFree(pmhdr) && !IsLocked(pmhdr));
	MemHeader mhdr = *pmhdr;
	mhdr.w |= kfMhdrLocked;
	WriteHeader(GetHeap(hmem), pmhdr, &mhdr);
}

void MemMgr::ClearLocked(HMem hmem)
{
#ifdef INCL_VALIDATEHEAP
	ValidateHandle(hmem);
#endif
	// Clear the locked bit

	MemHeader *pmhdr = GetMemHeader(hmem);
	Assert(!IsFree(pmhdr) && IsLocked(pmhdr));
	MemHeader mhdr = *pmhdr;
	mhdr.w &= ~kfMhdrLocked;
	WriteHeader(GetHeap(hmem), pmhdr, &mhdr);

	// This block can move again; the heap potentially isn't fully compacted now

	GetHeap(hmem)->wf &= ~kfHeapFullyCompacted;
}

void *MemMgr::GetPtr(HMem hmem)
{
#ifdef INCL_VALIDATEHEAP
	ValidateHandle(hmem);
#endif
	MemHeader *pmhdr = GetMemHeader(hmem);
	return (void *)(pmhdr + 1);
}

void MemMgr::WriteHeader(Heap *pheap, MemHeader *pmhdrDst, MemHeader *pmhdrSrc, word cb)
{
#ifdef PIL
	if (pheap->wf & kfHeapDbRam) {
		DmWrite(pheap->pbHeap, (byte *)pmhdrDst - pheap->pbHeap, pmhdrSrc, cb);
	} else {
		memcpy(pmhdrDst, pmhdrSrc, cb);
	}
#else
	if (pmhdrSrc + cb <= pmhdrDst || pmhdrDst + cb <= pmhdrSrc) {
		memcpy(pmhdrDst, pmhdrSrc, cb);
	} else {
		memmove(pmhdrDst, pmhdrSrc, cb);
	}
#endif
}

void MemMgr::WriteHandle(HMem hmem, word ib, void *pvSrc, word cb)
{
#ifdef INCL_VALIDATEHEAP
	ValidateHandle(hmem);
#endif
	MemHeader *pmhdr = GetMemHeader(hmem);
	Assert(ib + cb <= pmhdr->cbBlock);
#ifdef PIL
	Heap *pheap = GetHeap(hmem);
	if (pheap->wf & kfHeapDbRam) {
		DmWrite(pheap->pbHeap, (byte *)(pmhdr + 1) - pheap->pbHeap + ib, pvSrc, cb);
	} else {
		memcpy(((byte *)(pmhdr + 1)) + ib, pvSrc, cb);
	}
#else
	memcpy(((byte *)(pmhdr + 1)) + ib, pvSrc, cb);
#endif
}

void *MemMgr::AllocPtr(word cb)
{
	// Alloc as a handle. Only reason we do this is so we know what heap this is stored in,
	// which we need in order to allow calls to WritePtr() which needs to know if it is
	// in dyn ram or db ram.

	HMem hmem = AllocHandle(cb + sizeof(hmem), kfHintWillLock);
	if (hmem == NULL)
		return NULL;
	SetLocked(hmem);
	WriteHandle(hmem, 0, &hmem, sizeof(hmem));
	return (void *)((HMem *)GetPtr(hmem) + 1);
}

void MemMgr::FreePtr(void *pv)
{
	// Copy handle byte at a time to satisfy ARM

	HMem hmem;
	byte *pbSrc = (byte *)((HMem *)pv - 1);
	byte *pbDst = (byte *)&hmem;
	*pbDst++ = *pbSrc++;
	*pbDst++ = *pbSrc++;
	*pbDst++ = *pbSrc++;
	*pbDst++ = *pbSrc++;
	ClearLocked(hmem);
	FreeHandle(hmem);
}

void MemMgr::WritePtr(void *pvDst, word ib, void *pvSrc, word cb)
{
	// Copy handle byte at a time to satisfy ARM

	HMem hmem;
	byte *pbSrc = (byte *)((HMem *)pvDst - 1);
	byte *pbDst = (byte *)&hmem;
	*pbDst++ = *pbSrc++;
	*pbDst++ = *pbSrc++;
	*pbDst++ = *pbSrc++;
	*pbDst++ = *pbSrc++;
	WriteHandle(hmem, ib + sizeof(HMem), pvSrc, cb);
}

#ifdef INCL_VALIDATEHEAP
void MemMgr::Validate()
{
	// Make sure all free objects are on the free list

	int iheap;
	for (iheap = 0; iheap < m_cheap; iheap++) {
		Heap *pheap = m_apheap[iheap];
		MemHeader *pmhdr = (MemHeader *)pheap->pbHeap;
		MemHeader *pmhdrHeapEnd = (MemHeader *)(pheap->pbHeap + pheap->cbHeap);
		for (; pmhdr < pmhdrHeapEnd; pmhdr = GetNext(pmhdr)) {
			if (IsFree(pmhdr)) {
				MemHeader *pmhdrT = pheap->pmhdrFreeFirst;
				for (; pmhdrT != NULL; pmhdrT = GetNextFree(pmhdrT)) {
					Assert(IsFree(pmhdrT));
					if (pmhdrT == pmhdr)
						break;
				}
				Assert(pmhdrT != NULL);
			}
		}
		Assert(pmhdr == pmhdrHeapEnd);
	}

	// Ensure all alloced objects have HandleEntries that are valid

	for (iheap = 0; iheap < m_cheap; iheap++) {
		Heap *pheap = m_apheap[iheap];
		MemHeader *pmhdr = (MemHeader *)pheap->pbHeap;
		MemHeader *pmhdrHeapEnd = (MemHeader *)(pheap->pbHeap + pheap->cbHeap);
		for (; pmhdr < pmhdrHeapEnd; pmhdr = GetNext(pmhdr)) {
			if (!IsFree(pmhdr)) {
				int ihtab = GetHandleTableIndex(pmhdr->w);
				int ihe = GetHandleEntryIndex(pmhdr->w);
				Assert(ihtab < m_chtab);
				word *pw = GetHandleEntryPtr(pmhdr->w);
				Assert(*pw == (word)((byte *)pmhdr - pheap->pbHeap));
			}
		}
		Assert(pmhdr == pmhdrHeapEnd);
	}

	// Ensure largest count / bit is valid, and that total count is valid

	dword cbTotal = 0;
	for (iheap = 0; iheap < m_cheap; iheap++) {
		dword cbLargest = 0;
		Heap *pheap = m_apheap[iheap];
		MemHeader *pmhdr = (MemHeader *)pheap->pbHeap;
		MemHeader *pmhdrHeapEnd = (MemHeader *)(pheap->pbHeap + pheap->cbHeap);
		for (; pmhdr < pmhdrHeapEnd; pmhdr = GetNext(pmhdr)) {
			if (!IsFree(pmhdr))
				continue;
			if (pmhdr->cbBlock > cbLargest)
				cbLargest = pmhdr->cbBlock;
			cbTotal += pmhdr->cbBlock;
		}
		Assert(pmhdr == pmhdrHeapEnd);
		if (pheap->wf & kfHeapLargestFreeBlockSizeValid)
			Assert(cbLargest == pheap->cbLargestFreeBlock);
		Assert(cbLargest <= pheap->cbHeap);
	}
	Assert(cbTotal == m_cbTotalFreeBlocks);

	// Ensure the compacted bit is valid. Meaning there are not two contiguous free blocks, and that
	// there are no free blocks followed by unlocked blocks.

	for (iheap = 0; iheap < m_cheap; iheap++) {
		Heap *pheap = m_apheap[iheap];
		if (!(pheap->wf & kfHeapFullyCompacted))
			continue;
		MemHeader *pmhdrLast = NULL;
		MemHeader *pmhdr = (MemHeader *)pheap->pbHeap;
		MemHeader *pmhdrHeapEnd = (MemHeader *)(pheap->pbHeap + pheap->cbHeap);
		for (; pmhdr < pmhdrHeapEnd; pmhdr = GetNext(pmhdr)) {
			if (pmhdrLast != NULL) {
				if (IsFree(pmhdrLast) && IsFree(pmhdr))
					Assert(false);
				if (IsFree(pmhdrLast) && !IsLocked(pmhdr))
					Assert(false);
			}
		}
		Assert(pmhdr == pmhdrHeapEnd);
	}

	ValidateHandleTableFreeEntries();
}

void MemMgr::ValidateHandle(HMem hmem)
{
	Assert(hmem != 0);
	HMemStruct *phms = (HMemStruct *)&hmem;
	Assert(phms->iheap < m_cheap);
	Assert(GetHandleTableIndex(phms->her) < m_chtab);
	MemHeader *pmhdr = GetMemHeader(hmem);
	Heap *pheap = GetHeap(hmem);
	Assert((byte *)pmhdr >= pheap->pbHeap && ((byte *)GetNext(pmhdr)) <= pheap->pbHeap + pheap->cbHeap);
	Assert((pmhdr->w & ~(kfMhdrBusy | kfMhdrLocked)) == (phms->her & ~(kfMhdrBusy | kfMhdrLocked)));
}

void MemMgr::ValidateHandleTableFreeEntries()
{
	for (int ihtab = 0; ihtab < m_chtab; ihtab++) {
		// Make sure the free list is good

		bool afVisited[kcHandlesPerTable];
		memset(afVisited, 0, sizeof(afVisited));
		HandleTable *phtab = m_aphtab[ihtab];
		word iheFree = (word)phtab->iheFree;
		while (iheFree != (word)-1) {
			Assert(iheFree < ARRAYSIZE(phtab->ahe));
			Assert(!afVisited[iheFree]);
			afVisited[iheFree] = true;
			iheFree = phtab->ahe[iheFree];
		}

		// Make sure the rest of them could be valid offsets (even)

		for (int ihe = 0; ihe < ARRAYSIZE(phtab->ahe); ihe++) {
			if (afVisited[ihe] == false)
				Assert((phtab->ahe[ihe] & 1) == 0);
		}
	}


}
#endif

} // namespace wi
