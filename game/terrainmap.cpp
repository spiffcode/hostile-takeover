#include "ht.h"

namespace wi {

/*
for (j = 0; j < 64; j += 8) {
        var str = "";
        for (i = 0; i < 8; i++)
                str += (j + i) * (j + i) + ", ";
        WScript.Echo(str);
}
*/

int ganSquared[64] = {
	0, 1, 4, 9, 16, 25, 36, 49,
	64, 81, 100, 121, 144, 169, 196, 225,
	256, 289, 324, 361, 400, 441, 484, 529,
	576, 625, 676, 729, 784, 841, 900, 961,
	1024, 1089, 1156, 1225, 1296, 1369, 1444, 1521,
	1600, 1681, 1764, 1849, 1936, 2025, 2116, 2209,
	2304, 2401, 2500, 2601, 2704, 2809, 2916, 3025,
	3136, 3249, 3364, 3481, 3600, 3721, 3844, 3969,
};

byte gmpdirdirOpposite[8] = { 4, 5, 6, 7, 0, 1, 2, 3 };

// Road
// Open
// Wall
// Blocked

#define knScoreBlocked 10000
#define knScoreStructure 9000

word TerrainMap::s_anScoreTerrain[] = { 0, 0, knScoreBlocked, knScoreBlocked };

// Cost of moving by one tile in a certain direction

word TerrainMap::s_anScoreVector[] = { 5, 7, 5, 7, 5, 7, 5, 7 };

// Max nodes futility cutoff. Traverse the map three times, and * 3 because of how
// nodes are added to the list

#define kcNodesMax (ktcMax * 3 * 3)

TerrainMap::TerrainMap()
{
	m_ptrmaph = NULL;
	m_abBuffer = NULL;
	m_ppathhList = NULL;
	m_ppathhFreeList = NULL;
	m_cNodes = 0;
}

TerrainMap::~TerrainMap()
{
	if (m_ptrmaph != NULL)
		gpakr.UnmapFile(&m_fmap);
	delete[] m_abBuffer;
	while (m_ppathhList != NULL)
		RemovePathHead(m_ppathhList);
	for (PathHead *ppathh = m_ppathhFreeList; ppathh != NULL; ) {
		PathHead *ppathhT = ppathh;
		ppathh = ppathh->ppathhNext;
		delete ppathhT;
	}
}

bool TerrainMap::Init(char *pszFn)
{
	// Load the terrain map

	m_ptrmaph = (TerrainMapHeader *)gpakr.MapFile(pszFn, &m_fmap);
	if (m_ptrmaph == NULL)
		return false;

	// Alloc the path buffer

	m_ctx = BigWord(m_ptrmaph->ctx);
	m_cty = BigWord(m_ptrmaph->cty);
	m_cbBuffer = m_ctx * m_cty;
	m_abBuffer = new byte[m_cbBuffer];
	Assert(m_abBuffer != NULL, "out of memory!");
	if (m_abBuffer == NULL)
		return false;
	memset(m_abBuffer, 0, m_cbBuffer);

	// Init

	for (int n = 0; n < 8; n++)
		m_mpDirDelta[n] = g_mpDirToDy[n] * m_ctx + g_mpDirToDx[n];

	return true;
}

#ifdef MP_DEBUG
dword TerrainMap::GetChecksum()
{
	dword n = 0;
	for (int i = 0; i < m_cbBuffer; i++)
		n += m_abBuffer[i];
	return n;
}
#endif

#define knVerTerrainMapState 1
bool TerrainMap::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerTerrainMapState)
		return false;
	pstm->ReadBytesRLE(m_abBuffer, m_cbBuffer);
	return pstm->IsSuccess();
}

bool TerrainMap::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerTerrainMapState);
	for (int n = 0; n < m_cbBuffer; n++)
		m_abBuffer[n] &= kbfMobileUnit | kbfStructure | kbfReserved;
	pstm->WriteBytesRLE(m_abBuffer, m_cbBuffer);
	return pstm->IsSuccess();
}

void TerrainMap::SetFlags(int tx, int ty, int ctx, int cty, byte bf)
{
	Assert(tx >= 0 && ty >= 0);
	Assert(tx + ctx <= m_ctx && ty + cty <= m_cty);

	for (int txT = tx; txT < tx + ctx; txT++) {
		for (int tyT = ty; tyT < ty + cty; tyT++) {
			m_abBuffer[tyT * m_ctx + txT] |= bf;
		}
	}

#ifdef MP_DEBUG
//	MpTrace("Set Terrain Flags:%d,%d,%d,%d set 0x%02x", tx, ty, ctx, cty, bf);
#endif
}

