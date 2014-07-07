#include "ht.h"

namespace wi {

TileMap *LoadTileMap(char *pszFn, Size *psizPlayfield)
{
	TileMap *ptmap = new TileMap;
	if (!ptmap->Load(pszFn, psizPlayfield)) {
		delete ptmap;
		return NULL;
	}
	return ptmap;
}

TileMap::TileMap()
{
	memset(m_aptseth, 0, sizeof(m_aptseth));
	m_apbTileData = NULL;
	m_cTileSets = 0;
	m_cTiles = 0;
	m_wf = 0;
	m_apbDrawMap = NULL;
	m_pmtseth = NULL;
}

TileMap::~TileMap()
{
	if (m_wf & kfTmapMapped) {
		gpakr.UnmapFile(&m_fmapTmap);
		for (int nTset = 0; nTset < m_cTileSets; nTset++)
			gpakr.UnmapFile(&m_afmapTset[nTset]);
	}
	if (m_wf & kfMiniTsetMapped)
		gpakr.UnmapFile(&m_fmapMiniTset);
	if (m_apbTileData != NULL)
		gmmgr.FreePtr(m_apbTileData);
	delete m_apbDrawMap;
}

bool TileMap::Load(char *psz, Size *psizPlayfield)
{
	// First try to alloc local map

	m_ctx = (psizPlayfield->cx + (gcxTile - 1)) / gcxTile + 1;
	m_cty = (psizPlayfield->cy + (gcyTile - 1)) / gcyTile + 1;
	m_apbDrawMap = new byte *[m_ctx * m_cty];
	if (m_apbDrawMap == NULL)
		return false;

	// Load the tile data

	m_ptmaph = (TileMapHeader *)gpakr.MapFile(psz, &m_fmapTmap);
	if (m_ptmaph == NULL)
		return false;
	m_wf |= kfTmapMapped;
	m_pwMapData = (word *)(m_ptmaph + 1);

	// Start loading tsets.

	m_cTileSets = 0;
	m_cTiles = 0;
	while (true) {
		char sz[kcchFnTset];
		strcpy(sz, m_ptmaph->szFnTset);
		if (m_cTileSets != 0) {
			char szT[8];
			szT[0] = '.';
			itoa(m_cTileSets, &szT[1], 10);
			strcat(sz, szT);
		}
		TileSetHeader *ptseth = (TileSetHeader *)gpakr.MapFile(sz, &m_afmapTset[m_cTileSets]);
		if (ptseth == NULL)
			break;
		m_aptseth[m_cTileSets] = ptseth;
		m_cxTile = BigWord(ptseth->cxTile);
		m_cyTile = BigWord(ptseth->cyTile);
		m_cTileSets++;
		m_cTiles += BigWord(ptseth->cTiles);
	}
	if (m_cTiles == 0)
		return false;

	// Alloc enough tile pointers to point to the individual tiles

	byte **apbTileData = new byte *[m_cTiles];
	if (apbTileData == NULL)
		return false;

	// Fill in the pointers to tile data

	byte **ppb = apbTileData;
	int cbTile = m_cxTile * m_cyTile;
	for (int nTset = 0; nTset < m_cTileSets; nTset++) {
		TileSetHeader *ptseth = m_aptseth[nTset];
		byte *pbTileData = (byte *)ptseth + kcbTileSetHeader;
		for (int nTile = 0; nTile < BigWord(ptseth->cTiles); nTile++) {
			*ppb = pbTileData;
			ppb++;
			pbTileData += cbTile;
		}
	}

	// Save away

	int cbT = sizeof(byte *) * m_cTiles;
	m_apbTileData = (byte **)gmmgr.AllocPtr(cbT);
	if (m_apbTileData == NULL) {
		delete apbTileData;
		return false;
	}
	gmmgr.WritePtr(m_apbTileData, 0, apbTileData, cbT);
	delete apbTileData;

	// Load mini tset

	char sz[50];
	strcpy(sz, m_ptmaph->szFnTset);
	strcat(sz, "mini");
	m_pmtseth = (MiniTileSetHeader *)gpakr.MapFile(sz, &m_fmapMiniTset);
	if (m_pmtseth == NULL)
		return false;
	m_wf |= kfMiniTsetMapped;

	// We're done

	return true;
}

MiniTileSetHeader *TileMap::GetMiniTileSetHeader(int nScale)
{
	MiniTileSetHeader *pmtseth = m_pmtseth;
	while (true) {
		if (BigWord(pmtseth->cxTile) == nScale)
			return pmtseth;
		if (pmtseth->offNext == 0)
			return NULL;
		pmtseth = (MiniTileSetHeader *)((byte *)pmtseth + BigWord(pmtseth->offNext));
	}
}

void TileMap::GetMapSize(Size *psiz)
{
	psiz->cx = BigWord(m_ptmaph->ctx) * m_cxTile;
	psiz->cy = BigWord(m_ptmaph->cty) * m_cyTile;
}

void TileMap::GetTileSize(Size *psiz)
{
	psiz->cx = m_cxTile;
	psiz->cy = m_cyTile;
}

void TileMap::Draw(DibBitmap *pbm, int x, int y, int cx, int cy, int xMap, int yMap, byte *pbFogMap, UpdateMap *pupd)
{
	Assert(!((xMap | yMap) & 1));

	//For debugging:
	//BitmapType *pbmpScreen = WinGetBitmap(WinGetDisplayWindow());
	//pbBits = (byte *)BmpGetBits(pbmpScreen);

	// Get the tile coordinates into the map from which drawing is to start

	int tx = TcFromPc(xMap);
	int ty = TcFromPc(yMap);

	// Figure map info

	MapInfo *pmnfo = pupd->GetMapInfo();
	int ctx = TcFromPc(cx - pmnfo->cxLeftTile + gcxTile - 1) + TcFromPc(pmnfo->cxLeftTile + gcxTile - 1);
	int cty = TcFromPc(cy - pmnfo->cyTopTile + gcyTile - 1) + TcFromPc(pmnfo->cyTopTile + gcyTile - 1);
	int ctxMap = BigWord(m_ptmaph->ctx);

#ifdef DEBUG
	Assert(ctx <= m_ctx);
	Assert(cty <= m_cty);
	Assert(tx + ctx <= BigWord(m_ptmaph->ctx));
	Assert(ty + cty <= BigWord(m_ptmaph->cty));
	Size sizT;
	pupd->GetMapSize(&sizT);
	Assert(sizT.cx == m_ctx && sizT.cy == m_cty);
#endif

	// Get running pointers into various maps
	// Note: Fog will be accounted for in updatemap in the near future

	word iCell = ty * ctxMap + tx;
	word *pwMapT = &m_pwMapData[iCell];
	byte *pbFogT = &pbFogMap[iCell];
	bool *pfInvalid = pupd->GetInvalidMap();
	byte **ppbDrawMap = m_apbDrawMap;

	int cReturnDrawMap = m_ctx - ctx;
	int cReturnTileMap = ctxMap - ctx;
	for (int tyT = 0; tyT < cty; tyT++) {
		Assert(ty + tyT < BigWord(m_ptmaph->cty));
		for (int txT = 0; txT < ctx; txT++) {
			Assert(tx + txT < BigWord(m_ptmaph->ctx));

			*ppbDrawMap = NULL;
			if (!IsFogOpaque(*pbFogT)) {
				if (*pfInvalid) {
                    word offset = BigWord(*pwMapT);
                    Assert(offset < 0xff00);
                    *ppbDrawMap = *(byte **)((long)m_apbTileData + offset);
				}
			}

			ppbDrawMap++;
			pbFogT++;
			pfInvalid++;
			pwMapT++;
		}

		ppbDrawMap += cReturnDrawMap;
		pfInvalid += cReturnDrawMap;
		pwMapT += cReturnTileMap;
		pbFogT += cReturnTileMap;
	}
					 
	// Now draw this map
#if 0
	BitmapType *pbmpScreen = WinGetBitmap(WinGetDisplayWindow());
	byte *pbDib = (byte *)BmpGetBits(pbmpScreen);
	Size sizFill;
	pbm->GetSize(&sizFill);
	Fill(pbDib, sizFill.cx, sizFill.cy, sizFill.cx, 1);
#else
	byte *pbDib = pbm->GetBits();
#endif

	Size sizDib;
	pbm->GetSize(&sizDib);
	pbDib += y * sizDib.cx + x;

	DrawTileMapThunk(m_apbDrawMap, m_ctx, m_cty, pbDib, sizDib.cx, pmnfo->cxLeftTile, pmnfo->cyTopTile, pmnfo->cxRightTile, pmnfo->cyBottomTile, pmnfo->ctxInside, pmnfo->ctyInside, gcxTile, gcyTile);
}

} // namespace wi