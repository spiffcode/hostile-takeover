#include "ht.h"

namespace wi {

dword *TBitmap::s_ampscaiclrSide[kcColoredSides];

int gcyClipBuffer;
TBitmap *LoadTBitmap(char *pszFn)
{
	// Init clip buffer size for quick query

	if (gpbmClip != NULL) {
		Size sizClip;
		gpbmClip->GetSize(&sizClip);
		gcyClipBuffer = sizClip.cy;
	}

	TBitmap *ptbm = new TBitmap();
	if (ptbm == NULL)
		return NULL;
	if (!ptbm->Init(pszFn)) {
		delete ptbm;
		return NULL;
	}

	return ptbm;
}

// Some things want to pool shared TBitmaps

struct SharedRecord
{
	TBitmap *ptbm;
	char sz[1];
};

int gcshr;
int gcshrAlloc;
SharedRecord **gapshr;

void FreeSharedTBitmaps()
{
	for (int n = 0; n < gcshr; n++) {
		SharedRecord *pshr = gapshr[n];
		TBitmap *ptbm = pshr->ptbm;
		ptbm->ClearShared();
		delete pshr->ptbm;
		gmmgr.FreePtr(pshr);
	}
	delete gapshr;
	gapshr = NULL;
	gcshr = 0;
	gcshrAlloc = 0;
}

void FindSharedTBitmapFilename(TBitmap *ptbm, char *psz, int cb)
{
	*psz = 0;
	for (int n = 0; n < gcshr; n++) {
		SharedRecord *pshr = gapshr[n];
		if (pshr->ptbm == ptbm) {
			strncpyz(psz, pshr->sz, cb);
			return;
		}
	}

	Assert();
}

#define kcshrGrow 32

TBitmap *GetSharedTBitmap(char *pszFn)
{
	// First try to find it
	// Note could be faster with a binary search assuming the names are sorted

	SharedRecord **ppshrMax = &gapshr[gcshr];
	for (SharedRecord **ppshr = gapshr; ppshr < ppshrMax; ppshr++) {
		SharedRecord *pshr = *ppshr;
		if (strcmp(pszFn, pshr->sz) == 0) {
			return pshr->ptbm;
		}
	}

	// Couldn't find it, add it to the list
	// Grow list?

	if (gcshr == gcshrAlloc) {
		SharedRecord **ppshr = new SharedRecord *[gcshrAlloc + kcshrGrow];
		if (ppshr == NULL)
			return NULL;
		if (gapshr != NULL) {
			memcpy(ppshr, gapshr, sizeof(SharedRecord *) * gcshr);
			delete gapshr;
		}
		gapshr = ppshr;
		gcshrAlloc += kcshrGrow;
	}
	Assert(gcshrAlloc > gcshr);

	// Load the TBitmap

	TBitmap *ptbm = LoadTBitmap(pszFn);
	if (ptbm == NULL)
		return NULL;
	ptbm->SetShared();

	// Alloc record

	SharedRecord *pshr = (SharedRecord *)gmmgr.AllocPtr(sizeof(SharedRecord) - 1 + strlen(pszFn) + 1);
	if (pshr == NULL) {
		delete ptbm;
		return NULL;
	}

	// Add it to the list and fill structure

	gapshr[gcshr] = pshr;
	gcshr++;
	gmmgr.WritePtr(pshr, OFFSETOF(SharedRecord, ptbm), &ptbm, sizeof(ptbm));
	gmmgr.WritePtr(pshr, OFFSETOF(SharedRecord, sz), pszFn, strlen(pszFn) + 1);

	// Done

	return ptbm;
}

bool TBitmap::InitClass()
{
	static int aiclr[kcColoredSides][2] = {
			{ kiclrNeutralSideFirst, kiclrNeutralSideLast },
			{ kiclrBlueSideFirst, kiclrBlueSideLast },
			{ kiclrRedSideFirst, kiclrRedSideLast },
			{ kiclrYellowSideFirst, kiclrYellowSideLast },
			{ kiclrCyanSideFirst, kiclrCyanSideLast },
			{ kiclrWhite, kiclrWhite }
	};

	// Create the side code lookup tables

#define kcSideColors 5
#define kcSideCodeEntries (kcSideColors * kcSideColors * kcSideColors * kcSideColors)

	dword *mpscaiclrSideT = new dword[kcSideCodeEntries];
	word cbAlloc = kcSideCodeEntries * sizeof(dword);

	for (int i = 0; i < kcColoredSides; i++) {
		Color aclr[5];
		int iclrFirst = aiclr[i][0];
		int iclrLast = aiclr[i][1];
		if (iclrFirst == iclrLast) {
			for (int i = 0; i < 5; i++)
				aclr[i] = GetColor(iclrFirst);
		} else {
			Assert(iclrLast - iclrFirst + 1 == 5);
			for (int iclr = iclrFirst; iclr <= iclrLast; iclr++)
				aclr[iclr - iclrFirst] = GetColor(iclr);
		}

		MakeSideCodeMapping(aclr, mpscaiclrSideT);

		// We need more side code entries in order to flash a hires Replicator white

		if (iclrFirst == iclrLast && gcxTile >= 24)
			s_ampscaiclrSide[i] = (dword *)gmmgr.AllocPtr(cbAlloc * 2);
		else
			s_ampscaiclrSide[i] = (dword *)gmmgr.AllocPtr(cbAlloc);
		if (s_ampscaiclrSide[i] == NULL) {
			delete mpscaiclrSideT;
			return false;
		}
		gmmgr.WritePtr(s_ampscaiclrSide[i], 0, mpscaiclrSideT, cbAlloc);
		if (iclrFirst == iclrLast && gcxTile >= 24)
			gmmgr.WritePtr(s_ampscaiclrSide[i], cbAlloc, mpscaiclrSideT, cbAlloc);
	}
	delete mpscaiclrSideT;

	// Done

	return true;
}

void TBitmap::MakeSideCodeMapping(Color *aclr, dword *mpscaiclrSide)
{
	int sc = 0;
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			for (int k = 0; k < 5; k++) {
				for (int m = 0; m < 5; m++) {
					byte ab[4];
					ab[0] = (byte)aclr[i];
					ab[1] = (byte)aclr[j];
					ab[2] = (byte)aclr[k];
					ab[3] = (byte)aclr[m];
					mpscaiclrSide[sc++] = *((dword *)ab);
				}
			}
		}
	}
}