bool TerrainMap::TestFlags(int tx, int ty, int ctx, int cty, byte bf)
{
	// We consider all tiles off the edge of the map to be fully occupied

	if (tx < 0 || ty < 0 || tx + ctx > m_ctx || ty + cty > m_cty)
		return true;

	for (int txT = tx; txT < tx + ctx; txT++) {
		for (int tyT = ty; tyT < ty + cty; tyT++) {
			int off = tyT * m_ctx + txT;
			if (m_abBuffer[off] & bf)
				return true;
		}
	}
	return false;
}

bool TerrainMap::IsOccupied(int tx, int ty, int ctx, int cty, byte bf)
{
	// We consider all tiles off the edge of the map to be fully occupied

	if (tx < 0 || ty < 0 || tx + ctx > m_ctx || ty + cty > m_cty)
		return true;

	for (int txT = tx; txT < tx + ctx; txT++) {
		for (int tyT = ty; tyT < ty + cty; tyT++) {
			int off = tyT * m_ctx + txT;
			if (m_abBuffer[off] & bf)
				return true;
			if (((byte *)(m_ptrmaph + 1))[off] > kttOpen)
				return true;
		}
	}
	return false;
}

void TerrainMap::ClearFlags(int tx, int ty, int ctx, int cty, byte bf)
{
	Assert(tx >= 0 && ty >= 0);
	Assert(tx + ctx <= m_ctx && ty + cty <= m_cty);

	for (int txT = tx; txT < tx + ctx; txT++) {
		for (int tyT = ty; tyT < ty + cty; tyT++) {
			m_abBuffer[tyT * m_ctx + txT] &= ~bf;
		}
	}

#ifdef MP_DEBUG
//	MpTrace("Clear Terrain Flags:%d,%d,%d,%d clear 0x%02x", tx, ty, ctx, cty, bf);
#endif
}

bool TerrainMap::IsLineOccupied(TCoord txFrom, TCoord tyFrom, TCoord txTo, TCoord tyTo, byte bfTerrainAvoid)
{
	// There already?

	if (txFrom == txTo && tyFrom == tyTo)
		return false;

	// Number of x's and y's to travel

	int dx = abs(txTo - txFrom);
	int dy = abs(tyTo - tyFrom);

	// Direction on each axis

	int sx, sy;
	if (txFrom < txTo) {
		sx = 1;
	} else {
		sx = -1;
	}
	if (tyFrom < tyTo) {
		sy = 1;
	} else {
		sy = -1;
	}

	TCoord txT = txFrom;
	TCoord tyT = tyFrom;
	int nStep = 0;
	int dc = 0;
	while (true) {
		if (dx > dy) {
			txT += sx;
			dc += dy;
			if (dc >= dx) {
				dc -= dx;
				tyT += sy;
			}
		} else {
			tyT += sy;
			dc += dx;
			if (dc >= dy) {
				dc -= dy;
				txT += sx;
			}
		}

		// If this is blocking, bail

		if (IsBlocked(txT, tyT, bfTerrainAvoid))
			return true;

		// If we're there, break

		if (txT == txTo && tyT == tyTo)
			break;
	}

	return false;
}

bool TerrainMap::FindFirstUnoccupied(TCoord txFrom, TCoord tyFrom, TCoord txTo, TCoord tyTo, TCoord *ptxFree, TCoord *ptyFree)
{
	// Number of x's and y's to travel

	int dx = abs(txTo - txFrom);
	int dy = abs(tyTo - tyFrom);

	// Direction on each axis

	int sx, sy;
	if (txFrom < txTo) {
		sx = 1;
	} else {
		sx = -1;
	}
	if (tyFrom < tyTo) {
		sy = 1;
	} else {
		sy = -1;
	}

	// If this is blocking, bail

	TCoord txT = txFrom;
	TCoord tyT = tyFrom;
	if (!IsBlocked(txT, tyT, 0)) {
		*ptxFree = txT;
		*ptyFree = tyT;
		return true;
	}
	if (txFrom == txTo && tyFrom == tyTo)
		return false;

	int nStep = 0;
	int dc = 0;
	while (true) {
		if (dx > dy) {
			txT += sx;
			dc += dy;
			if (dc >= dx) {
				dc -= dx;
				tyT += sy;
			}
		} else {
			tyT += sy;
			dc += dx;
			if (dc >= dy) {
				dc -= dy;
				txT += sx;
			}
		}

		// If this is blocking, bail

		if (!IsBlocked(txT, tyT, 0)) {
			*ptxFree = txT;
			*ptyFree = tyT;
			return true;
		}

		// If we're there, break

		if (txT == txTo && tyT == tyTo)
			break;
	}

	return false;
}

