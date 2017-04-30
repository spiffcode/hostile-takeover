#include "ht.h"

namespace wi {

FogMap::FogMap()
{
	m_pbMap = NULL;
	m_panidWalls = NULL;
	memset(m_aptbm, 0, sizeof(m_aptbm));
	memset(m_aptbmGalax, 0, sizeof(m_aptbmGalax));

	// Each tile holds 4 bits. 0 is right most, 1 is top most, 2 is bottom most,
	// 3 is left most
	//
	// Init the pattern results. The only reason a table exists
	// (rather than using mask exclusively) is to disallow
	// unwanted results. 1001 and 0110 are the only two illegal
	// results with the current scheme.

	for (byte bSrc = 0; bSrc < 16; bSrc++) {
		for (byte bDst = 0; bDst < 16; bDst++) {
			byte bRes = ~(~bDst & ~bSrc) & 0xf;
			if (bRes == 9 || bRes == 6)
				bRes = 0;
			m_mpSrcDstResult[(((~bSrc) & 0xf) << 4) | bDst] = bRes;
		}
	}
}

FogMap::~FogMap()
{
	delete m_panidWalls;
	delete[] m_pbMap;
	int n;
	for (n = 0; n < 16; n++)
		delete m_aptbm[n];

	for (n = 0; n < 9; n++)
		delete m_aptbmGalax[n];
}

bool FogMap::Init(Size *psizTile, Size *psizMap)
{
	m_cxTile = psizTile->cx;
	m_cyTile = psizTile->cy;
	m_ctxMap = psizMap->cx / m_cxTile;
	m_ctyMap = psizMap->cy / m_cyTile;

	// Alloc the map and make it all "fogged" and Galaxite-free

	int cb = m_ctxMap * m_ctyMap;
	m_pbMap = new byte[cb];
	Assert(m_pbMap != NULL, "out of memory!");
	if (m_pbMap == NULL)
		return false;
	memset(m_pbMap, kbOpaque, cb);

	// Load the edges

	m_aptbm[15] = NULL;
	m_aptbm[14] = CreateTBitmap("fog0001.png");
	m_aptbm[13] = CreateTBitmap("fog0010.png");
	m_aptbm[12] = CreateTBitmap("fog0011.png");
	m_aptbm[11] = CreateTBitmap("fog0100.png");
	m_aptbm[10] = CreateTBitmap("fog0101.png");
	m_aptbm[9] = NULL;
	m_aptbm[8] = CreateTBitmap("fog0111.png");
	m_aptbm[7] = CreateTBitmap("fog1000.png");
	m_aptbm[6] = NULL;
	m_aptbm[5] = CreateTBitmap("fog1010.png");
	m_aptbm[4] = CreateTBitmap("fog1011.png");
	m_aptbm[3] = CreateTBitmap("fog1100.png");
	m_aptbm[2] = CreateTBitmap("fog1101.png");
	m_aptbm[1] = CreateTBitmap("fog1110.png");
	m_aptbm[0] = CreateTBitmap("fog1111.png");

	int c = 0;
	int n;
	for (n = 0; n < 16; n++) {
		if (m_aptbm[n] == NULL)
			c++;
	}
	if (c != 3)
		return false;
	
	m_aptbmGalax[0] = CreateTBitmap("galax1a.png");
	m_aptbmGalax[1] = CreateTBitmap("galax1b.png");
	m_aptbmGalax[2] = CreateTBitmap("galax1c.png");
	m_aptbmGalax[3] = CreateTBitmap("galax2a.png");
	m_aptbmGalax[4] = CreateTBitmap("galax2b.png");
	m_aptbmGalax[5] = CreateTBitmap("galax2c.png");
	m_aptbmGalax[6] = CreateTBitmap("galax3a.png");
	m_aptbmGalax[7] = CreateTBitmap("galax3b.png");
	m_aptbmGalax[8] = CreateTBitmap("galax3c.png");


	for (n = 0; n < 9; n++) {
		if (m_aptbmGalax[n] == NULL) {
			Assert("Failed to load one of the Galaxite bitmaps");
			return false;
		}
	}

	// Load the wall bitmaps

	m_panidWalls = LoadAnimationData("wall.anir");
	if (m_panidWalls == NULL) {
		Assert("Failed to load wall.anir");
		return false;
	}

	return true;
}

#define knVerFogMapState 4
bool FogMap::LoadState(Stream *pstm)
{
	// Note at this point the fogmap has already been "initialized" from
	// level constants so we just need to load in the contents of the map
	// which includes both fog and galaxite state

	byte nVer = pstm->ReadByte();
	if (nVer != knVerFogMapState)
		return false;
	pstm->ReadBytesRLE(m_pbMap, m_ctxMap * m_ctyMap);
	return pstm->IsSuccess();
}

bool FogMap::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerFogMapState);
	pstm->WriteBytesRLE(m_pbMap, m_ctxMap * m_ctyMap);
	return pstm->IsSuccess();
}