void TBitmap::ExitClass()
{
	for (int i = 0; i < kcColoredSides; i++) {
        gmmgr.FreePtr(s_ampscaiclrSide[i]);
		s_ampscaiclrSide[i] = NULL;
	}
}

TBitmap::TBitmap()
{
	m_pfil = NULL;
	m_ibtbh = 0;
	m_wf = 0;
	m_ctbm = 0;
	m_atbe = NULL;
	m_ahc = NULL;
}

TBitmap::~TBitmap()
{
	Assert(!(m_wf & kfTbShared));

	delete m_ahc;
	m_ahc = NULL;
	if (m_atbe != NULL) {
		gmmgr.FreePtr(m_atbe);
		m_atbe = NULL;
	}
	if (m_pfil != NULL && (m_wf & kfTbCloseFile) != 0) {
		gpakr.fclose(m_pfil);
		m_pfil = NULL;
	}
}

bool TBitmap::Init(char *pszFn)
{
	File *pfil = gpakr.fopen(pszFn, "rb");
	if (pfil == NULL) {
		return false;
    }
	if (!Init(pfil, 0)) {
		gpakr.fclose(pfil);
		return false;
	}
	m_wf |= kfTbCloseFile;
	return true;
}

bool TBitmap::Init(File *pfil, word ib)
{
	// How many?

	if (gpakr.fseek(pfil, (long)ib, SEEK_SET) != 0)
		return false;
	word ctbmT;
	if (gpakr.fread(&ctbmT, sizeof(ctbmT), 1, pfil) == 0)
		return false;
	m_ctbm = BigWord(ctbmT);

	// Alloc array of cache handles. These get stored in ram

	m_ahc = new CacheHandle[m_ctbm * 2];
	if (m_ahc == NULL)
		return false;
	memset(m_ahc, 0, sizeof(CacheHandle) * m_ctbm * 2);

	// Allocate space for entry headers, read them in, store
	// in db ram.

	TBitmapEntry *ptbe = new TBitmapEntry[m_ctbm];
	if (ptbe == NULL)
		return false;
    for (int itbe = 0; itbe < m_ctbm; itbe++) {
        TBitmapEntry *ptbeT = &ptbe[itbe];
        if ((int)gpakr.fread(ptbeT, kcbTBitmapEntry, 1, pfil) != 1) {
            return false;
        }
#ifndef __CPU_68K
        // Swap bytes (non-Palm only)

		ptbeT->cx = BigWord(ptbeT->cx);
		ptbeT->cy = BigWord(ptbeT->cy);
		ptbeT->yBaseline = BigWord(ptbeT->yBaseline);
		ptbeT->ibsd = BigWord(ptbeT->ibsd);
		ptbeT->cbsd = BigWord(ptbeT->cbsd);
#endif
    }

	word cbAlloc = sizeof(TBitmapEntry) * m_ctbm;
	TBitmapEntry *ptbeT = (TBitmapEntry *)gmmgr.AllocPtr(cbAlloc);
	if (ptbeT == NULL) {
		delete ptbe;
		return false;
	}
	gmmgr.WritePtr(ptbeT, 0, ptbe, cbAlloc);
	m_atbe = ptbeT;
	delete ptbe;

	// Remember file and offset

	m_pfil = pfil;
	m_ibtbh = ib;
	return true;
}