bool TerrainMap::FindLastUnoccupied(TCoord txFrom, TCoord tyFrom, TCoord txTo, TCoord tyTo, TCoord *ptxFree, TCoord *ptyFree)
{
	// Number of x's and y's to travel

	int dx = abs(txTo - txFrom);
	int dy = abs(tyTo - tyFrom);

	// Direction on each axis

	int sx, sy;
	if (txFrom < txTo) {
		sx = 1;
	} else {
		sx = -1;
	}
	if (tyFrom < tyTo) {
		sy = 1;
	} else {
		sy = -1;
	}

	// If this is not blocking, remember

	TCoord txT = txFrom;
	TCoord tyT = tyFrom;
	if (IsBlocked(txT, tyT, 0))
		return false;
	*ptxFree = txT;
	*ptyFree = tyT;
	if (txFrom == txTo && tyFrom == tyTo)
		return true;

	int nStep = 0;
	int dc = 0;
	while (true) {
		if (dx > dy) {
			txT += sx;
			dc += dy;
			if (dc >= dx) {
				dc -= dx;
				tyT += sy;
			}
		} else {
			tyT += sy;
			dc += dx;
			if (dc >= dy) {
				dc -= dy;
				txT += sx;
			}
		}

		// If this is not blocking, remember it

		if (IsBlocked(txT, tyT, 0))
			return true;
		*ptxFree = txT;
		*ptyFree = tyT;

		// If we're there, break

		if (txT == txTo && tyT == tyTo)
			break;
	}

	return true;
}

Path *TerrainMap::FindLinePath(TCoord txFrom, TCoord tyFrom, TCoord txTo, TCoord tyTo, byte bfTerrainAvoid)
{
	if (txFrom == txTo && tyFrom == tyTo)
		return NULL;

	TPoint atpt[ktcMax + ktcMax / 2];

	// Number of x's and y's to travel

	int dx = abs(txTo - txFrom);
	int dy = abs(tyTo - tyFrom);

	// Direction on each axis

	int sx, sy;
	if (txFrom < txTo) {
		sx = 1;
	} else {
		sx = -1;
	}
	if (tyFrom < tyTo) {
		sy = 1;
	} else {
		sy = -1;
	}

	TCoord txT = txFrom;
	TCoord tyT = tyFrom;
	int nStep = 0;
	int dc = 0;
	while (true) {
		if (dx > dy) {
			txT += sx;
			dc += dy;
			if (dc >= dx) {
				dc -= dx;
				tyT += sy;
			}
		} else {
			tyT += sy;
			dc += dx;
			if (dc >= dy) {
				dc -= dy;
				txT += sx;
			}
		}

		// If this is blocking, bail

		if (IsBlocked(txT, tyT, bfTerrainAvoid))
			return NULL;

		// Location looks good, save it

		atpt[nStep].tx = (byte)txT;
		atpt[nStep].ty = (byte)tyT;
		nStep++;
		Assert(nStep < ARRAYSIZE(atpt));

		// If we're there, break

		if (txT == txTo && tyT == tyTo)
			break;
	}

	// Create a path from this

	return CreatePath(this, txFrom, tyFrom, atpt, nStep);
}