bool FogMap::IsCovered(TRect *ptrc)
{
    TRect trcMap;
    trcMap.left = 0;
    trcMap.top = 0;
    trcMap.right = m_ctxMap;
    trcMap.bottom = m_ctyMap;
    TRect trcT = *ptrc;
    if (!trcT.Intersect(&trcT, &trcMap)) {
        return false;
    }

    for (int ty = trcT.top; ty < trcT.bottom; ty++) {
        for (int tx = trcT.left; tx < trcT.right; tx++) {
            byte bFog = m_pbMap[ty * m_ctxMap + tx] & kbfFogMask;
            if (bFog != 0) {
                return false;
            }
        }
    }
    return true;
}
        
void FogMap::RevealAll(UpdateMap *pupd)
{
	byte *pbT = m_pbMap;
	byte *pbEnd = m_pbMap + (m_ctxMap * m_ctyMap);
	while (pbT < pbEnd) {
//		*pbT++ = (*pbT & ~kbfFogMask) | kbEmpty;
		// BUGBUG: if we post-increment pbT as above the CodeWarrior ARM compiler
		// screws it up such that the RHS pbT is one byte ahead of the LHS pbT,
		// the end result being that all the Galaxite in the map gets shifted one
		// tile to the left!

		*pbT = (*pbT & ~kbfFogMask) | kbEmpty;
		pbT++;
	}

	// Invalidate the whole update map

	pupd->InvalidateRect();

	// Redraw the whole minimap

	gpmm->Redraw();
}

void FogMap::Reveal(TRect *ptrc, UpdateMap *pupd, WCoord wxView, WCoord wyView) 
{
	int ctxT = ptrc->Width();

	// Top row and corners

	byte *pbDst = &m_pbMap[(ptrc->top * m_ctxMap) + ptrc->left];
	int ctx = ctxT - 2;
	*pbDst = *pbDst | (15 - 10);
	pbDst++;
	while (ctx-- > 0) {
//		*pbDst++ = *pbDst | (15 - 2);
		// BUGBUG: if we post-increment pbDst as above the CodeWarrior ARM compiler
		// screws it up such that the RHS pbDst is one byte ahead of the LHS pbDst,
		// the end result being that defog pattern isn't applied properly.
		*pbDst = *pbDst | (15 - 2);
		pbDst++;
	}
	*pbDst = *pbDst | (15 - 3);

	// Bottom row and corners

	pbDst = &m_pbMap[((ptrc->bottom - 1) * m_ctxMap) + ptrc->left];
	ctx = ctxT - 2;
	*pbDst = *pbDst | (15 - 12);
	pbDst++;
	while (ctx-- > 0) {
//		*pbDst++ = *pbDst | (15 - 4);
		// BUGBUG: if we post-increment pbDst as above the CodeWarrior ARM compiler
		// screws it up such that the RHS pbDst is one byte ahead of the LHS pbDst,
		// the end result being that defog pattern isn't applied properly.
		*pbDst = *pbDst | (15 - 4);
		pbDst++;
	}
	*pbDst = *pbDst | (15 - 5);

	// Left column

	pbDst = &m_pbMap[((ptrc->top + 1) * m_ctxMap) + ptrc->left];
	ctx = ptrc->Height() - 2;
	while (ctx-- > 0) {
//		*pbDst++ = *pbDst | (15 - 8);
		// BUGBUG: if we post-increment pbDst as above the CodeWarrior ARM compiler
		// screws it up such that the RHS pbDst is one byte ahead of the LHS pbDst,
		// the end result being that defog pattern isn't applied properly.
		*pbDst = *pbDst | (15 - 8);
		pbDst += m_ctxMap;
	}

	// Right column

	pbDst = &m_pbMap[((ptrc->top + 1) * m_ctxMap) + ptrc->right - 1];
	ctx = ptrc->Height() - 2;
	while (ctx-- > 0) {
//		*pbDst++ = *pbDst | (15 - 1);
		// BUGBUG: if we post-increment pbDst as above the CodeWarrior ARM compiler
		// screws it up such that the RHS pbDst is one byte ahead of the LHS pbDst,
		// the end result being that defog pattern isn't applied properly.
		*pbDst = *pbDst | (15 - 1);
		pbDst += m_ctxMap;
	}

	// Interior

	TRect trcT;
	trcT = *ptrc;
	trcT.Inflate(-1, -1);
	ctxT = trcT.Width();

	pbDst = &m_pbMap[(trcT.top * m_ctxMap) + trcT.left];
	int cty = trcT.Height();
	while (cty-- > 0) {
		ctx = ctxT;
		while (ctx-- > 0) {
			*pbDst = *pbDst | kbEmpty;
			pbDst++;
		}
		pbDst += m_ctxMap - ctxT;
	}

	// Invalidate

	gpmm->RedrawTRect(ptrc);
	Rect rcT;
	rcT.FromTileRect(ptrc);
	int xView = PcFromWc(wxView) & 0xfffe;
	int yView = PcFromWc(wyView) & 0xfffe;
	rcT.Offset(-xView, -yView);
	pupd->InvalidateRect(&rcT);
}