void TBitmap::GetSize(Size *psiz)
{
	GetSize(0, psiz);
}

void TBitmap::GetSize(int itbm, Size *psiz)
{
	Assert(itbm >= 0 && itbm < m_ctbm);
	psiz->cx = (int)m_atbe[itbm].cx;
	psiz->cy = (int)m_atbe[itbm].cy;
}

extern "C" {
void CopyBits128Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits124Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits120Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits116Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits112Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits108Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits104Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits100Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits96Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits92Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits88Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits84Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits80Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits76Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits72Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits68Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits64Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits60Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits56Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits52Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits48Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits44Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits40Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits36Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits32Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits28Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits24Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits20Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits16Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits12Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits8Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
void CopyBits4Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy) secCode8;
};

typedef void (*CopyBy4Proc)(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy);
CopyBy4Proc gapfnCopyBy4[kcCopyBy4Procs] = {
	CopyBits4Wide, CopyBits8Wide, CopyBits12Wide, CopyBits16Wide, CopyBits20Wide, CopyBits24Wide, 
	CopyBits28Wide, CopyBits32Wide, CopyBits36Wide, CopyBits40Wide, CopyBits44Wide, CopyBits48Wide,
	CopyBits52Wide, CopyBits56Wide, CopyBits60Wide, CopyBits64Wide, CopyBits68Wide, CopyBits72Wide,
	CopyBits76Wide, CopyBits80Wide, CopyBits84Wide, CopyBits88Wide, CopyBits92Wide, CopyBits96Wide,
	CopyBits100Wide, CopyBits104Wide, CopyBits108Wide, CopyBits112Wide, CopyBits116Wide, CopyBits120Wide,
	CopyBits124Wide, CopyBits128Wide
};

void TBitmap::BltTo(class DibBitmap *pbmDst, int xDst, int yDst, Side side, Rect *prcSrc)
{
	BltTo(0, pbmDst, xDst, yDst, side, prcSrc);
}

void TBitmap::BltTo(class DibBitmap *pbmDst, int xDst, int yDst, Rect *prcSrc)
{
	BltTo(0, pbmDst, xDst, yDst, ksideNeutral, prcSrc);
}