Path *TerrainMap::FindPath(int txFrom, int tyFrom, int txTo, int tyTo, byte bfTerrainAvoid)
{
	// Arbitrary pathing

	if (txFrom == txTo && tyFrom == tyTo)
		return NULL;

	// Quick out. Miners do this when they want to get to the dump point but it's set because
	// the processor is occupied.

	if (abs(txFrom - txTo) <= 1 && abs(tyFrom - tyTo) <= 1) {
		TPoint tptT;
		tptT.tx = txTo;
		tptT.ty = tyTo;
		return CreatePath(this, txFrom, tyFrom, &tptT, 1);
	}

	// Initialize.

#if 0
	for (int n = 0; n < m_cbBuffer; n++)
		m_abBuffer[n] &= kbfMobileUnit | kbfStructure | kbfReserved;
#else
#define kbClear (kbfMobileUnit | kbfStructure | kbfReserved)
#define kdwClear MakeDword(kbClear, kbClear, kbClear, kbClear)

	// Try to speed up initial buffer clearning 
	// If not dword aligned, adjust

	byte *pbT = m_abBuffer;
  if (*((byte *)&pbT) & 2) {
		*pbT++ &= kbClear;
		*pbT++ &= kbClear;
	}

	// 32 byte aligned count of dwords

	dword *pdwT = (dword *)pbT;
	int cdw = (((dword *)&m_abBuffer[m_cbBuffer]) - pdwT) & ~7;
	dword *pdwMax = &((dword *)m_abBuffer)[cdw];
	dword dwClear = kdwClear;
	while (pdwT < pdwMax) {
		*pdwT++ &= dwClear;
		*pdwT++ &= dwClear;
		*pdwT++ &= dwClear;
		*pdwT++ &= dwClear;
		*pdwT++ &= dwClear;
		*pdwT++ &= dwClear;
		*pdwT++ &= dwClear;
		*pdwT++ &= dwClear;
	}

	// Clear dwords (up to 7 dwords)

	pdwT = pdwMax;
	int cdwT = (int)(((dword *)&m_abBuffer[m_cbBuffer]) - pdwT);
	pdwMax = &pdwT[cdwT];
	while (pdwT < pdwMax)
		*pdwT++ &= dwClear;

	// Clear remaining bytes (up to 3 bytes)

	switch (&m_abBuffer[m_cbBuffer] - (byte *)pdwMax) {
	case 0:
		break;
	case 1:
		((byte *)pdwMax)[0] &= kbClear;
		break;

	case 2:
		((byte *)pdwMax)[0] &= kbClear;
		((byte *)pdwMax)[1] &= kbClear;
		break;

	case 3:
		((byte *)pdwMax)[0] &= kbClear;
		((byte *)pdwMax)[1] &= kbClear;
		((byte *)pdwMax)[2] &= kbClear;
		break;

	default:
		Assert();
		break;
	}
#endif

	while (m_ppathhList != NULL)
		RemovePathHead(m_ppathhList);
	Assert(m_ppathhList == NULL);

	byte *pbTo = &m_abBuffer[tyTo * m_ctx + txTo];

	PathHead pathh;
	memset(&pathh, 0, sizeof(pathh));
	pathh.offHead = tyFrom * m_ctx + txFrom;
	pathh.txLast = txFrom;
	pathh.tyLast = tyFrom;
	PathHead *ppathhLast = &pathh;

	m_abBuffer[pathh.offHead] |= kbfLinked;

	word offClosest;
	word nDistClosest = (word)-1;
	while (ppathhLast != NULL) {
		for (int n = 0; n < 8; n++) {
			// Get next pos

			int txNew = ppathhLast->txLast + g_mpDirToDx[n];
			int tyNew = ppathhLast->tyLast + g_mpDirToDy[n];

			// Check out of bounds. If we use 64*64 maps, we can use an AND mask
			// of 0x2040 to determine out of bounds conditions.

			if (txNew < 0 || txNew >= m_ctx || tyNew < 0 || tyNew >= m_cty)
				continue;

			// If this tile is linked already, abandon this path since
			// it is longer

			byte *pbNew = &m_abBuffer[ppathhLast->offHead] + m_mpDirDelta[n];
			if (*pbNew & kbfLinked)
				continue;

			// The tile it came from is no longer a head

			if (m_abBuffer[ppathhLast->offHead] & kbfHead) {
				m_abBuffer[ppathhLast->offHead] &= ~kbfHead;
				if (ppathhLast != &pathh) {
					pathh = *ppathhLast;
					RemovePathHead(ppathhLast);
					ppathhLast = &pathh;
				}
			}

			// Calc score of terrain

			word offHead = pbNew - m_abBuffer;
			word wScoreTerrain;
			if (*pbNew & bfTerrainAvoid) {
				wScoreTerrain = knScoreStructure;
			} else {
				wScoreTerrain = s_anScoreTerrain[((byte *)(m_ptrmaph + 1))[offHead]];
			}
#ifdef STATS_DISPLAY
			gcPathScoresCalced++;
#endif

			// Link in this square, add it to the list. It is a path head.

			*pbNew = (*pbNew & (kbfMobileUnit | kbfStructure | kbfReserved)) | kbfHead | kbfLinked | n;
			word wScoreKnown = ppathhLast->wScoreKnown + s_anScoreVector[n] + wScoreTerrain;
			word nDist = ganSquared[abs(txNew - txTo)] + ganSquared[abs(tyNew - tyTo)];
			word wScore = wScoreKnown + nDist;
			PathHead *ppathhNew = AddPathHead(wScoreKnown, wScore, ppathhLast->cSteps + 1, offHead, txNew, tyNew);

			// Remember "closest" point

			if (nDist < nDistClosest) {
				nDistClosest = nDist;
				offClosest = pbNew - m_abBuffer;
			}

			// Either out of memory or max # nodes reached. Return best path so far

			if (ppathhNew == NULL)
				return MakePath(txFrom, tyFrom, offClosest);

			// Are we at the destination? Then we're done!

			if (pbNew == pbTo)
				return MakePath(txFrom, tyFrom, ppathhNew->offHead);
		}

		// Perhaps this was a dead end?

		if (ppathhLast == m_ppathhList)
			RemovePathHead(ppathhLast);

		// Grab the best path and do again

		ppathhLast = m_ppathhList;
	}

	return NULL;
}