void FogMap::Reveal(TCoord txMap, TCoord tyMap, RevealPattern *prvlp, UpdateMap *pupd, WCoord wxView, WCoord wyView)
{
	Assert(txMap >= 0 && txMap < m_ctxMap && tyMap >= 0 && tyMap < m_ctyMap);

	// RevealPattern is centered around txMap and tyMap

	int ctx = prvlp->ctx;
	int cty = prvlp->cty;
	int tx = txMap - ctx / 2;
	int ty = tyMap - cty / 2;

	// Clip to map edges

	int txPattern = 0;
	int tyPattern = 0;

	if (tx < 0) {
		ctx = tx + ctx;
		txPattern = -tx;
		tx = 0;
	}
	if (ty < 0) {
		cty = ty + cty;
		tyPattern = -ty;
		ty = 0;
	}
	if (tx + ctx > m_ctxMap)
		ctx = m_ctxMap - tx;
	if (ty + cty > m_ctyMap)
		cty = m_ctyMap - ty;

	Assert(ctx > 0 && ctx <= prvlp->ctx && cty > 0 && cty <= prvlp->cty);

	// Reveal

	int cbNextSrc = prvlp->ctx - ctx;
	int cbNextDst = m_ctxMap - ctx;
	byte *pbDst = &m_pbMap[ty * m_ctxMap + tx];
	byte *pbSrc = &prvlp->ab[tyPattern * prvlp->ctx + txPattern];
	bool fChanged = false;
	for (int tyT = ty; tyT < ty + cty; tyT++) {
		for (int txT = tx; txT < tx + ctx; txT++) {
			byte bDst = *pbDst;
			byte bDstNew = (*pbDst & ~kbfFogMask) | m_mpSrcDstResult[((*pbSrc) << 4) | (*pbDst & kbfFogMask)];
			if (bDst != bDstNew) {
				*pbDst = bDstNew;
				pupd->InvalidateTile(txT, tyT);
				gpmm->RedrawTile(txT, tyT);
			}
			pbDst++;
			pbSrc++;
		}
		pbSrc += cbNextSrc;
		pbDst += cbNextDst;
	}
}