void TBitmap::BltTo(int itbm, class DibBitmap *pbmDst, int xDst, int yDst, Side side, Rect *prcSrc)
{
	// Get tbm dimensions

	Assert(itbm >= 0 && itbm < m_ctbm);
	int cx = (int)m_atbe[itbm].cx;
	int cy = (int)m_atbe[itbm].cy;
   
	// Source rect to blt from

	Rect rcSrc;
	if (prcSrc != NULL) {
		rcSrc = *prcSrc;
	} else {
		rcSrc.left = 0;
		rcSrc.top = 0;
		rcSrc.right = cx;
		rcSrc.bottom = cy;
	}

	// Get dib dimensions

	Size sizT;
	pbmDst->GetSize(&sizT);

	// Right and bottom edge clipping

	if (sizT.cx - xDst < rcSrc.Width())
		rcSrc.right = rcSrc.left + sizT.cx - xDst;
	if (sizT.cy - yDst < rcSrc.Height())
		rcSrc.bottom = rcSrc.top + sizT.cy - yDst;

	// Left and top edge clipping

	int xDstT = xDst;
	if (xDstT < 0) {
		rcSrc.left -= xDstT;
		xDstT = 0;
	}
	int yDstT = yDst;
	if (yDstT < 0) {
		rcSrc.top -= yDstT;
		yDstT = 0;
	}

	// Anything to blt?

	if (rcSrc.IsEmpty())
		return;

#ifdef STATS_DISPLAY
	extern int gcBitmapsDrawn;
	gcBitmapsDrawn++;
#endif

#if 1
	byte *pbBits = pbmDst->GetBits();
#else
	BitmapType *pbmpScreen = WinGetBitmap(WinGetDisplayWindow());
	byte *pbBits = (byte *)BmpGetBits(pbmpScreen);
	Fill(pbBits, 160, 160, 160, 1);
#endif

	// Look in the cache to see if this tbitmap has been compiled

	byte *pbDraw = GetCompiledBits(itbm, (xDst & 1) != 0);
	if (pbDraw == NULL)
		return;

	// If it is not clipped, draw directly to the dib, otherwise clip.

	if (rcSrc.Width() == cx && rcSrc.Height() == cy) {
		// Draw

		int cbRowDst = (sizT.cx + 1) & ~1;
		byte *pbDst = pbBits + (long)yDstT * cbRowDst + xDstT;

		if ((short)side < 0) {
			DrawDispatchThunk(pbDraw, (byte *)s_ampscaiclrSide[5], pbDst, cbRowDst - cx, s_ampscaiclrSide[5], gmpiclriclrShadow);
		} else {
			DrawDispatchThunk(pbDraw, NULL, pbDst, cbRowDst - cx, s_ampscaiclrSide[side], gmpiclriclrShadow);
		}
	} else {
        // If there is no fast copy proc for this size, then blt this slower
        // way, which can clip any size.

        int cxMax = kcCopyBy4Procs * 4;
        if (cx > cxMax || cy > cxMax) {
            BltToScan(pbDraw, cx, cy, pbmDst, xDstT, yDstT, side, &rcSrc);
            return;
        }

        if (gpbmClip != NULL && gcyClipBuffer >= cy) {
			Rect rc;
			rc.left = xDstT & ~3;
			rc.top = yDstT;
			rc.right = rc.left + (((xDstT & 3) + rcSrc.Width() + 3) & ~3);
			rc.bottom = rc.top + rcSrc.Height();
			int xDstClip = (((xDst & 3) + rcSrc.left) & ~3);

			gpbmClip->Blt(pbmDst, &rc, xDstClip, rcSrc.top);
			byte *pbDrawAt = gpbmClip->GetBits() + (xDst & 3);
			Size sizClip;
			gpbmClip->GetSize(&sizClip);
			int cbDrawReturn = sizClip.cx - cx;

			if ((short)side < 0) {
				DrawDispatchThunk(pbDraw, (byte *)s_ampscaiclrSide[5], pbDrawAt, cbDrawReturn, s_ampscaiclrSide[5], gmpiclriclrShadow);
			} else {
				DrawDispatchThunk(pbDraw, NULL, pbDrawAt, cbDrawReturn, s_ampscaiclrSide[side], gmpiclriclrShadow);
			}

			rc.Offset(xDstClip - rc.left, rcSrc.top - rc.top);
			pbmDst->Blt(gpbmClip, &rc, xDstT & ~3, yDstT);
		} else {
			// We want to use gpbScratch to hold the screen bits, so if our cache alloc failed, don't
			// do the operation

			if (pbDraw == gpbScratch)
				return;

			// Copy from screen, draw, copy back to screen

			int xCopy = xDstT & ~3;
			byte *pbDib = pbBits + (long)yDstT * sizT.cx + xCopy;

			int cbRunCopy = ((xDst & 3) + cx + 3) & ~3;
			byte *pbCopy = gpbScratch + rcSrc.top * cbRunCopy + (((xDst & 3) + rcSrc.left) & ~3);

			int cxCopy = ((xDstT & 3) + rcSrc.Width() + 3) & ~3;
			Assert((cxCopy >> 2) - 1 < kcCopyBy4Procs);
			if (cxCopy > kcCopyBy4Procs * 4)
				return;
			CopyBy4Proc pfnCopy = gapfnCopyBy4[(cxCopy >> 2) - 1];
			pfnCopy(pbDib, sizT.cx - cxCopy, pbCopy, cbRunCopy - cxCopy, rcSrc.Height());

			// Draw into buffer

			byte *pbDrawAt = gpbScratch + (xDst & 3);
			int cbDrawReturn = cbRunCopy - cx;

			if ((short)side < 0) {
				DrawDispatchThunk(pbDraw, (byte *)s_ampscaiclrSide[5], pbDrawAt, cbDrawReturn, s_ampscaiclrSide[5], gmpiclriclrShadow);
			} else {
				DrawDispatchThunk(pbDraw, NULL, pbDrawAt, cbDrawReturn, s_ampscaiclrSide[side], gmpiclriclrShadow);
			}

			// Copy back to screen

			pfnCopy(pbCopy, cbRunCopy - cxCopy, pbDib, sizT.cx - cxCopy, rcSrc.Height());
		}
	}
}

