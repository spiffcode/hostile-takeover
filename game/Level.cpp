#include "ht.h"

namespace wi {

Level::Level()
{
	m_fInitialized = false;
	m_pfogm = NULL;
	m_ptrmap = NULL;
	m_ptmap = NULL;
	m_nPlayersMin = 1;
	m_nPlayersMax = 1;
	m_szTitle[0] = 0;
	m_szFileLevel[0] = 0;
	m_nVersion = 0;
	m_dwRevision = 0;
}

Level::~Level()
{
	if (m_fInitialized) {
		// Reset gobm state

		ggobm.Reset();

		// Clear replicator points

		ReplicatorGob::ClearReplicatorCount();

		delete m_ptmap;
		delete m_pfogm;
		delete m_ptrmap;

		// Free up any cached paths

		MobileUnitGob::FreeCachedPaths();
	}
}

bool Level::LoadLevelInfo(const char *pszLevelName, IniReader *piniLoaded)
{
	IniReader *pini;

	if (piniLoaded == NULL) {
		pini = LoadIniFile(gpakr, pszLevelName);
		if (pini == NULL) {
			Assert(false);
			return false;
		}
	} else {
		pini = piniLoaded;
	}

	// Get level version

	if (pini->GetPropertyValue("General", "Version", "%d", &m_nVersion) == 0)
		m_nVersion = 0;

	// Get revision #

	if (pini->GetPropertyValue("General", "Revision", "%d", &m_dwRevision) == 0)
		m_dwRevision = 0;

	// Can't run if the level version is newer than what the game supports

	bool fSuccess = true;
	if (m_nVersion > knVersionLevelSupported) {
		fSuccess = false;
		HtMessageBox(kfMbWhiteBorder, "Error", "Newer version of Hostile Takeover required to run this mission.");
	} else {
		// Load General level data

		if (!pini->GetPropertyValue("General", "Title", m_szTitle, sizeof(m_szTitle)))
			strcpy(m_szTitle, "<untitled>");

        m_nPlayersMin = m_nPlayersMax = 0;
		pini->GetPropertyValue("General", "MinPlayers", "%d", &m_nPlayersMin);
		pini->GetPropertyValue("General", "MaxPlayers", "%d", &m_nPlayersMax);

        // Normalize
        if (pszLevelName[0] == 'm' && pszLevelName[1] == '_') {
            if (m_nPlayersMin < 2) {
                m_nPlayersMin = 2;
            }
            if (m_nPlayersMax > 4) {
                m_nPlayersMax = 4;
            }
            if (m_nPlayersMax < m_nPlayersMin) {
                m_nPlayersMax = m_nPlayersMin;
            }
        } else {
            m_nPlayersMin = 1;
            m_nPlayersMax = 1;
        }

		// Read in the side info for each side

		Side side;
		for (side = ksideNeutral; side < kcSides; side++) {
			if (side == ksideNeutral) {
				LoadSideInfo(pini, "sideNeutral", &m_asidi[side]);
			} else {
				char szSideName[] = "side1";
				szSideName[4] = '0' + side;
				LoadSideInfo(pini, szSideName, &m_asidi[side]);
			}
		}

		// If there is no human side, assume it is side 1
		// because gpplrLocal has to be set to something (because our code is intolerant)

		bool fHumanFound = false;
		for (side = ksideNeutral; side < kcSides; side++) {
			if (m_asidi[side].nIntelligence == knIntelligenceHuman)
				fHumanFound = true;
		}
		if (!fHumanFound)
			m_asidi[kside1].nIntelligence = knIntelligenceHuman;
	}

	if (piniLoaded == NULL)
		delete pini;

	return fSuccess;
}

bool Level::LoadSideInfo(IniReader *pini, char *pszSideName, SideInfo *psidi)
{
	memset(psidi, 0, sizeof(*psidi));

	// Include some good defaults in case this level contains no SideInfo for this side

	psidi->nInitialCredits = 5000;
	pini->GetPropertyValue(pszSideName, "InitialCredits", "%d", &psidi->nInitialCredits);

	int tx = 0;
	int ty = 0;
	pini->GetPropertyValue(pszSideName, "InitialView", "%d,%d", &tx, &ty);
	psidi->wptInitialView.wx = WcFromTc(tx);
	psidi->wptInitialView.wy = WcFromTc(ty);
	psidi->nIntelligence = knIntelligenceComputer;
	pini->GetPropertyValue(pszSideName, "Intelligence", "%d", &psidi->nIntelligence);
	pini->GetPropertyValue(pszSideName, "InitialStructureCount", "%d", &psidi->cStructuresInitial);
	pini->GetPropertyValue(pszSideName, "InitialMobileUnitCount", "%d", &psidi->cMobileUnitsInitial);
	return true;
}

bool Level::Init(const char *pszLevel, bool fConstantsOnly)
{
	// Save level filename

	strncpyz(m_szFileLevel, (char *)pszLevel, sizeof(m_szFileLevel));

	// Init reads all useful level data and then deletes the IniReader
	// to save memory.

	Status("Level::Init LoadIniFile...");
	IniReader *pini = LoadIniFile(gpakr, pszLevel);
	if (pini == NULL) {
		return false;
	}

	if (!LoadLevelConstants(pszLevel, pini)) {
		delete pini;
		return false;
	}

	// Establish unit count deltas

	ggame.CalcUnitCountDeltas(this);

	// Load variables

	if (!fConstantsOnly) {
		if (!LoadLevelVariables(pini)) {
			Assert(false);
			delete pini;
			return false;
		}
	}
	delete pini;

	return true;
}

bool Level::LoadLevelConstants(const char *pszLevelName, IniReader *pini)
{
	if (!LoadLevelInfo(pszLevelName, pini))
		return false;

	// m_fInitialized is to distinguish between just having the level parameters
	// read and having been initialized enough to require some cleanup.

	m_fInitialized = true;

	// Load map

	Status("Load Tile Map...");
	char szT[kcbFilename];
	if (!pini->GetPropertyValue("General", "TileMap", szT, sizeof(szT))) {
		Assert(false);
		return false;
	}

	Size sizPlayfield;
	ggame.GetPlayfieldSize(&sizPlayfield);
	m_ptmap = LoadTileMap(szT, &sizPlayfield);
	Assert(m_ptmap != NULL);
	if (m_ptmap == NULL) {
		Assert(false);
		return false;
	}

	// Init GobMgr

	Size sizMap;
	m_ptmap->GetMapSize(&sizMap);
	Size sizTile;
	m_ptmap->GetTileSize(&sizTile);
	if (!ggobm.Init(sizMap.cx / sizTile.cx, sizMap.cy / sizTile.cy, kcpgobMax)) {
		Assert(false);
		return false;
	}

	// Load terrain map

	Status("Load Terrain Map...");
	if (!pini->GetPropertyValue("General", "TerrainMap", szT, sizeof(szT))) {
		Assert(false);
		return false;
	}
	m_ptrmap = new TerrainMap;
	if (m_ptrmap == NULL) {
		Assert(false);
		return false;
	}
	if (!m_ptrmap->Init(szT)) {
		Assert(false);
		return false;
	}

	// Create fog map.

	Status("Create Fog Map...");
	m_pfogm = new FogMap;
	if (m_pfogm == NULL) {
		Assert(false);
		return false;
	}
	if (!m_pfogm->Init(&sizTile, &sizMap)) {
		Assert(false);
		return false;
	}

	// Instantiate an OvermindGob for each Computer Player

	Player *pplr = gplrm.GetNextPlayer(NULL);
	for (; pplr != NULL; pplr = gplrm.GetNextPlayer(pplr)) {
#ifdef STRESS
		// Instantiate an Overmind for the human player if we're running stress

		if (gfStress) {
			if ((pplr->GetFlags() & (kfPlrComputer | kfPlrComputerOvermind)) == kfPlrComputer)
				continue;
		} else {
			if (!(pplr->GetFlags() & kfPlrComputerOvermind))
				continue;
		}
#else
		if (!(pplr->GetFlags() & kfPlrComputerOvermind))
			continue;
#endif

		// Instantiate and initialize an OvermindGob

		OvermindGob *pgobOvermind = (OvermindGob *)CreateGob(kgtOvermind);
		if (pgobOvermind == NULL) {
			Assert(false);
			delete pini;
			return false;
		}

		if (!pgobOvermind->Init(NULL)) {
			Assert(false);
			delete pgobOvermind;
			delete pini;
			return false;
		}

		// Who's your daddy?

		pgobOvermind->SetOwner(pplr);
	}

	// Load the Triggers

	Status("Load Triggers...");
	if (!m_tgrm.Init(pini)) {
		Assert(false);
		return false;
	}

	// Load the UnitGroups

	Status("Load UnitGroups...");
	if (!m_ugm.Init(pini)) {
		Assert(false);
		return false;
	}

	return true;
}	

bool Level::LoadLevelVariables(IniReader *pini)
{
	// Load areas

	Status("Load Areas...");
	if (!ggobm.LoadAreas(pini)) {
		return false;
	}

	// Enumerate all Gob descriptions and instantiate in-memory versions

	Status("Load Gobs...");
	char szName[kcbFilename];
	FindProp find;
	while (pini->FindNextProperty(&find, "GameObjects", szName, sizeof(szName))) {
		GobType gt;
		pini->GetPropertyValue(&find, "%d,", &gt);

		Gob *pgob = CreateGob(gt);
		if (pgob == NULL) {
			Assert(false);
			return false;
		}

		// Don't waste space on Gobs that don't need names

		char *pszName;
		if (strcmp("nil", szName) == 0)
			pszName = NULL;
		else
			pszName = szName;

		if (!pgob->Init(pini, &find, pszName)) {
			Assert(false);
			delete pgob;
			return false;
		}

#ifdef STRESS
		if (gfStress) {
			// Make player's units invulnerable

			if (pgob->GetOwner() == gpplrLocal) {
				if (pgob->GetFlags() & kfGobUnit) {
					UnitGob *punt = (UnitGob *)pgob;
					punt->SetUnitFlags(punt->GetUnitFlags() | kfUnitInvulnerable);
				}
			}
		}
#endif
	}

	// Enumerate all Galaxite positions and add them to the Fog/Galaxite map

	Status("Load Galaxite...");
	FindProp find2;
	while (pini->FindNextProperty(&find2, "Galaxite", szName, sizeof(szName))) {
		int nGx, tx, ty;
		pini->GetPropertyValue(&find2, "%d,%d,%d", &nGx, &tx, &ty);
		m_pfogm->SetGalaxite(nGx, tx, ty);
	}

// In terrain for the moment (forever?)
#if 0
	// Enumerate all Wall positions and add them to the Fog/Galaxite/Wall map

	Status("Load walls...");
	FindProp find3;
	while (pini->FindNextProperty(&find3, "Walls", szName, sizeof(szName))) {
		int nHealth, tx, ty;
		pini->GetPropertyValue(&find3, "%d,%d,%d", &nHealth, &tx, &ty);
		m_pfogm->SetWallHealth(nHealth, tx, ty);
	}
#endif

	// Instantiate the "CreateAtLevelLoad" UnitGroups

	m_ugm.CreateAtLevelLoadGroups();

	return true;
}

#define knVerLevelState 2
bool Level::LoadState(Stream *pstm)
{
	// Do version handling

	byte nVer = pstm->ReadByte();
	if (nVer != knVerLevelState)
		return false;

	// Get level name

	char szLevel[kcbFilename];
	pstm->ReadString(szLevel, sizeof(szLevel));

	// Load level constants

	if (!Init(szLevel, true)) {
		return false;
	}

	// Check to see if the save game has a different mission revision number than the mission
	// itself. If so, error. This is the case if a game is saved, then the level is revised
	// in M and reloaded.

	dword dwRevision = pstm->ReadDword();
	if (dwRevision != m_dwRevision) {
		HtMessageBox(kfMbWhiteBorder | kfMbClearDib, "Error", "This saved game is based on an older version of this mission!");
		return false;
	}

	// Load gobm state

	if (!ggobm.LoadState(pstm))
		return false;

	// Load fog and galaxite state

	if (!m_pfogm->LoadState(pstm)) {
		return false;
	}
	if (!m_ptrmap->LoadState(pstm)) {
		return false;
	}

	// Load gobs

	int cGobs = pstm->ReadWord();
	while (cGobs-- != 0) {
		// Get gob type

		GobType gt = pstm->ReadByte();
		Gob *pgob = CreateGob(gt);
		if (pgob == NULL) {
			return false;
		}

		if (!pgob->LoadState(pstm)) {
			return false;
		}
	}

	// Load triggers

	if (!m_tgrm.LoadState(pstm)) {
		return false;
	}

	// Load UnitGroups

	if (!m_ugm.LoadState(pstm)) {
		return false;
	}

	return pstm->IsSuccess();
}

bool Level::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerLevelState);
	pstm->WriteString(m_szFileLevel);
	pstm->WriteDword(m_dwRevision);
	ggobm.SaveState(pstm);
	m_pfogm->SaveState(pstm);
	m_ptrmap->SaveState(pstm);

	int cgobs = 0;
	Gob *pgobT;
	for (pgobT = ggobm.GetFirstGob(); pgobT != NULL; pgobT = ggobm.GetNextGob(pgobT)) {
		if (pgobT->IsSavable())
			cgobs++;
	}
	pstm->WriteWord(cgobs);

	for (pgobT = ggobm.GetFirstGob(); pgobT != NULL; pgobT = ggobm.GetNextGob(pgobT)) {
		if (pgobT->IsSavable()) {
			pstm->WriteByte(pgobT->GetType());
			pgobT->SaveState(pstm);
		}
	}

	// Save triggers

	m_tgrm.SaveState(pstm);

	// Save UnitGroups

	m_ugm.SaveState(pstm);

	return pstm->IsSuccess();
}

} // namespace wi