void FogMap::Draw(DibBitmap *pbm, int xMap, int yMap, UpdateMap *pupd)
{
	Size siz;
	pbm->GetSize(&siz);
	Rect rcDib;
	rcDib.left = 0;
	rcDib.top = 0;
	rcDib.right = siz.cx;
	rcDib.bottom = siz.cy;

	int tx = xMap / gcxTile;
	int ty = yMap / gcyTile;
	int ctx = (siz.cx + (gcxTile - 1)) / gcxTile + 1;
	if (tx + ctx > m_ctxMap)
		ctx = m_ctxMap - tx;
	int cty = (siz.cy + (gcyTile - 1)) / gcyTile + 1;
	if (ty + cty > m_ctyMap)
		cty = m_ctyMap - ty;
	Color clrBlack = GetColor(kiclrBlack);

	bool *pfInvalid = pupd->GetInvalidMap();
	Size sizMap;
	pupd->GetMapSize(&sizMap);
	int cfInvalidNextScan = sizMap.cx - ctx;
	Assert(sizMap.cx >= ctx && sizMap.cy >= cty);

	byte *pbMapT = &m_pbMap[ty * m_ctxMap + tx];
	int cbNextScan = m_ctxMap - ctx;
	int xTile = tx * gcxTile - xMap;
	int yTile = ty * gcyTile - yMap;
	int xTileStart = xTile;
	for (int tyT = ty; tyT < ty + cty; tyT++) {
		int cEmpty = 0;
		int xStart = 0;
		for (int txT = tx; txT < tx + ctx; txT++) {
			byte bFog = *pbMapT++ & kbfFogMask;
			if (*pfInvalid++ == false)
				bFog = kbEmpty;

			// Opaque?

			if (bFog == kbOpaque) {
				if (cEmpty == 0)
					xStart = xTile;
				cEmpty++;
				xTile += gcxTile;
				continue;
			} 

			// Not opaque; fill block if there is one

			if (cEmpty != 0) {
				pbm->Fill(xStart, yTile, cEmpty * gcxTile, gcyTile, clrBlack);
				cEmpty = 0;
#ifdef PIL
				if (gfOS5Pa1Device)
					HostSoundServiceProc();
#endif
			}

			// Not opaque; draw unique tile

			TBitmap *ptbm = m_aptbm[bFog];
			if (ptbm == NULL) {
				xTile += gcxTile;
				continue;
			}

			ptbm->BltTo(pbm, xTile, yTile);
			xTile += gcxTile;
		}
		if (cEmpty != 0) {
			pbm->Fill(xStart, yTile, cEmpty * gcxTile, gcyTile, clrBlack);
			cEmpty = 0;

#ifdef PIL
			if (gfOS5Pa1Device)
				HostSoundServiceProc();
#endif
		}

		pbMapT += cbNextScan;
		pfInvalid += cfInvalidNextScan;
		yTile += gcxTile;
		xTile = xTileStart;
	}
}

byte s_abGxTranslate[] = { 0, 2, 1, 1, 0, 0, 5, 4, 4, 3, 3, 8, 7, 7, 6, 6 };
byte s_abGxInc[] = { 0, 2, 3, 4, 5, 5, 7, 8, 9, 10, 10, 12, 13, 14, 15, 15 };
byte s_abGxDec[] = { 0, 0, 1, 2, 3, 4, 0, 6, 7, 8, 9, 0, 11, 12, 13, 14 };