void TBitmap::BltToScan(byte *pbDraw, int cx, int cy, DibBitmap *pbmDst,
        int xDst, int yDst, Side side, Rect *prcSrc)
{
    // Clipping has already been performed

#ifdef STATS_DISPLAY
	extern int gcBitmapsDrawn;
	gcBitmapsDrawn++;
#endif

	byte *pbBits = pbmDst->GetBits();
    Size sizT;
    pbmDst->GetSize(&sizT);

    int cbRowDst = (sizT.cx + 1) & ~1;
    byte *pbDst = pbBits + (long)yDst * cbRowDst + xDst;

    word *pw = (word *)pbDraw;
    word *psc = (word *)(pbDraw + pw[0]);
    byte *pbSrc = pbDraw + pw[1];
    byte *pop = pbDraw + sizeof(word) + sizeof(word);

    // Scan past any piece of the source vertically clipped
    // at the top

    int y = 0;
    int xoffset = 0;
    if (prcSrc->top > 0) {
        xoffset = YClipToScan(prcSrc->top, cx, pop, pbSrc, psc);
        y = prcSrc->top;
    }

	if (prcSrc->Width() == cx) {
        // Not horz clipped; draw each scan directly to dest
        pbDst += xoffset;
        for (; y < prcSrc->bottom; y++) {
            xoffset = DrawScan(pbDst, cx, pbSrc, pop, psc,
                    s_ampscaiclrSide[side], gmpiclriclrShadow);
            pbDst += xoffset + (cbRowDst - cx);
        }
	} else {
        // Horz clipped; copy from dst to temp scan, draw onto scan,
        // copy from scan back to dst

        byte *pbScan = gpbmClip->GetBits();
        byte *pbCopyTo = pbScan + prcSrc->left;

        // Draw the scans
        for (; y < prcSrc->bottom; y++) {
            memcpy(pbCopyTo, pbDst, prcSrc->Width());
            xoffset += DrawScan(&pbScan[xoffset], cx, pbSrc, pop, psc,
                    s_ampscaiclrSide[side], gmpiclriclrShadow) - cx;
            memcpy(pbDst, pbCopyTo, prcSrc->Width());
            pbDst += cbRowDst;
        }
	}
}

