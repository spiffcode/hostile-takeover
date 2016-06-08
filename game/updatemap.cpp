#include "ht.h"

namespace wi {

UpdateMap::UpdateMap()
{
	m_afInvalid = NULL;
	m_afInvalidDamage = NULL;
	m_ctx = 0;
	m_cty = 0;
	m_fInvalid = false;
	m_fInvalidDamage = false;
	m_fMergeDamage = false;
	m_txOrigin = 0;
	m_tyOrigin = 0;
	m_xOriginView = 0;
	m_yOriginView = 0;
}

UpdateMap::~UpdateMap()
{
	delete[] m_afInvalid;
	delete[] m_afInvalidDamage;
}

bool UpdateMap::Init(Size *psiz)
{
	m_rcDib.Set(0, 0, psiz->cx, psiz->cy);
	m_ctx = (psiz->cx + (gcxTile - 1)) / gcxTile + 1;
	m_cty = (psiz->cy + (gcyTile - 1)) / gcyTile + 1;
	int cb = m_ctx * m_cty;
	delete[] m_afInvalid;
	delete[] m_afInvalidDamage;
	m_afInvalid = new bool[cb];
	if (m_afInvalid == NULL)
		return false;
	memset(m_afInvalid, 0, cb);
	m_afInvalidDamage = new bool[cb];
	if (m_afInvalidDamage == NULL)
		return false;
	SetMapOffset(0, 0, false);
	return true;
}

void UpdateMap::Reset()
{
	SetMapOffset(0, 0, true);
	m_txOrigin = 0;
	m_tyOrigin = 0;
}

bool UpdateMap::IsInvalid()
{
	return m_fInvalid;
}

bool UpdateMap::EnumUpdateRects(bool fFirst, Rect *prcBounds, Rect *prc)
{
	static Rect s_rcEnumLast;
	static bool s_fDone;

	// Enum row rects two at a time. If the 2nd one can be coalesced
	// vertically with the last one, do it, otherwise return them as is

	if (fFirst) {
		if (!EnumRowRects(fFirst, prcBounds, &s_rcEnumLast))
			return false;
		fFirst = false;
		s_fDone = false;
	} else {
		// Not first; done from last time?

		if (s_fDone)
			return false;
	}

	Rect rcT;
	while (true) {
		if (!EnumRowRects(fFirst, prcBounds, &rcT)) {
			s_fDone = true;
			*prc = s_rcEnumLast;
			return true;
		}

		// If this is a clean union, then do it to join rows

		if (rcT.left == s_rcEnumLast.left && rcT.right == s_rcEnumLast.right && rcT.top == s_rcEnumLast.bottom) {
			s_rcEnumLast.bottom = rcT.bottom;
			continue;
		}

		// Not a clean union - return the last one and save the current one

		*prc = s_rcEnumLast;
		s_rcEnumLast = rcT;
		return true;
	}
}

bool UpdateMap::EnumRowRects(bool fFirst, Rect *prcBounds, Rect *prc) {
	// If first, reset

	static int s_tyEnumNext = 0;
	static int s_txEnumNext = 0;
	static int s_txLeftBounds = 0;
	static int s_tyTopBounds = 0;
	static int s_txRightBounds = 0;
	static int s_tyBottomBounds = 0;
	static int s_cbInvalidReturn = 0;

	if (fFirst) {
		if (prcBounds == NULL) {
			s_txLeftBounds = 0;
			s_tyTopBounds = 0;
			s_txRightBounds = m_ctx;
			s_tyBottomBounds = m_cty;
			s_cbInvalidReturn = 0;
		} else {
			s_txLeftBounds = TcFromPc(prcBounds->left + m_xMapOffset);
			if (s_txLeftBounds < 0)
				s_txLeftBounds = 0;
			s_tyTopBounds = TcFromPc(prcBounds->top + m_yMapOffset);
			if (s_tyTopBounds < 0)
				s_tyTopBounds = 0;
			s_txRightBounds = TcFromPc(prcBounds->right + m_xMapOffset + gcxTile - 1);
			if (s_txRightBounds > m_ctx)
				s_txRightBounds = m_ctx;
			s_tyBottomBounds = TcFromPc(prcBounds->bottom + m_yMapOffset + gcyTile - 1);
			if (s_tyBottomBounds > m_cty)
				s_tyBottomBounds = m_cty;
			s_cbInvalidReturn = m_ctx - (s_txRightBounds - s_txLeftBounds);
		}
		s_txEnumNext = s_txLeftBounds;
		s_tyEnumNext = s_tyTopBounds;
	}
	int txT = s_txEnumNext;
	int tyT = s_tyEnumNext;
	bool *pfInvalid = &m_afInvalid[tyT * m_ctx + txT];

	// Try to coalesce into larger rectangles

	for (; tyT < s_tyBottomBounds; tyT++) {
		int txStart = -1;
		int txEnd = -1;
		for (; txT < s_txRightBounds; txT++) {
			if (*pfInvalid) {
				// Invalid tile. Start or extend an invalid rect?

				if (txStart == -1)
					txStart = txT;
				txEnd = txT + 1;
			} else {
				// Valid tile. Return accumulated invalid rect if it exists

				if (txStart != -1) {
					s_txEnumNext = txT + 1;
					s_tyEnumNext = tyT;
					goto ReturnRect;
				}
			}
			pfInvalid++;
		}
		s_txEnumNext = s_txLeftBounds;
		s_tyEnumNext = tyT + 1;
		pfInvalid += s_cbInvalidReturn;
		txT = s_txLeftBounds;

		// End of the row. Return accumulated rect if it exists

		if (txStart != -1) {
ReturnRect:
			prc->left = txStart * gcxTile;
			prc->top = tyT * gcyTile;
			prc->right = prc->left + (txEnd - txStart) * gcxTile;
			prc->bottom = prc->top + gcyTile;
			prc->Offset(-m_xMapOffset, -m_yMapOffset);
			if (prcBounds == NULL) {
				prc->Intersect(prc, &m_rcDib);
			} else {
				prc->Intersect(prc, prcBounds);
			}
			return true;
		}
	}

	// All done

	return false;
}

// OPT: Assembly candidate

void UpdateMap::InvalidateTileRect(TRectSmall *ptrc)
{
	Assert(ptrc->txLeft >= 0 && ptrc->tyTop >= 0);
	Assert(ptrc->txRight <= m_ctx && ptrc->tyBottom <= m_cty);

	bool *pfInvalid = &m_afInvalid[ptrc->tyTop * m_ctx + ptrc->txLeft];
	int cbNext = m_ctx - (ptrc->txRight - ptrc->txLeft);
	for (int tyT = ptrc->tyTop; tyT < ptrc->tyBottom; tyT++) {
		for (int txT = ptrc->txLeft; txT < ptrc->txRight; txT++) {
			if (*pfInvalid == false) {
				m_fInvalid = true;
				(*pfInvalid) = true;
			}
			pfInvalid++;
		}
		pfInvalid += cbNext;
	}
}

bool UpdateMap::IsTileRectInvalid(TRectSmall *ptrc)
{
	Assert(ptrc->txLeft >= 0 && ptrc->tyTop >= 0);
	Assert(ptrc->txRight <= m_ctx && ptrc->tyBottom <= m_cty);

	bool *pfInvalid = &m_afInvalid[ptrc->tyTop * m_ctx + ptrc->txLeft];
	int cbNext = m_ctx - (ptrc->txRight - ptrc->txLeft);
	for (int tyT = ptrc->tyTop; tyT < ptrc->tyBottom; tyT++) {
		for (int txT = ptrc->txLeft; txT < ptrc->txRight; txT++) {
			if (*pfInvalid)
				return true;
			pfInvalid++;
		}
		pfInvalid += cbNext;
	}
	return false;
}

bool UpdateMap::IsTileRectInvalidAndTrackDamage(TRectSmall *ptrc, bool *pfNewInvalid)
{
	Assert(ptrc->txLeft >= 0 && ptrc->tyTop >= 0);
	Assert(ptrc->txRight <= m_ctx && ptrc->tyBottom <= m_cty);

	bool *pfInvalid = &m_afInvalid[ptrc->tyTop * m_ctx + ptrc->txLeft];
	int cbNext = m_ctx - (ptrc->txRight - ptrc->txLeft);

	// See if any part of trc is touching invalid area

	bool fAnyInvalid = false;
	bool *pfInvalidT = pfInvalid;
	int tyT;
	for (tyT = ptrc->tyTop; tyT < ptrc->tyBottom; tyT++) {
		for (int txT = ptrc->txLeft; txT < ptrc->txRight; txT++) {
			if (*pfInvalidT) {
				fAnyInvalid = true;
				break;
			}
			pfInvalidT++;
		}
		if (fAnyInvalid)
			break;
		pfInvalidT += cbNext;
	}

	// If not, don't invalidate the whole thing

	if (!fAnyInvalid)
		return false;

	// If merging damage, don't add to the damage map

	if (m_fMergeDamage) {
		// Some part is touching invalid area, invalidate the whole thing

		bool fNewInvalid = false;
		pfInvalidT = pfInvalid;
		for (tyT = ptrc->tyTop; tyT < ptrc->tyBottom; tyT++) {
			for (int txT = ptrc->txLeft; txT < ptrc->txRight; txT++) {
				if (*pfInvalidT == false) {
					(*pfInvalidT) = true;
					fNewInvalid = true;
				}
				pfInvalidT++;
			}
			pfInvalidT += cbNext;
		}

		// Accumulate whether there was "new" invalidation

		*pfNewInvalid |= fNewInvalid;
	} else {
		// Something is invalid, mark the damage map for the portions that will be
		// damaged


		bool *pfInvalidDamage = &m_afInvalidDamage[pfInvalid - m_afInvalid];
		for (tyT = ptrc->tyTop; tyT < ptrc->tyBottom; tyT++) {
			for (int txT = ptrc->txLeft; txT < ptrc->txRight; txT++) {
				*pfInvalidDamage = true;
				m_fInvalidDamage = true;
				pfInvalidDamage++;
			}
			pfInvalidDamage += cbNext;
		}
	}

	// Yes this ptrc was touching invalid area

	return true;
}

bool UpdateMap::IsMapTileRectInvalidAndTrackDamage(TRectSmall *ptrc, bool *pfNewInvalid)
{
	// Convert from map tile coords to updatemap coords

	TRectSmall trc;
	trc.txLeft = ptrc->txLeft - m_txOrigin;
	trc.tyTop = ptrc->tyTop - m_tyOrigin;
	trc.txRight = ptrc->txRight - m_txOrigin;
	trc.tyBottom = ptrc->tyBottom - m_tyOrigin;

	// Clip to the update map

	if (trc.txLeft < 0)
		trc.txLeft = 0;
	if (trc.tyTop < 0)
		trc.tyTop = 0;
	if (trc.txRight > m_ctx)
		trc.txRight = m_ctx;
	if (trc.tyBottom > m_cty)
		trc.tyBottom = m_cty;

	return IsTileRectInvalidAndTrackDamage(&trc, pfNewInvalid);
}

void UpdateMap::Validate()
{
	memset(m_afInvalid, 0, m_ctx * m_cty);
	m_fInvalid = false;
}

void UpdateMap::CalcTileRect(Rect *prc, TRectSmall *ptrc)
{
#if WIN
	Assert(m_xOriginView == 0 || (m_xOriginView % gcxTile) == m_xMapOffset);
	Assert(m_yOriginView == 0 || (m_yOriginView % gcyTile) == m_yMapOffset);
#endif

	int x = prc->left + m_xMapOffset;
	if (x >= kpcMax)
		x = kpcMax - 1;
	if (x < 0)
		x = 0;
	ptrc->txLeft = (char)TcFromPc(x);
	int y = prc->top + m_yMapOffset;
	if (y >= kpcMax)
		y = kpcMax - 1;
	if (y < 0)
		y = 0;
	ptrc->tyTop = (char)TcFromPc(y);

	// Preclip right/bottom to the coordinate range supported
	// by TcFromPc.

	int xT = x + (prc->right - prc->left) + (gcxTile - 1);
	if (xT >= kpcMax)
		xT = kpcMax - 1;
	int yT = y + (prc->bottom - prc->top) + (gcyTile - 1);
	if (yT >= kpcMax)
		yT = kpcMax - 1;

	ptrc->txRight = (char)TcFromPc(xT);
	if (ptrc->txRight > m_ctx)
		ptrc->txRight = m_ctx;

	ptrc->tyBottom = (char)TcFromPc(yT);
	if (ptrc->tyBottom > m_cty)
		ptrc->tyBottom = m_cty;
}

void UpdateMap::InvalidateRect(Rect *prc)
{
	Rect rcT;
	if (prc == NULL) {
		rcT.Set(0, 0, m_ctx * gcxTile, m_cty * gcyTile);
		prc = &rcT;
	}

	TRectSmall trc;
	CalcTileRect(prc, &trc);
	InvalidateTileRect(&trc);
}

void UpdateMap::InvalidateMapTileRect(TRectSmall *ptrc)
{
	// Convert from map tile coords to updatemap coords

	TRectSmall trc;
	trc.txLeft = ptrc->txLeft - m_txOrigin;
	trc.tyTop = ptrc->tyTop - m_tyOrigin;
	trc.txRight = ptrc->txRight - m_txOrigin;
	trc.tyBottom = ptrc->tyBottom - m_tyOrigin;

	// Clip to the update map

	if (trc.txLeft < 0) {
		if (trc.txRight <= 0)
			return;
		trc.txLeft = 0;
	}
	if (trc.tyTop < 0) {
		if (trc.tyBottom <= 0)
			return;
		trc.tyTop = 0;
	}
	if (trc.txRight > m_ctx) {
		if (trc.txLeft >= m_ctx)
			return;
		trc.txRight = m_ctx;
	}
	if (trc.tyBottom > m_cty) {
		if (trc.tyTop >= m_cty)
			return;
		trc.tyBottom = m_cty;
	}

	// Invalidate update map

	InvalidateTileRect(&trc);
}

void UpdateMap::InvalidateTile(TCoord txMap, TCoord tyMap)
{
	// Convert from map tile coords to updatemap coords

	int tx = txMap - m_txOrigin;
	int ty = tyMap - m_tyOrigin;
	if (tx < 0 || tx >= m_ctx)
		return;
	if (ty < 0 || ty >= m_cty)
		return;

	bool *pfInvalid = &m_afInvalid[ty * m_ctx + tx];
	if (*pfInvalid)
		return;

	// Invalidate

	*pfInvalid = true;
	m_fInvalid = true;
}

void UpdateMap::StartMergeDamagedInvalid()
{
	byte *pbDst = (byte *)m_afInvalid;
	byte *pbSrc = (byte *)m_afInvalidDamage;
	int cb = m_ctx * m_cty;
	while (cb-- != 0)
		*pbDst++ |= *pbSrc++;
	m_fMergeDamage = true;
}

void UpdateMap::EndMergeDamagedInvalid()
{
	memset(m_afInvalidDamage, 0, m_ctx * m_cty);
	m_fInvalidDamage = false;
	m_fMergeDamage = false;
}

void UpdateMap::SetViewOrigin(int xOrigin, int yOrigin)
{
	m_xOriginView = xOrigin;
	m_yOriginView = yOrigin;
}

void UpdateMap::SetMapOffset(int xMapOffset, int yMapOffset, bool fInvalidate)
{
	m_xMapOffset = xMapOffset;
	m_yMapOffset = yMapOffset;
	int cxDib = m_rcDib.Width();
	int cyDib = m_rcDib.Height();
	if (gsim.GetLevel() != NULL) {
		Size sizTMap;
		gsim.GetLevel()->GetTileMap()->GetMapSize(&sizTMap);
		if (sizTMap.cx < cxDib)
			cxDib = sizTMap.cx;
		if (sizTMap.cy < cyDib)
			cyDib = sizTMap.cy;
	}
	m_mnfo.cxLeftTile = PcFracFromUpc(gcxTile - m_xMapOffset);
	if (cyDib < gcyTile - m_yMapOffset) {
		m_mnfo.cyTopTile = cyDib;
	} else {
		m_mnfo.cyTopTile = PcFracFromUpc(gcyTile - m_yMapOffset);
	}
	m_mnfo.cxRightTile = PcFracFromUpc(cxDib - m_mnfo.cxLeftTile);
	m_mnfo.cyBottomTile = PcFracFromUpc(cyDib - m_mnfo.cyTopTile);
	m_mnfo.ctxInside = TcFromPc(cxDib - m_mnfo.cxLeftTile - m_mnfo.cxRightTile);
	m_mnfo.ctyInside = TcFromPc(cyDib - m_mnfo.cyTopTile - m_mnfo.cyBottomTile);
	if (fInvalidate)
		InvalidateRect();
}

MapInfo *UpdateMap::GetMapInfo()
{
	return &m_mnfo;
}

bool UpdateMap::Scroll(int dx, int dy)
{
	// If no scroll, nothing to do

	if (dx == 0 && dy == 0)
		return false;

	// Calc new sub-tile map offsets

	int xMapOffsetNew, yMapOffsetNew;
	if (dx <= 0) {
		xMapOffsetNew = PcFracFromUpc(m_xMapOffset - dx);
	} else {
		xMapOffsetNew = PcFracFromUpc(gcxTile - PcFracFromUpc(gcxTile - m_xMapOffset + dx));
	}
	if (dy <= 0) {
		yMapOffsetNew = PcFracFromUpc(m_yMapOffset - dy);
	} else {
		yMapOffsetNew = PcFracFromUpc(gcyTile - PcFracFromUpc(gcyTile - m_yMapOffset + dy));
	}

	// Figure out the number of whole tiles to scroll

	int dtx;
	if (dx <= 0) {
		dtx = -TcFromPc(m_xMapOffset - dx);
	} else {
		dtx = TcFromPc((gcxTile - 1) - m_xMapOffset + dx);
	}

	int dty;
	if (dy <= 0) {
		dty = -TcFromPc(m_yMapOffset - dy);
	} else {
		dty = TcFromPc((gcyTile - 1) - m_yMapOffset + dy);
	}

	// Adjust tile origin

	m_txOrigin -= dtx;
	m_tyOrigin -= dty;

    // Scroll the map if needed. If we don't actually need to scroll the map,
    // we still need to cause invalidation along the edges

	if (dtx != 0 || dty != 0) {
		// Need to scroll the map. Figure out the scrolling rect

		Rect rcSrc;
		rcSrc.Set(0, 0, m_ctx, m_cty);
		rcSrc.Offset(-dtx, -dty);

		// Clip

		if (rcSrc.left < 0)
			rcSrc.left = 0;
		if (rcSrc.top < 0)
			rcSrc.top = 0;
		if (rcSrc.right > m_ctx)
			rcSrc.right = m_ctx;
		if (rcSrc.bottom > m_cty)
			rcSrc.bottom = m_cty;

		// Figure out dst

		int txDst = dtx;
		if (txDst < 0)
			txDst = 0;
		int tyDst = dty;
		if (tyDst < 0)
			tyDst = 0;

		// Don't need to worry about copy direction since we're using a temp buffer

		int cb = m_ctx * m_cty;
		bool *pfT = new bool[cb];
		if (pfT != NULL) {
			// Scroll the invalid map

			memcpy(pfT, m_afInvalid, cb);
			memset(m_afInvalid, 1, cb);
			if (txDst < m_ctx && tyDst < m_cty) {
				bool *pfSrc = &pfT[rcSrc.top * m_ctx + rcSrc.left];
				bool *pfDst = &m_afInvalid[tyDst * m_ctx + txDst];
				int cbNext = m_ctx - rcSrc.Width();
				int ctx = rcSrc.Width();
				int cty = rcSrc.Height();
				for (int ty = 0; ty < cty; ty++) {
					for (int tx = 0; tx < ctx; tx++) {
						*pfDst++ = *pfSrc++;
					}
					pfSrc += cbNext;
					pfDst += cbNext;
				}
			}

			// Scroll the damage map

			memcpy(pfT, m_afInvalidDamage, cb);
			memset(m_afInvalidDamage, 0, cb);
			if (txDst < m_ctx && tyDst < m_cty) {
				bool *pfSrc = &pfT[rcSrc.top * m_ctx + rcSrc.left];
				bool *pfDst = &m_afInvalidDamage[tyDst * m_ctx + txDst];
				int cbNext = m_ctx - rcSrc.Width();
				int ctx = rcSrc.Width();
				int cty = rcSrc.Height();
				for (int ty = 0; ty < cty; ty++) {
					for (int tx = 0; tx < ctx; tx++) {
						*pfDst++ = *pfSrc++;
					}
					pfSrc += cbNext;
					pfDst += cbNext;
				}
			}

			// Now scrolled. Cleanup

			delete[] pfT;
		} else {
			// Couldn't alloc temp buffer, so invalidate everything

			memset(m_afInvalid, 1, cb);
			memset(m_afInvalidDamage, 0, cb);
		}
	}

	// Update the new map offsets before edge invalidation

	SetMapOffset(xMapOffsetNew, yMapOffsetNew, false);

	// Invalidate edges

	Rect rcNew;
	if (dx != 0) {
		rcNew = m_rcDib;
		rcNew.Offset(dx, 0);
		rcNew.Intersect(&rcNew, &m_rcDib);
		if (rcNew.Subtract(&m_rcDib, &rcNew))
			InvalidateRect(&rcNew);
	}
	if (dy != 0) {
		rcNew = m_rcDib;
		rcNew.Offset(0, dy);
		rcNew.Intersect(&rcNew, &m_rcDib);
		if (rcNew.Subtract(&m_rcDib, &rcNew))
			InvalidateRect(&rcNew);
	}

//	Trace("xMapOffset: %d, yMapOffset: %d", m_xMapOffset, m_yMapOffset);

	// Return true to cause bits to be scrolled

	return true;
}

bool *UpdateMap::GetInvalidMap()
{
	return m_afInvalid;
}

void UpdateMap::GetMapSize(Size *psiz)
{
	psiz->cx = m_ctx;
	psiz->cy = m_cty;
}

} // namespace wi