PathHead *TerrainMap::AddPathHead(word wScoreKnown, word wScore, int cSteps, word offHead, int txNew, int tyNew)
{
	// Alloc

	PathHead *ppathh;
	if (m_ppathhFreeList != NULL) {
		ppathh = m_ppathhFreeList;
		m_ppathhFreeList = ppathh->ppathhNext;
	} else {
		if (m_cNodes >= kcNodesMax)
			return NULL;
		ppathh = new PathHead;
		if (ppathh == NULL)
			return NULL;
		m_cNodes++;
	}

	// Initialize

	ppathh->txLast = txNew;
	ppathh->tyLast = tyNew;
	ppathh->cSteps = cSteps;
	ppathh->offHead = offHead;
	ppathh->wScoreKnown = wScoreKnown;
	ppathh->wScore = wScore;

	// Sorted insertion
	// OPT: The fast approach here will be to use a binary tree of some variety
	//      (AVL / Right Handed / Splay etc, TBD). For the time being, using
	//      a simple linear list w/scanning for simplicity to get searching
	//      working.

	for (PathHead **pppathhT = &m_ppathhList; true; pppathhT = &(*pppathhT)->ppathhNext) {
		if (*pppathhT == NULL) {
			*pppathhT = ppathh;
			ppathh->ppathhNext = NULL;
			break;
		}
		if (ppathh->wScore <= (*pppathhT)->wScore) {
			ppathh->ppathhNext = *pppathhT;
			*pppathhT = ppathh;
			break;
		}
	}

	return ppathh;
}

void TerrainMap::RemovePathHead(PathHead *ppathh)
{
	for (PathHead **pppathhT = &m_ppathhList; (*pppathhT) != NULL; pppathhT = &(*pppathhT)->ppathhNext) {
		if (*pppathhT == ppathh) {
			*pppathhT = ppathh->ppathhNext;
			ppathh->ppathhNext = m_ppathhFreeList;
			m_ppathhFreeList = ppathh;
			return;
		}
	}
	Assert();
}

bool TerrainMap::IsBlocked(TCoord tx, TCoord ty, byte bf)
{
	word offMap = ty * m_ctx + tx;

	// We path through structures and blocked terrain (at very high cost) so that structures or
	// terrain that are "blocking" can at least be approached (albeit at high pathing cost)
	// as opposed to be totally blocked and never considered. Then at path following we determine
	// if there is blockage with this method.

	if (m_abBuffer[offMap] & bf)
		return true;
	if (((byte *)(m_ptrmaph + 1))[offMap] > kttOpen)
		return true;

	return false;
}

int TerrainMap::GetTerrainType(TCoord tx, TCoord ty)
{
	Assert(tx >= 0 && tx < m_ctx && ty >= 0 && ty < m_cty);
	word offMap = ty * m_ctx + tx;
	return ((byte *)(m_ptrmaph + 1))[offMap];
}

bool TerrainMap::GetFlags(int tx, int ty, byte *pbf)
{
	Assert(tx >= 0 && ty >= 0);
	Assert(tx < m_ctx && ty < m_cty);

	word offMap = ty * m_ctx + tx;
	if (((byte *)(m_ptrmaph + 1))[offMap] > kttOpen)
		return false;
	*pbf = m_abBuffer[offMap];
	return true;
}

Path *TerrainMap::MakePath(TCoord txStart, TCoord tyStart, word off)
{
	Direction adir[kcNodesMax];

	TCoord ty = off / m_ctx;
	TCoord tx = off % m_ctx;
	byte *pbPathWalk = &m_abBuffer[off];
	Direction *pdirT = &adir[ARRAYSIZE(adir)];
	while (tx != txStart || ty != tyStart) {
		Assert(*pbPathWalk & kbfLinked);
		Direction dir = (*pbPathWalk) & kbfDirMask;
		pdirT--;
		*pdirT = dir;
		Direction dirOpposite = gmpdirdirOpposite[dir];
		pbPathWalk += m_mpDirDelta[dirOpposite];
		tx += g_mpDirToDx[dirOpposite];
		ty += g_mpDirToDy[dirOpposite];
	}

	return CreatePath(this, txStart, tyStart, pdirT, (int)(&adir[ARRAYSIZE(adir)] - pdirT));
}

//
// Path class
//