byte *TBitmap::GetCompiledBits(int itbm, bool fOdd)
{
	CacheHandle *phc = &m_ahc[itbm * 2 + (fOdd ? 1 : 0)];
	byte *pbDraw = (byte *)gcam.GetPtr(*phc);
	if (pbDraw == NULL) {
		// Need to get the ScanData first. This may fail.
		// Note: load ScanData into end gpbScratch, compile to
		// beginning. _Should_ work.

		TBitmapEntry *ptbe = &m_atbe[itbm];
		if (gpakr.fseek(m_pfil, m_ibtbh + ptbe->ibsd, SEEK_SET) != 0)
			return NULL;
		ScanData *psd = (ScanData *)(gpbScratch + gcbScratch - ((ptbe->cbsd + 1) & ~1));
		if (gpakr.fread(psd, ptbe->cbsd, 1, m_pfil) != 1)
			return NULL;
		word cb = Compile8Thunk(gpbScratch, psd, (fOdd ? 1 : 0));
		*phc = gcam.NewObject(gpbScratch, cb);
		pbDraw = (byte *)gcam.GetPtr(*phc);
		if (pbDraw == NULL)
			pbDraw = gpbScratch;
	}
	return pbDraw;
}

void TBitmap::FillTo(int itbm, class DibBitmap *pbmDst, int xDst, int yDst, int cxDst, int cyDst, int xOrigin, int yOrigin)
{
	// Clip to the dib

	int cxTbm = m_atbe[itbm].cx;
	int cyTbm = m_atbe[itbm].cy;

	if (xDst < 0) {
		xOrigin = -(xDst - xOrigin) % cxTbm;
		cxDst += xDst;
		xDst = 0;
	}
	if (yDst < 0) {
		yOrigin = -(yDst - yOrigin) % cyTbm;
		cyDst += yDst;
		yDst = 0;
	}
	Size sizDib;
	pbmDst->GetSize(&sizDib);
	if (xDst + cxDst > sizDib.cx)
		cxDst = sizDib.cx - xDst;
	if (yDst + cyDst > sizDib.cy)
		cyDst = sizDib.cy - yDst;

	// Determine what the non-clipped "inside" is

	int xLeftInside;
	if (xOrigin == 0) {
		xLeftInside = xDst;
	} else {
		xLeftInside = xDst + cxTbm - xOrigin;
	}

	int yTopInside;
	if (yOrigin == 0) {
		yTopInside = yDst;
	} else {
		yTopInside = yDst + cyTbm - yOrigin;
	}

	int xRightInside;
	if (xOrigin == 0 && cxDst == cxTbm) {
		xRightInside = xDst + cxDst;
	} else {
		xRightInside = xDst + cxDst - ((xDst + cxDst) - (xDst - xOrigin)) % cxTbm;
	}

    int yBottomInside;
	if (yOrigin == 0 && cyDst == cyTbm) {
		yBottomInside = yDst + cyDst;
	} else {
		yBottomInside = yDst + cyDst - ((yDst + cyDst) - (yDst - yOrigin)) % cyTbm;
	}

	// Draw inside if there is anything to do
	// OPT: Could write this part in assembly

	if (yTopInside < yBottomInside && xLeftInside < xRightInside) {
		int cbRowDst = pbmDst->GetRowBytes();
		byte *pbRow = pbmDst->GetBits() + (long)yTopInside * sizDib.cx + xLeftInside;
		word cbNextRow = cyTbm * cbRowDst;
		byte *pbDraw = GetCompiledBits(itbm, (xLeftInside & 1) != 0);
		if (pbDraw == NULL)
			return;
		for (int y = yTopInside; y < yBottomInside; y += cyTbm) {
			byte *pbDst = pbRow;
			for (int x = xLeftInside; x < xRightInside; x += cxTbm) {
				DrawDispatchThunk(pbDraw, NULL, pbDst, cbRowDst - cxTbm, s_ampscaiclrSide[kside1], gmpiclriclrShadow);
				pbDst += cxTbm;
			}
			pbRow += cbNextRow;
		}
	}

	// Draw edges

	if (xLeftInside <= xRightInside) {
		if (xLeftInside != xDst) {
			Rect rcT;
			rcT.Set(-xOrigin, -yOrigin, cxTbm - xOrigin, cyTbm - yOrigin);
			for (int y = yDst; y < yDst + cyDst; y += cyTbm) {
				BltTo(itbm, pbmDst, xLeftInside - cxTbm, y, kside1, /*&rcT*/ NULL);
				rcT.top = 0;
				rcT.bottom = (yDst + cyDst) - y;
				if (rcT.bottom > cyTbm)
					rcT.bottom = cyTbm;
			}
		}
		if (xRightInside != xDst + cxDst) {
			Rect rcT;
			rcT.Set(0, -yOrigin, xDst + cxDst - xRightInside, cyTbm - yOrigin);
			for (int y = yDst; y < yDst + cyDst; y += cyTbm) {
				BltTo(itbm, pbmDst, xRightInside, y, kside1, /*&rcT*/ NULL);
				rcT.top = 0;
				rcT.bottom = (yDst + cyDst) - y;
				if (rcT.bottom > cyTbm)
					rcT.bottom = cyTbm;
			}
		}
	}
	if (yTopInside <= yBottomInside) {
		if (yTopInside != yDst) {
			Rect rcT;
			rcT.Set(0, -yOrigin, cxTbm, cyTbm - yOrigin);
			for (int x = xLeftInside; x < xRightInside; x += cxTbm) {
				rcT.right = xRightInside - x;
				if (rcT.right > cxTbm)
					rcT.right = cxTbm;
				BltTo(itbm, pbmDst, x, yTopInside, kside1, &rcT);
			}
		}
		if (yBottomInside != yDst + cyDst) {
			Rect rcT;
			rcT.Set(0, -yOrigin, cxTbm, cyTbm - yOrigin);
			for (int x = xLeftInside; x < xRightInside; x += cxTbm) {
				rcT.right = xRightInside - x;
				if (rcT.right > cxTbm)
					rcT.right = cxTbm;
				BltTo(itbm, pbmDst, x, yBottomInside, kside1, &rcT);
			}
		}
	}
}

// C code for compiling / drawing

#ifndef __CPU_68K
void CopyBits128Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits124Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits120Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits116Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits112Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits108Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits104Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits100Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits96Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits92Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits88Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits84Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits80Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits76Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits72Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits68Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits64Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits60Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits56Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits52Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits48Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits44Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits40Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits36Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits32Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits28Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits24Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits20Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits16Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits12Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits8Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}

void CopyBits4Wide(byte *pbSrc, int cbSrcReturn, byte *pbDst, int cbDstReturn, int cy)
{
	dword *pdwSrc = (dword *)pbSrc;
	dword *pdwDst = (dword *)pbDst;
	while (cy-- != 0) {
		*pdwDst++ = *pdwSrc++;
		pdwSrc = (dword *)((byte *)pdwSrc + cbSrcReturn);
		pdwDst = (dword *)((byte *)pdwDst + cbDstReturn);
	}
}
#endif

} // namespace wi