void FogMap::DrawGalaxite(DibBitmap *pbm, int xMap, int yMap, UpdateMap *pupd, byte *pbTrMap)
{
	Size siz;
	pbm->GetSize(&siz);
	Rect rcDib;
	rcDib.left = 0;
	rcDib.top = 0;
	rcDib.right = siz.cx;
	rcDib.bottom = siz.cy;

	int tx = xMap / gcxTile;
	int ty = yMap / gcyTile;
	int ctx = (siz.cx + (gcxTile - 1)) / gcxTile + 1;
	if (tx + ctx > m_ctxMap)
		ctx = m_ctxMap - tx;
	int cty = (siz.cy + (gcyTile - 1)) / gcyTile + 1;
	if (ty + cty > m_ctyMap)
		cty = m_ctyMap - ty;
	byte *pbMapT = &m_pbMap[ty * m_ctxMap + tx];
	int cbNextScan = m_ctxMap - ctx;
	int xTile = tx * gcxTile - xMap;
	int yTile = ty * gcyTile - yMap;
	int xTileStart = xTile;

	bool *pfInvalid = pupd->GetInvalidMap();
	Size sizMap;
	pupd->GetMapSize(&sizMap);
	int cfInvalidNextScan = sizMap.cx - ctx;
	Assert(sizMap.cx >= ctx && sizMap.cy >= cty);
	int nServiceSfx = 0;

	for (int tyT = ty; tyT < ty + cty; tyT++) {
		for (int txT = tx; txT < tx + ctx; txT++, pbMapT++) {
			byte b = *pbMapT;
			if (*pfInvalid++ == false) {
				xTile += gcxTile;
				continue;
			}

			// Is no Galaxite/Wall present or is this tile completely obscured by fog?

			byte *ptt = &pbTrMap[pbMapT - m_pbMap];
			if (((b & kbfGalaxiteMask) == 0 && *ptt != kttWall) || IsFogOpaque(b)) {
				xTile += gcxTile;
				continue;
			}

			// Galaxite?

			if (HasGalaxite(b)) {
				// Galaxite

				TBitmap *ptbm = m_aptbmGalax[s_abGxTranslate[(b & kbfGalaxiteMask) >> kcGalaxiteShift]];
				ptbm->BltTo(pbm, xTile, yTile);

#ifdef PIL
				if (gfOS5Pa1Device) {
					nServiceSfx++;
					if ((nServiceSfx & 31) == 0)
						HostSoundServiceProc();
				}
#endif

			} 

			// Wall?

			if (*ptt == kttWall) {

#define HasWall(tt) ((tt) == kttWall)

				// Check neighbor cells for walls to decide what shape this
				// segment should take.

				int ifrm = 0;
				if (tyT > 0) {
					if (HasWall(*(ptt - m_ctxMap)))
						ifrm |= 1;
				}
				if (txT < m_ctxMap - 1) {
					if (HasWall(*(ptt + 1)))
						ifrm |= 2;
				}
				if (tyT < m_ctyMap - 1) {
					if (HasWall(*(ptt + m_ctxMap)))
						ifrm |= 4;
				}
				if (txT > 0) {
					if (HasWall(*(ptt - 1)))
						ifrm |= 8;
				}
			
				// The kwfWallMask >> 1 test makes walls with less than half their health
				// draw from the 'damaged' strip.

//				m_panidWalls->DrawFrame(w <= (kwfWallMask >> 1) ? 1 : 0, ifrm, pbm, xTile, yTile, ksideNeutral);
				m_panidWalls->DrawFrame(0, ifrm, pbm, xTile, yTile, ksideNeutral);

#ifdef PIL
				if (gfOS5Pa1Device) {
					nServiceSfx++;
					if ((nServiceSfx & 31) == 0)
						HostSoundServiceProc();
				}
#endif
			}

			xTile += gcxTile;
		}

		pfInvalid += cfInvalidNextScan;
		pbMapT += cbNextScan;
		yTile += gcxTile;
		xTile = xTileStart;
	}
}

void FogMap::SetGalaxite(int nGx, TCoord tx, TCoord ty)
{
	Assert(tx >= 0 && ty >= 0 && tx < m_ctxMap && ty < m_ctyMap);

	byte *pb = &m_pbMap[(ty * m_ctxMap) + tx];
	*pb = (*pb & ~kbfGalaxiteMask) | (nGx << kcGalaxiteShift);
}

int FogMap::GetGalaxite(TCoord tx, TCoord ty)
{
	// Any attempt to look for Galaxite off the edge of the map returns none

	if (tx < 0 || ty < 0 || tx >= m_ctxMap || ty >= m_ctyMap)
		return 0;

	byte *pb = &m_pbMap[(ty * m_ctxMap) + tx];
	int nGx = (*pb & kbfGalaxiteMask) >> kcGalaxiteShift;

#ifdef MP_DEBUG
//	MpTrace("GetGalaxite:%d,%d amount %d", tx, ty, nGx);
#endif

	return nGx;
}

void FogMap::IncGalaxite(TCoord tx, TCoord ty)
{
	Assert(tx >= 0 && ty >= 0 && tx < m_ctxMap && ty < m_ctyMap);

	byte *pb = &m_pbMap[(ty * m_ctxMap) + tx];
	*pb = (*pb & ~kbfGalaxiteMask) | (s_abGxInc[(*pb & kbfGalaxiteMask) >> kcGalaxiteShift] << kcGalaxiteShift);

	// Cause this tile to redraw

	if (gptrcMapOpaque == NULL || !gptrcMapOpaque->PtIn(tx, ty))
		gpupdSim->InvalidateTile(tx, ty);
	if (gpmm != NULL)
		gpmm->RedrawTile(tx, ty);
}