Path *CreatePath(TerrainMap *ptrmap, TCoord txStart, TCoord tyStart, TPoint *atpt, int ctpt)
{
	Direction adir[kcNodesMax];

	TCoord txLast = txStart;
	TCoord tyLast = tyStart;
	for (int idir = 0; idir < ctpt; idir++) {
		adir[idir] = DirectionFromLocations(txLast, tyLast, atpt[idir].tx, atpt[idir].ty);
		txLast = atpt[idir].tx;
		tyLast = atpt[idir].ty;
	}

	return CreatePath(ptrmap, txStart, tyStart, adir, ctpt);
}

Path *CreatePath(TerrainMap *ptrmap, TCoord txStart, TCoord tyStart, Direction *adir, int cdir)
{
	Path *ppath = new Path;
	if (ppath == NULL)
		return NULL;
	if (!ppath->Init(ptrmap, txStart, tyStart, adir, cdir)) {
		delete ppath;
		return NULL;
	}
	return ppath;
}

Path::Path()
{
	m_cdirs = 0;
	m_adir = NULL;
}

Path::~Path()
{
	delete[] m_adir;
}

bool Path::Init(TerrainMap *ptrmap, TCoord txStart, TCoord tyStart, Direction *adir, int cdir)
{
	m_adir = new Direction[cdir];
	if (m_adir == NULL)
		return false;
	memcpy(m_adir, adir, cdir * sizeof(Direction));

	m_cdirs = cdir;
	m_txStart = txStart;
	m_tyStart = tyStart;

	// Init cache

	m_idirCache = -1;
	m_txCache = txStart;
	m_tyCache = tyStart;
	m_ptrmap = ptrmap;

	return true;
}

int Path::GetCount()
{
	return m_cdirs;
}

bool Path::GetPoint(int itpt, TPoint *ptpt, byte bf)
{
	if (itpt < 0 || itpt >= m_cdirs)
		return false;

	TCoord txT, tyT;
	CalcTo(itpt, &txT, &tyT);

	if (m_ptrmap->IsBlocked(txT, tyT, bf))
		return false;

	ptpt->tx = txT;
	ptpt->ty = tyT;
	return true;
}

bool Path::GetPointRaw(int itpt, TPoint *ptpt)
{
	if (itpt < 0 || itpt >= m_cdirs)
		return false;
	TCoord txT, tyT;
	CalcTo(itpt, &txT, &tyT);
	ptpt->tx = txT;
	ptpt->ty = tyT;
	return true;
}

void Path::SetCacheIndex(int idir)
{
	if (idir < 0 || idir >= m_cdirs)
		return;

	TCoord txT, tyT;
	CalcTo(idir, &txT, &tyT);
	m_idirCache = idir;
	m_txCache = txT;
	m_tyCache = tyT;
}

void Path::CalcTo(int idir, TCoord *ptx, TCoord *pty)
{
	if (idir < m_idirCache) {
		m_idirCache = -1;
		m_txCache = m_txStart;
		m_tyCache = m_tyStart;
		Assert(false);
	}

	int idirT = m_idirCache;
	TCoord txT = m_txCache;
	TCoord tyT = m_tyCache;
	while (true) {
		idirT++;
		if (idirT > idir)
			break;
		txT += g_mpDirToDx[m_adir[idirT]];
		tyT += g_mpDirToDy[m_adir[idirT]];
	}

	*ptx = txT;
	*pty = tyT;
}

int Path::FindClosestPoint(TCoord txFrom, TCoord tyFrom, int itptStart, int ctptFurtherStop, TPoint *ptptClosest)
{
	int ctptRemaining = m_cdirs - itptStart;

	word n2Last = (word)-1;
	word nSmallest = (word)-1;
	int jSmallest = -1;
	int ctptFurtherStopT = ctptFurtherStop;
	for (int j = 0; j < ctptRemaining; j++) {
		// Get location. False would mean it's blocked

		TPoint tpt;
		if (!GetPointRaw(j + itptStart, &tpt))
			break;

		// If we're on the path, return that

		if (tpt.tx == txFrom && tpt.ty == tyFrom) {
			*ptptClosest = tpt;
			return j + itptStart;
		}

		// Calc smallest distance, always advance if we get smaller

		word n2 = ganSquared[abs(tpt.tx - txFrom)] + ganSquared[abs(tpt.ty - tyFrom)];
		if (n2 <= nSmallest) {
			nSmallest = n2;
			jSmallest = j;
			*ptptClosest = tpt;
		}

		// If we've increased distance the last cFurtherAwayStop times, stop

		if (n2 > n2Last) {
			ctptFurtherStopT--;
			if (ctptFurtherStopT == 0)
				break;
		} else {
			ctptFurtherStopT = ctptFurtherStop;
		}
		n2Last = n2;
	}

	if (jSmallest == -1)
		return -1;
	return jSmallest + itptStart;
}

