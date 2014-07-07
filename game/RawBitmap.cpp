#include "ht.h"

namespace wi {

// This exists soley for the purpose of being overridden

HtBitmap::~HtBitmap()
{
}

RawBitmap *LoadRawBitmap(char *pszFn)
{
	RawBitmap *prbm = new RawBitmap();
	if (prbm == NULL)
		return NULL;
	if (!prbm->Init(pszFn)) {
		delete prbm;
		return NULL;
	}
	return prbm;
}

RawBitmap::RawBitmap()
{
	m_pfil = NULL;
	m_cx = 0;
	m_cy = 0;
}

RawBitmap::~RawBitmap()
{
	if (m_pfil != NULL) {
		gpakr.fclose(m_pfil);
		m_pfil = NULL;
	}
}

bool RawBitmap::Init(char *pszFn)
{
	File *pfil = gpakr.fopen(pszFn, "rb");
	if (pfil == NULL)
		return false;

	short cx;
	if (gpakr.fread(&cx, sizeof(cx), 1, pfil) != 1) {
		gpakr.fclose(pfil);
		return false;
	}

	short cy;
	if (gpakr.fread(&cy, sizeof(cy), 1, pfil) != 1) {
		gpakr.fclose(pfil);
		return false;
	}

	m_pfil = pfil;
	m_cx = BigWord(cx);
	m_cy = BigWord(cy);
	return true;
}

void RawBitmap::GetSize(Size *psiz)
{
	psiz->cx = m_cx;
	psiz->cy = m_cy;
}

#if 0
// fread based blt

void RawBitmap::BltTo(DibBitmap *pbmDst, int xDst, int yDst, Rect *prcSrc)
{
	// Get dib dimensions

	Size sizDib;
	pbmDst->GetSize(&sizDib);

	// Src rect to blt from

	if (prcSrc == NULL) {
		Rect rcSrcT;
		rcSrcT.Set(0, 0, sizDib.cx, sizDib.cy);
		prcSrc = &rcSrcT;
	}

	// Right and bottom edge clipping

	if (sizDib.cx - xDst < prcSrc->Width())
		prcSrc->right = prcSrc->left + sizDib.cx - xDst;
	if (sizDib.cy - yDst < prcSrc->Height())
		prcSrc->bottom = prcSrc->top + sizDib.cy - yDst;

	// Left and top edge clipping

	int xDstT = xDst;
	if (xDstT < 0) {
		prcSrc->left -= xDstT;
		xDstT = 0;
	}
	int yDstT = yDst;
	if (yDstT < 0) {
		prcSrc->top -= yDstT;
		yDstT = 0;
	}

	// Anything to blt?

	if (prcSrc->IsEmpty())
		return;

	// Get destination address and dest row bytes

	int cbRow = pbmDst->GetRowBytes();
	byte *pbDst = pbmDst->GetBits() + (long)yDstT * cbRow + xDstT;

	// Seek to the start of bits

	gpakr.fseek(m_pfil, sizeof(word) * 2 + (long)prcSrc->top * m_cx + prcSrc->left, SEEK_SET);

	// Can't use the scratch buffer to chunk because the decompressor
	// uses it! Use this lame approach for now.

	int cbSrcReturn = m_cx - prcSrc->Width();
	int cy = prcSrc->Height();
	for (int y = 0; y < cy; y++) {
		if (gpakr.fread(pbDst, prcSrc->Width(), 1, m_pfil) != 1)
			return;
		if (cbSrcReturn != 0)
			gpakr.fseek(m_pfil, cbSrcReturn, SEEK_CUR);
		pbDst += cbRow;
	}
}
#else
// Record mapping based blt

void RawBitmap::BltTo(class DibBitmap *pbmDst, int xDst, int yDst, Side side, Rect *prcSrc)
{
	BltTo(pbmDst, xDst, yDst, prcSrc);
}

void RawBitmap::BltTo(DibBitmap *pbmDst, int xDst, int yDst, Rect *prcSrc)
{
	// Src rect to blt from

	if (prcSrc == NULL) {
		Rect rcSrcT;
		rcSrcT.Set(0, 0, m_cx, m_cy);
		prcSrc = &rcSrcT;
	}

	// Right and bottom edge clipping

	Size sizDib;
	pbmDst->GetSize(&sizDib);
	if (sizDib.cx - xDst < prcSrc->Width())
		prcSrc->right = prcSrc->left + sizDib.cx - xDst;
	if (sizDib.cy - yDst < prcSrc->Height())
		prcSrc->bottom = prcSrc->top + sizDib.cy - yDst;

	// Left and top edge clipping

	int xDstT = xDst;
	if (xDstT < 0) {
		prcSrc->left -= xDstT;
		xDstT = 0;
	}
	int yDstT = yDst;
	if (yDstT < 0) {
		prcSrc->top -= yDstT;
		yDstT = 0;
	}

	// Anything to blt?

	if (prcSrc->IsEmpty())
		return;

	// Get destination address and dest row bytes

	int cbRowDst = pbmDst->GetRowBytes();
	byte *pbDst = pbmDst->GetBits() + (long)yDstT * cbRowDst + xDstT;

	// Seek to the start of bits

	long offBits = sizeof(word) * 2 + (long)prcSrc->top * m_cx + prcSrc->left;
	gpakr.fseek(m_pfil, offBits, SEEK_SET);
	int nRecCurrent = BigWord(m_pfil->nRecFirst) + m_pfil->nRecOffStream;
	dword offRecCurrent = m_pfil->offRecStart;

	// Try to blt chunks as large as possible. If a scan intersects a record boundary, handle it
	// specially.

	int cxBlt = prcSrc->Width();
	int cyLeft = prcSrc->Height();
	byte *pbRec = NULL;
	word cbRec;
	dword dwCookie;
	while (cyLeft > 0) {
		// See how much we can blt from the current record

		if (pbRec == NULL) {
			pbRec = m_pfil->prnfo->ppdbReader->MapRecord(nRecCurrent, &dwCookie, &cbRec);
			if (pbRec == NULL)
				break;
		}

		dword cbRecLeft = offRecCurrent + cbRec - offBits;
		int cyRecWholeScans = cbRecLeft / m_cx;

		// Blt as much as we can

		int cyBlt = _min(cyLeft, cyRecWholeScans);
		LeftToRightBltThunk(&pbRec[offBits - offRecCurrent], m_cx, pbDst, cbRowDst, cxBlt, cyBlt);
		cyLeft -= cyBlt;
		if (cyLeft == 0)
			break;
		offBits += (long)cyBlt * m_cx;
		pbDst += (long)cyBlt * cbRowDst;

		// Does the next scan intersect a record boundary?

		if (offRecCurrent + cbRec - offBits < (word)cxBlt) {
			// Blt left half, unlock current record, lock next record, blt right half

			int cbLeftHalf = (int)(offRecCurrent + cbRec - offBits);
			memcpy(pbDst, &pbRec[offBits - offRecCurrent], cbLeftHalf);
			m_pfil->prnfo->ppdbReader->UnmapRecord(nRecCurrent, dwCookie);
			offRecCurrent += cbRec;
			nRecCurrent++;
			pbRec = m_pfil->prnfo->ppdbReader->MapRecord(nRecCurrent, &dwCookie, &cbRec);
			if (pbRec == NULL)
				return;
			memcpy(pbDst + cbLeftHalf, pbRec, cxBlt - cbLeftHalf);
			pbDst += cbRowDst;
			offBits += m_cx;
			cyLeft--;
			continue;
		}

		// Next scan doesn't intersect a record boundary

		memcpy(pbDst, &pbRec[offBits - offRecCurrent], cxBlt);
		m_pfil->prnfo->ppdbReader->UnmapRecord(nRecCurrent, dwCookie);
		pbRec = NULL;
		offRecCurrent += cbRec;
		offBits += m_cx;
		nRecCurrent++;
		pbDst += cbRowDst;
		cyLeft--;
	}

	if (pbRec != NULL)
		m_pfil->prnfo->ppdbReader->UnmapRecord(nRecCurrent, dwCookie);
}
#endif

} // namespace wi