bool FogMap::DecGalaxite(TCoord tx, TCoord ty)
{
	Assert(tx >= 0 && ty >= 0 && tx < m_ctxMap && ty < m_ctyMap);

	byte *pb = &m_pbMap[(ty * m_ctxMap) + tx];
	byte bNew = s_abGxDec[(*pb & kbfGalaxiteMask) >> kcGalaxiteShift] << kcGalaxiteShift;
	*pb = (*pb & ~kbfGalaxiteMask) | bNew;
#ifdef MP_DEBUG
//	MpTrace("DecGalaxite:%d,%d now %d", tx, ty, bNew >> kcGalaxiteShift);
#endif

	// Cause this tile to redraw

	if (gptrcMapOpaque == NULL || !gptrcMapOpaque->PtIn(tx, ty))
		gpupdSim->InvalidateTile(tx, ty);
	if (gpmm != NULL)
		gpmm->RedrawTile(tx, ty);

	return bNew != 0;
}

bool FogMap::FindNearestGalaxite(TCoord txOrigin, TCoord tyOrigin, TPoint *ptpt, bool fIgnoreFog)
{
	Assert(txOrigin >= 0 && tyOrigin >= 0 && txOrigin < m_ctxMap && tyOrigin < m_ctyMap);

#if 1
	TPoint *atptGx = (TPoint *)gpbScratch;
#else
	TPoint atptT[sizeof(TPoint) * ktcMax * 4]; // 1024 bytes, these days
	TPoint *atptGx = atptT;
#endif
	TPoint *ptptGx = atptGx;
	int ctptGx;

	int nMax1 = _max((int)txOrigin, m_ctxMap - txOrigin);
	int nMax2 = _max((int)tyOrigin, m_ctyMap - tyOrigin);
	int nRadiusMax = _max(nMax1, nMax2);

	for (int nRadius = 1; nRadius < nRadiusMax; nRadius++) {

		// We treat Galaxite directly under the Miner as being
		// the equivalent of 2.5 tiles away.

		if (nRadius == 3) {
			byte *pb = &m_pbMap[(tyOrigin * m_ctxMap) + txOrigin];
			if (HasGalaxite(*pb)) {
				ptpt->tx = txOrigin;
				ptpt->ty = tyOrigin;
//				MpTrace("  -- found %d, %d", txOrigin, tyOrigin);
				return true;
			}
		}

		// look for galaxite across the top

		TCoord txMin = txOrigin - nRadius;
		if (txMin < 0)
			txMin = 0;
		TCoord txMax = txOrigin + nRadius;
		if (txMax > m_ctxMap)
			txMax = m_ctxMap;

		TCoord tx, ty;
		ty = tyOrigin - nRadius;
		if (ty >= 0) {
			byte *pb = &m_pbMap[(ty * m_ctxMap) + txMin];
			for (tx = txMin; tx < txMax; tx++, pb++) {
				if (!HasGalaxite(*pb))	// Galaxite?
					continue;			// no
				if (IsFogOpaque(*pb) && !fIgnoreFog)	// Fogged?
					continue;			// yes
//				MpTrace(" t %d, %d, 0x%lx", tx, ty, ptptGx);
				ptptGx->tx = tx;
				ptptGx->ty = ty;
				ptptGx++;
			}
		}

		// look for galaxite across the bottom

		txMax += 1;
		if (txMax > m_ctxMap)
			txMax = m_ctxMap;
		txMin = txOrigin - nRadius + 1;
		if (txMin < 0)
			txMin = 0;

		ty = tyOrigin + nRadius;
		if (ty < m_ctyMap) {
			byte *pb = &m_pbMap[(ty * m_ctxMap) + txMin];
			for (tx = txMin; tx < txMax; tx++, pb++) {
				if (!HasGalaxite(*pb))
					continue;
				if (IsFogOpaque(*pb) && !fIgnoreFog)
					continue;
//				MpTrace(" b %d, %d, 0x%lx", tx, ty, ptptGx);
				ptptGx->tx = tx;
				ptptGx->ty = ty;
				ptptGx++;
			}
		}

		// look for galaxite down the right side

		TCoord tyMin = tyOrigin - nRadius;
		if (tyMin < 0)
			tyMin = 0;
		TCoord tyMax = tyOrigin + nRadius;
		if (tyMax > m_ctyMap)
			tyMax = m_ctyMap;

		tx = txOrigin + nRadius;
		if (tx < m_ctxMap) {
			byte *pb = &m_pbMap[(tyMin * m_ctxMap) + tx];
			for (ty = tyMin; ty < tyMax; ty++, pb += m_ctxMap) {
				if (!HasGalaxite(*pb))
					continue;
				if (IsFogOpaque(*pb) && !fIgnoreFog)
					continue;
//				MpTrace(" r %d, %d, 0x%lx", tx, ty, ptptGx);
				ptptGx->tx = tx;
				ptptGx->ty = ty;
				ptptGx++;
			}
		}

		// look for galaxite down the left side

		tyMin = tyOrigin - nRadius + 1;
		if (tyMin < 0)
			tyMin = 0;
		tyMax += 1;
		if (tyMax > m_ctyMap)
			tyMax = m_ctyMap;

		tx = txOrigin - nRadius;
		if (tx >= 0) {
			byte *pb = &m_pbMap[(tyMin * m_ctxMap) + tx];
			for (ty = tyMin; ty < tyMax; ty++, pb += m_ctxMap) {
				if (!HasGalaxite(*pb))
					continue;
				if (IsFogOpaque(*pb) && !fIgnoreFog)
					continue;
//				MpTrace(" l %d, %d, 0x%lx", tx, ty, ptptGx);
				ptptGx->tx = tx;
				ptptGx->ty = ty;
				ptptGx++;
			}
		}

		// If Galaxite has been found at this radius randomly pick one
		// of the found locations and return it.

		if (ptptGx != atptGx) {
			ctptGx = (int)(ptptGx - atptGx);
			int i = GetRandom() % ctptGx;
			*ptpt = atptGx[i];
//			MpTrace("  -- found %d, %d, 0x%lx [%d of %d]", ptpt->tx, ptpt->ty, &atptGx[i], i, ctptGx);
			return true;
		}
	}
	
//	MpTrace("  -- found none");
	return false;
}