bool Path::Append(Path *ppath)
{
	Direction *adirNew = new Direction[m_cdirs + ppath->m_cdirs];
	if (adirNew == NULL)
		return false;
	memcpy(adirNew, m_adir, m_cdirs);
	memcpy(&adirNew[m_cdirs], ppath->m_adir, ppath->m_cdirs);
	delete[] m_adir;
	m_adir = adirNew;
	m_cdirs += ppath->m_cdirs;
	return true;
}

bool Path::TrimEnd(int itptStart)
{
	if (itptStart >= m_cdirs)
		return false;
	int cdirsNew = itptStart;
	Direction *adirNew = new Direction[cdirsNew];
	if (adirNew == NULL)
		return false;
	memcpy(adirNew, m_adir, cdirsNew);
	delete[] m_adir;
	m_adir = adirNew;
	m_cdirs = cdirsNew;
	return true;
}

Path *Path::Clone()
{
	Path *ppathNew = new Path();
	if (ppathNew == NULL)
		return NULL;

	ppathNew->m_ptrmap = m_ptrmap;
	ppathNew->m_cdirs = m_cdirs;
	ppathNew->m_txStart = m_txStart;
	ppathNew->m_tyStart = m_tyStart;
	ppathNew->m_idirCache = -1;
	ppathNew->m_txCache = m_txStart;
	ppathNew->m_tyCache = m_tyStart;

	ppathNew->m_adir = new Direction[m_cdirs];
	if (ppathNew->m_adir == NULL) {
		delete ppathNew;
		return NULL;
	}
	memcpy(ppathNew->m_adir, m_adir, m_cdirs);
	return ppathNew;
}

void Path::GetStartPoint(TPoint *ptpt)
{
	ptpt->tx = m_txStart;
	ptpt->ty = m_tyStart;
}

#define knVerPathState 4
bool Path::LoadState(TerrainMap *ptrmap, Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerPathState)
		return false;

	m_ptrmap = ptrmap;
	m_cdirs = pstm->ReadWord();
	m_txStart = pstm->ReadWord();
	m_tyStart = pstm->ReadWord();
	m_adir = new Direction[m_cdirs];
	if (m_adir == NULL)
		return false;
	m_idirCache = (int)(short)pstm->ReadWord();
	m_txCache = pstm->ReadWord();
	m_tyCache = pstm->ReadWord();

	byte bT;
	for (int idir = 0; idir < m_cdirs; idir++) {
		Direction dir;
		if ((idir & 1) == 0) {
			bT = pstm->ReadByte();
			dir = (bT & 0xf0) >> 4;
		} else {
			dir = bT & 0x0f;
		}
		m_adir[idir] = dir;
	}

	// Done

	return pstm->IsSuccess();
}

bool Path::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerPathState);
	pstm->WriteWord(m_cdirs);
	pstm->WriteWord(m_txStart);
	pstm->WriteWord(m_tyStart);
	pstm->WriteWord(m_idirCache);
	pstm->WriteWord(m_txCache);
	pstm->WriteWord(m_tyCache);

	// 2 directions per byte, smaller encoding

	byte bT = 0;
	for (int idir = 0; idir < m_cdirs; idir++) {
		Direction dir = m_adir[idir];
		if ((idir & 1) == 0) {
			bT = dir << 4;
			if (idir == m_cdirs - 1)
				pstm->WriteByte(bT);
		} else {
			bT |= dir;
			pstm->WriteByte(bT);
		}
	}

	// Done

	return pstm->IsSuccess();
}

#ifdef DRAW_PATHS
void Path::Draw(DibBitmap *pbm, int xView, int yView, Side side)
{
	TCoord txOld = m_txStart;
	TCoord tyOld = m_tyStart;

	for (int idir = 0; idir < m_cdirs; idir++) {
		Direction dir = m_adir[idir];
		TCoord txNew = txOld + g_mpDirToDx[dir];
		TCoord tyNew = tyOld + g_mpDirToDy[dir];
		DrawArrow(pbm, PcFromTc(txOld) - xView, PcFromTc(tyOld) - yView, dir, side);
		txOld = txNew;
		tyOld = tyNew;
	}

	// Draw 'X' at destination

	DrawArrow(pbm, PcFromTc(txOld) - xView, PcFromTc(tyOld) - yView, 8, side);
}
#endif