//
// Wall methods
//

#if 0
int FogMap::GetWallHealth(TCoord tx, TCoord ty)
{
	// Any attempt to look for wall off the edge of the map returns none

	if (tx < 0 || ty < 0 || tx >= m_ctxMap || ty >= m_ctyMap)
		return 0;

	word *pw = &m_pwMap[(ty * m_ctxMap) + tx];
	return (*pw & kwfWallMask) >> kcWallShift;
}

void FogMap::SetWallHealth(int nHealth, TCoord tx, TCoord ty)
{
	// If state changes between existing and not existing then the N/S/E/W
	// neighbor cells need to be invalidated if they also contain a wall
	// segment.

	Assert(tx >= 0 && ty >= 0 && tx < m_ctxMap && ty < m_ctyMap);

	word *pw = &m_pwMap[(ty * m_ctxMap) + tx];
	word wOld = *pw;
	word wNew = (wOld & ~kwfWallMask) | (word)(nHealth << kcWallShift);

	if (wNew != wOld) {
		*pw = wNew;

		// UNDONE: whoa! Spaghetti! FogMap should not be reaching out to sim/level/etc
		TerrainMap *ptrmap = gsim.GetLevel()->GetTerrainMap();
		if (nHealth == 0)
			ptrmap->ClearFlags(tx, ty, 1, 1, kbfStructure);
		else
			ptrmap->SetFlags(tx, ty, 1, 1, kbfStructure);

#if 0 
		// UNDONE: right now there is no dynamic changing of a wall's health so we don't
		// need this stuff. If we allow damaging/destroying walls or give the player the
		// ability to create walls we'll need to deal with this.

		// Cause this tile to redraw

		if (gptrcMapOpaque == NULL || !gptrcMapOpaque->PtIn(tx, ty))
			gpupdSim->InvalidateTile(tx, ty);
		if (gpmm != NULL)
			gpmm->RedrawTile(tx, ty);

		// UNDONE: deal with neighbors
#endif
	}
}
#endif

} // namespace wi