Direction DirectionFromLocations(TCoord txOld, TCoord tyOld, TCoord txNew, TCoord tyNew)
{
	Assert(abs(txOld - txNew) <= 1 && abs(tyOld - tyNew) <= 1);

	Direction dir;
	if (txNew < txOld) {
		if (tyNew < tyOld) {
			dir = kdirNW;
		} else if (tyNew > tyOld) {
			dir = kdirSW;
		} else {
			dir = kdirW;
		}
	} else if (txNew > txOld) {
		if (tyNew < tyOld) {
			dir = kdirNE;
		} else if (tyNew > tyOld) {
			dir = kdirSE;
		} else {
			dir = kdirE;
		}
	} else {
		if (tyNew < tyOld) {
			dir = kdirN;
		} else if (tyNew > tyOld) {
			dir = kdirS;
		} else {
			Assert(false);
			dir = kdirInvalid;
		}
	}

	return dir;
}

Direction16 Direction16FromLocations(TCoord txOld, TCoord tyOld, TCoord txNew, TCoord tyNew) {
    return DirectionFromLocations(txOld, tyOld, txNew, tyNew) * 2;
}

//
// TrackPoint class
//

bool TrackPoint::Init(Path *ppath, TCoord txFrom, TCoord tyFrom, int itptStart, int ctptFurtherStop)
{
	// Create a tracking point from txFrom, tyFrom to the path

	m_itptClosest = ppath->FindClosestPoint(txFrom, tyFrom, itptStart, ctptFurtherStop, &m_tptClosest);
	if (m_itptClosest == -1)
		return false;
	
	// Get the previous and next locations

	if (!ppath->GetPoint(m_itptClosest - 1, &m_tptBefore, 0))
		m_tptBefore = m_tptClosest;
	if (!ppath->GetPoint(m_itptClosest + 1, &m_tptAfter, 0))
		m_tptAfter = m_tptClosest;

	// Measure current distances to these locations. We'll measure
	// progress against these

	m_n2Before = ganSquared[abs(txFrom - m_tptBefore.tx)] + ganSquared[abs(tyFrom - m_tptBefore.ty)];
	m_n2After = ganSquared[abs(txFrom - m_tptAfter.tx)] + ganSquared[abs(tyFrom - m_tptAfter.ty)];
	m_tptInitial.tx = txFrom;
	m_tptInitial.ty = tyFrom;
	return true;
}

void TrackPoint::InitFrom(TrackPoint *ptrkp)
{
	m_tptInitial = ptrkp->m_tptInitial;
	m_tptClosest = ptrkp->m_tptClosest;
	m_tptBefore = ptrkp->m_tptBefore;
	m_tptAfter = ptrkp->m_tptAfter;
	m_itptClosest = ptrkp->m_itptClosest;
	m_n2Before = ptrkp->m_n2Before;
	m_n2After = ptrkp->m_n2After;
}

bool TrackPoint::IsProgress(TrackPoint *ptrkpA)
{
	// Returns true if this is closer than ptrkp
	// First compare closest path point. Longer is better "progress"

	TrackPoint *ptrkpB = this;
	if (ptrkpA->m_itptClosest < ptrkpB->m_itptClosest)
		return false;
	if (ptrkpA->m_itptClosest > ptrkpB->m_itptClosest)
		return true;		

	// Either same distance to before and closer to after, or further from before and same or greater to
	// after or closer to both before and after are recognized as "progress".

	if ((ptrkpA->m_n2Before == ptrkpB->m_n2Before && ptrkpA->m_n2After < ptrkpB->m_n2After) ||
			(ptrkpA->m_n2Before > ptrkpB->m_n2Before && ptrkpA->m_n2After <= ptrkpB->m_n2After) ||
			(ptrkpA->m_n2Before < ptrkpB->m_n2Before && ptrkpA->m_n2After < ptrkpB->m_n2After)) {
		return true;
	}
	return false;
}

bool TrackPoint::IsCloser(TrackPoint *ptrkpA)
{
	// Is ptrkpA closer to the unit path than ptrkpB? If so, it is closer. If same distance,
	// measure progress.

	TrackPoint *ptrkpB = this;
	if (ptrkpA->m_n2After < ptrkpB->m_n2After)
		return true;
	if (ptrkpA->m_n2After == ptrkpB->m_n2After)
		return IsProgress(ptrkpA);
	return false;
}

bool TrackPoint::IsBetterSort(TrackPoint *ptrkpA)
{
	return IsProgress(ptrkpA);
#if 0
	TrackPoint *ptrkpB = this;
	if (ptrkpA->m_n2After < ptrkpB->m_n2After)
		return true;
	if (ptrkpA->m_n2After == ptrkpB->m_n2After) {
		if (ptrkpA->m_n2Before > ptrkpB->m_n2Before)
			return true;
	}
	return false;
#endif
}

} // namespace wi
