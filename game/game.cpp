#include "game/ht.h"
#include "game/wistrings.h"
#include "mpshared/netmessage.h"
#include "game/httppackmanager.h"
#include "game/completemanager.h"
#include "game/chatter.h"

#if 0
//temp
#include "base/bytebuffer.h"
#include "game/serviceurls.h"
#include "game/uploader.h"
#include "game/httpservice.h"
#endif

namespace wi {

Display *gpdisp;
Game ggame;
Simulation gsim;
TimerMgr gtimm;
IniReader *gpiniForms;
IniReader *gpiniGame;
Font *gapfnt[kcFonts];
GobStateMachineMgr gsmm;
GobMgr ggobm;
byte *gpbScratch;
word gcbScratch;
int gimmReinitialize = -1;
bool gfLoadReinitializeSave = false;
int gcxTile;
int gcyTile;
int gcxFullScreenFormDiv10;
int gcxyBorder;
bool gfGrayscale = false;
#ifdef DRAW_UPDATERECTS
bool gfDrawUpdateRects;
#endif
short *gmpPcFromWc;
WCoord *gmpWcFromPc;
DibBitmap *gpbmClip;
FormMgr *gpfrmmSim;
FormMgr *gpfrmmInput;
MultiFormMgr *gpmfrmm;
int gnHueOffset = 0;
int gnSatMultiplier = 0;
int gnLumOffset = 0;
int gnDemoRank = 0;
float gnScrollSpeed = 1.0;
char gszAskURL[512];
char gszDeviceId[34];
Stream *gpstmSavedGame;
UpdateMap *gpupdSim;
int gtGameSpeed = kcmsUpdate / 10;	// 80 ms/frame or 12.5 FPS (1000/80)
Color *gaclrFixed;
bool gfClearFog;
word gwfPerfOptions = kfPerfAll;
word gwfHandicap = kfHcapDefault;
int gcStructGobsHumanLimitSP;
int gcStructGobsComputerLimitSP;
int gcMuntGobsHumanLimitSP;
int gcMuntGobsComputerLimitSP;
int gcStructGobsComputerDeltaSP;
int gcStructGobsHumanDeltaSP;
int gcStructGobsLimitMP;
int gcMuntGobsLimitMP;
int gcSceneryGobsLimit;
int gcScorchGobsLimit;
int gcSupportGobsLimit;
bool gfMultiplayer;
bool gfIgnoreBluetoothWarning;
SpriteManager *gpsprm;
TexAtlasMgr *gptam;
int gcmsDisplayUpdate = 8; // mimimum ms to elapse between paints

#ifdef STRESS
bool gfStress = false;
long gtStressTimeout;
#endif

#ifndef PIL
#if (defined(IPHONE) || defined(SDL)) && (!defined(DEV_BUILD) && !defined(BETA_TIMEOUT))
char *gszVersion = "1.7";
#else
char *gszVersion = "+++VERSION+++";
#endif
#endif

// Forward declarations

bool AllocComputerPlayers() secGame;
void FreeComputerPlayers() secGame;

//
//
//

void GameMain(char *pszCmds)
{
#if defined(DEBUG) && !defined(CE) && !defined(PIL) && !defined(IPHONE) && !defined(SDL)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	char *pszLevel = NULL;
	char szLevel[kcbFilename];
#ifdef WIN
	char szData[_MAX_PATH];
#endif

	// Parse command line

	int nchAbs = 0;
	int nchRel;
	int imm = -1;
	while (true) {
		char szT[80];
		int c = IniScanf(&pszCmds[nchAbs], "%s %+", szT, &nchRel);
		nchAbs += nchRel;
		if (c == 0)
			break;

		// -m %d (mode / data match: index)

		if (strcmp(szT, "-m") == 0) {
			c = IniScanf(&pszCmds[nchAbs], "%d%+", &imm, &nchRel);
			nchAbs += nchRel;
			if (c == 0)
				return;

		// -l %s (level: level.lvl)

		} else if (strcmp(szT, "-l") == 0) {
			c = IniScanf(&pszCmds[nchAbs], "%s%+", szLevel, &nchRel);
			nchAbs += nchRel;
			if (c == 0)
				return;
			pszLevel = szLevel;
#ifdef STRESS
		// -s

		} else if (strcmp(szT, "-s") == 0) {
			gfStress = true;
#endif
		} else if (strcmp(szT, "-nofog") == 0) {
			gfClearFog = true;

#if defined(MP_STRESS) && !defined(PIL) && !defined(CE) && !defined(IPHONE)
		// -mp-spawn <count>
		// TODO: -mp-spawn <client|server> side arg_string

		} else if (strcmp(szT, "-mp-spawn") == 0) {
			int cSpawn;
			c = IniScanf(&pszCmds[nchAbs], "%d %+", &cSpawn, &nchRel);
			nchAbs += nchRel;
			if (c != 2)
				return;

			for (int nSpawn = 0; nSpawn < cSpawn; nSpawn++) {
				char szModule[MAX_PATH];
				GetModuleFileName(NULL, szModule, sizeof(szModule) - 1);
				char szCmdLine[MAX_PATH];
				sprintf(szCmdLine, "%s -mp-start %d", szModule, nSpawn + 1);
				WinExec(szCmdLine, SW_SHOW);
			}

		// -mp-start <pos>
		// TODO: -mp-start <client|server> side arg_string

		} else if (strcmp(szT, "-mp-start") == 0) {
			int nSpawn;
			c = IniScanf(&pszCmds[nchAbs], "%d %+", &nSpawn, &nchRel);
			nchAbs += nchRel;
			if (c != 2)
				return;

			gfMPStress = true;
			gnMPPos = nSpawn;
#endif
		}
	}

#ifdef DEBUG_HELPERS
	InitDebugHelpers();
#endif

    if (!HostInit()) {
        return;
    }

  	while (true) {
		if (!ggame.Init(imm)) {
			ggame.Exit();
			return;
		}

#if 0
//for testing sync error uploading
        //temp
        base::ByteBuffer *bb = new base::ByteBuffer();
        bb->WriteString("hello world");
        const char *url = base::Format::ToString("%s?gameid=1&pid=2",
                kszSyncErrorUploadUrl);
        UploadByteBuffer(gphttp, bb, url);
        ggame.Exit();
        return;
#endif

		if (gfLoadReinitializeSave) {
			gfLoadReinitializeSave = false;
			gshl.Launch(true);
		} else {
			gshl.Launch(false);
		}

		if (gimmReinitialize != -1) {
			imm = gimmReinitialize;
			gimmReinitialize = -1;
			ggame.Exit();
			continue;
		}
		break;
	}

	// Demo mode only nag screen
    // Will mean you'll need to press the applications silkscreen button twice
    // in order to exit from demo

	if (gfDemo) {
		gevm.ClearAppStopping();
		DoRegisterNowForm();
	}

	ggame.Exit();

    HostExit();

#ifdef DEBUG_HELPERS
	ExitDebugHelpers();
#endif

#if defined(CHECK_OLD_ALLOCS) && defined(DEBUG) && !defined(CE) && !defined(PIL)
	_CrtDumpMemoryLeaks();
#endif
}

Game::Game()
{
	m_wf = 0;
	m_pfrmSimUI = NULL;
	m_fSimUninitialized = true;
	m_cmm = 0;
	m_cmmAlloc = 0;
	m_immCurrent = -1;
	m_immBest = -1;
	m_szNextLevel[0] = 0;
    m_fSkipSaveReinitialize = false;
    m_fUpdateTriggers = false;
    m_gameid = 0;
}

Game::~Game()
{
	Assert(m_pfrmSimUI == NULL);
}

bool Game::Init(int imm)
{
	// Haven't initialized yet

	m_wf &= ~kfGameInitDone;

	// Check Dynamic Memory size

	if (!CheckMemoryAvailable())
		return false;

	// Load prefs first

    if (!LoadPreferences())
        return false;

    // Suck what we can from preferences

    gtGameSpeed = gpprefs->GetInteger(knPrefGameSpeed);
    gfLassoSelection = gpprefs->GetBool(kfPrefLassoSelection);
    gnHueOffset = gpprefs->GetInteger(knPrefHueOffset);
	gnSatMultiplier = gpprefs->GetInteger(knPrefSatMultiplier);
	gnLumOffset = gpprefs->GetInteger(knPrefLumOffset);
    strncpy(gszUsername, gpprefs->GetString(kszPrefUsername), sizeof(gszUsername));
    strncpy(gszPassword, gpprefs->GetString(kszPrefPassword), sizeof(gszPassword));
    strncpy(gszToken, gpprefs->GetString(kszPrefToken), sizeof(gszToken));
    gfAnonymous = gpprefs->GetBool(kfPrefAnonymous);
    gwfPerfOptions = (word)gpprefs->GetInteger(kwfPrefPerfOptions);
    gwfHandicap = (word)gpprefs->GetInteger(kwfPrefHandicap);
    gnDemoRank = gpprefs->GetInteger(knPrefDemoRank);
    gnScrollSpeed = gpprefs->GetFloat(knPrefScrollSpeed);
    // gfIgnoreBluetoothWarning = gpprefs->GetBool(kfPrefIgnoreBluetoothWarning);
    strncpy(gszAskURL, gpprefs->GetString(kszPrefAskUrl), sizeof(gszAskURL));
    strncpy(gszDeviceId, gpprefs->GetString(kszPrefDeviceId), sizeof(gszDeviceId));
    gcmsDisplayUpdate = gpprefs->GetInteger(knPrefUpdateDisplay);

	// Temp buffer used for several things, including decompression, TBitmap compiling.

	gcbScratch = 10 * 1024;
	gpbScratch = new byte[gcbScratch];
	if (gpbScratch == NULL)
		return false;

	// Requires gpbScratch to work

#if defined(PIL) && defined(TRACE_TO_DB_LOG)
	DbLogInit();
#endif

	// Init event manager

	gevm.Init();

	// Match display with data

	if (!InitDisplay(imm)) {
        HostOutputDebugString("Display didn't initialize.");
		return false;
    }

	if (!LoadGameData()) {
        HostOutputDebugString("Game data didn't load.");
		return false;
    }

    if (!InitTexAtlasMgr()) {
        HostOutputDebugString("Failed to initilize texture atlas manager");
    }

	// Init memory manager *after* setting screen mode so we know how much dyn ram is left after
	// allocating back buffers (potentially from main memory).

	if (!InitMemMgr()) {
        HostOutputDebugString("Memory manager didn't initialize.");
		return false;
    }

	// Init cache mgr

	if (!gcam.Init()) {
        HostOutputDebugString("Cache manager didn't initialize.");
		return false;
    }

	// Init mapping tables after mem mgr since it uses mem mgr to store the tables

	if (!ggame.InitCoordMappingTables()) {
		HostMessageBox(TEXT("Out of memory, could not initialize game!"));
		return false;
	}

	// Init MultiFormMgr

	if (!InitMultiFormMgr()) {
        HostOutputDebugString("Multi form manager didn't initialize.");
		return false;
    }

	// Initialize the Shell

	if (!gshl.Init()) {
        HostOutputDebugString("Shell didn't initialize.");
		return false;
    }

	// Init form / control requirements

	ButtonControl::InitClass();
	CheckBoxControl::InitClass();
	PipMeterControl::InitClass();
	ListControl::InitClass();
	DamageMeterControl::InitClass();

	// Save around game.ini

	gpiniGame = LoadIniFile(gpakr, "game.ini");
	if (gpiniGame == NULL)
		return false;

	// Save around the form.ini

	gpiniForms = LoadIniFile(gpakr, "forms.ini");
	if (gpiniForms == NULL)
		return false;

	// Load fonts

	for (int ifnt = 0; ifnt < kcFonts; ifnt++) {
		char szProp[2];
		szProp[0] = ifnt + '0';
		szProp[1] = 0;
		char szT[kcbFilename];
		if (gpiniGame->GetPropertyValue("Fonts", szProp, szT, sizeof(szT))) {
			gapfnt[ifnt] = LoadFont(szT);
			if (gapfnt[ifnt] == NULL) {
				Assert("LoadFont failed");
				return false;
			}
		}
	}
	m_wf |= kfGameInitFonts;

	// Load string table

	gpstrtbl = new StringTable();
	Assert(gpstrtbl != NULL, "out of memory!");
	if (gpstrtbl == NULL)
		return false;
	if (!gpstrtbl->Init("strings.bin"))
		return false;

	// Init sound system

	gsndm.Init();
	if (gpprefs->GetInteger(knPrefVolume) != (word)-1)
		gsndm.SetVolume(gpprefs->GetInteger(knPrefVolume));
    gsndm.Enable(!gpprefs->GetBool(kfPrefSoundMuted));

	// Clear out stale save games

	DeleteStaleSaveGames();

	// We're done with game initialization

	m_wf |= kfGameInitDone;

	return true;
}

bool Game::CheckMemoryAvailable()
{
	// The following constants have been calculated based on actual device measurements,
	// scenario tests and gob count partitioning as of 8/16/03. Actual data is in \ht\docs\size.xls.
	// The "min" scenario is the minimum playable scenario. If kcbDynMemMin is available, the following
	// are the unit counts. If "max" scenario is the maximum scenario. If kcbDynMemMax is available, the
	// following are the unit counts. This is partitioned by the gob count maximum and not the memory
	// available (once over kcbDynMemMax it's gravy). The mem sizes are what is available at this
	// point in time: PalmOS allocs globals and calls PilotMain.
	// Note also the computer unit counts are all shared between all computer sides.
	// Note that some of the computer units counts in certain levels may already be exceeded; that is ok.

// TODO: All gob limits should be validated in M.

// Non-unit gob limits.

#define kcSceneryGobsLimit 100
#define kcScorchGobsLimit 30
#define kcSupportGobsLimit 50

// min limits
#define kcbDynMemMin 130000
#define kcStructGobsHumanMinSP 39
#define kcStructGobsComputerMinSP 52
#define kcMuntGobsHumanMinSP 60
#define kcMuntGobsComputerMinSP 80

// med limits
#define kcbDynMemMed 140000
#define kcStructGobsHumanMedSP 49
#define kcStructGobsComputerMedSP 65
#define kcMuntGobsHumanMedSP 79
#define kcMuntGobsComputerMedSP 105

// max limits
#define kcbDynMemMax 150000
#define kcStructGobsHumanMaxSP 55
#define kcMuntGobsHumanMaxSP 88
#define kcStructGobsComputerMaxSP 72
#define kcMuntGobsComputerMaxSP 117

#if (kcSceneryGobsLimit+kcScorchGobsLimit+kcSupportGobsLimit+kcStructGobsHumanMaxSP+kcStructGobsComputerMaxSP+kcMuntGobsHumanMaxSP+kcMuntGobsComputerMaxSP > kcpgobMax)
#error Gob count limit error!
#endif

// mp limits

#define kcStructGobsMaxMP 55 // 33
#define kcMuntGobsMaxMP 88 // 66

#ifndef __CPU_68K
#if (kcSceneryGobsLimit+kcScorchGobsLimit+kcSupportGobsLimit+kcStructGobsMaxMP*4+kcMuntGobsMaxMP*4 > kcpgobMax)
#error Gob count limit error!
#endif
#endif

	// Assume max. Works for all platforms except lowest end Palms.

	gcStructGobsHumanLimitSP = kcStructGobsHumanMaxSP;
	gcStructGobsComputerLimitSP = kcStructGobsComputerMaxSP;
	gcMuntGobsHumanLimitSP = kcMuntGobsHumanMaxSP;
	gcMuntGobsComputerLimitSP = kcMuntGobsComputerMaxSP;
	gcStructGobsLimitMP = kcStructGobsMaxMP;
	gcMuntGobsLimitMP = kcMuntGobsMaxMP;
	gcSceneryGobsLimit = kcSceneryGobsLimit;
	gcScorchGobsLimit = kcScorchGobsLimit;
	gcSupportGobsLimit = kcSupportGobsLimit;

#ifdef PIL

#if 0
	// Limit test

	gcStructGobsHumanLimitSP = kcStructGobsHumanMinSP;
	gcStructGobsComputerLimitSP = kcStructGobsComputerMinSP;
	gcMuntGobsHumanLimitSP = kcMuntGobsHumanMinSP;
	gcMuntGobsComputerLimitSP = kcMuntGobsComputerMinSP;

#else

	// Max scenario?
	UInt32 cbFree, cbMax;
	MemHeapFreeBytes(0, &cbFree, &cbMax);
	if (cbFree < kcbDynMemMax) {
		if (cbFree >= kcbDynMemMed) {
			gcStructGobsHumanLimitSP = kcStructGobsHumanMedSP;
			gcStructGobsComputerLimitSP = kcStructGobsComputerMedSP;
			gcMuntGobsHumanLimitSP = kcMuntGobsHumanMedSP;
			gcMuntGobsComputerLimitSP = kcMuntGobsComputerMedSP;
		}
		else if (cbFree >= kcbDynMemMin) {
			gcStructGobsHumanLimitSP = kcStructGobsHumanMinSP;
			gcStructGobsComputerLimitSP = kcStructGobsComputerMinSP;
			gcMuntGobsHumanLimitSP = kcMuntGobsHumanMinSP;
			gcMuntGobsComputerLimitSP = kcMuntGobsComputerMinSP;
		}
		else {
			HostNotEnoughMemory(false, cbFree, kcbDynMemMin);
			return false;
		}
	}
#endif
#endif

	return true;
}

void Game::CalcUnitCountDeltas(Level *plvl)
{
    // After the level is loaded this gets called. It calcs how many initial
    // units there are and recalcs unit deltas.
	//
    // NOTE: For now all this does is take the computer's extra structure count
    // and gives it to the human player.

	int cComputerStructuresInitial = 0;
	for (int side = ksideNeutral; side < kcSides; side++) {
		Player *pplr = gplrm.GetPlayer(side);
		if (pplr->GetFlags() & kfPlrComputer)
			cComputerStructuresInitial += plvl->GetSideInfo(side)->cStructuresInitial;
	}

	// HACK ALERT: Give the computer room to build 5 structures

	int cStructuresAvailable = gcStructGobsComputerLimitSP - cComputerStructuresInitial - 5;

	// Give the rest to the human player

	if (cStructuresAvailable > 0) {
		gcStructGobsComputerDeltaSP = -cStructuresAvailable;
		gcStructGobsHumanDeltaSP = cStructuresAvailable;
	} else {
		gcStructGobsComputerDeltaSP = 0;
		gcStructGobsHumanDeltaSP = 0;
	}
}

bool Game::LoadGameData()
{
	// We've choosen the best mode / data combo. Load the matching data

	Assert(m_immCurrent != -1);

    const char *pszMainDataDir = HostGetMainDataDir();

    // Game data may be a pdb

	char szPdb[20];
	sprintf(szPdb, "htdata%d%d.pdb", m_amm[m_immCurrent].nDepthData, m_amm[m_immCurrent].nSizeData);

    // Game data may be a directory

    char szDir[MAX_PATH];
    if (pszMainDataDir == NULL) {
        sprintf(szDir, "htdata%d%d", m_amm[m_immCurrent].nDepthData, m_amm[m_immCurrent].nSizeData);
    } else {
        sprintf(szDir, "%s/htdata%d%d", pszMainDataDir, m_amm[m_immCurrent].nDepthData,
            m_amm[m_immCurrent].nSizeData);
    }

    // Pdb is first choice, dir is second

	if (!gpakr.Push(pszMainDataDir, szPdb))
        if (!gpakr.Push(szDir, NULL))
            return false;

	// Load sound effects

	if (!gpakr.Push(HostGetMainDataDir(), "htsfx.pdb")){
		HostMessageBox(TEXT("Required sound effects file missing"));
		return false;
	}

#if 0
	// Enum and push addon data

	Enum enm;
	char szAddon[64];
    char szDir[1024];
	while (HostEnumAddonFiles(&enm, szDir, sizeof(szDir),
            szAddon, sizeof(szAddon))) {
		if (CheckDatabaseVersion(szDir, szAddon, true)) {
			gpakr.Push(szDir, szAddon);
        }
	}
#endif

	// Set global tile dimensions
	// Perform hack for size 20 which really reuses 24x24 tiles but sizes other graphics proportionally

	gcxTile = m_amm[m_immCurrent].nSizeData;
	if (gcxTile == 20)
		gcxTile = 24;
	gcyTile = gcxTile;

	// If the tile size is 24, then gpbScratch is ~2.25 times bigger to accomodate TBitmap compiling size
	// increase

	if (gcxTile > 16) {
		if (gcbScratch <= 10 * 1024) {
			delete[] gpbScratch;
            if (gcxTile == 32) {
                gcbScratch = 35 * 1024;
            } else {
                gcbScratch = 23 * 1024;
            }
			gpbScratch = new byte[gcbScratch];
			if (gpbScratch == NULL)
				return false;
		}
	}

	return true;
}

bool Game::InitTexAtlasMgr()
{
    gptam = new TexAtlasMgr();
    Assert(gptam != NULL, "out of memory!");
	if (gptam == NULL)
		return false;
	if (!gptam->Init()) {
		delete gptam;
		return false;
	}
    return true;
}

bool Game::InitCoordMappingTables()
{
	// Initialize world -> pixel coord mapping table

#define knSplit 32
//#define knSplit 4
	gmpPcFromWc = (short *)gmmgr.AllocPtr(2 * kwcMax);
	if (gmpPcFromWc == NULL)
		return false;

	Assert(kwcMax / knSplit <= gcbScratch);
	Assert((kwcMax / knSplit) * knSplit == kwcMax);

	short s = 0;
	short nOverflow = 0;
	for (int j = 0; j < knSplit; j++) {
		short *psT = (short *)gpbScratch;
		for (int i = 0; i < kwcMax / knSplit; i++) {
			// Duplicate the behavior of gmpWcFromPc so that
			// gmpPcFromWc[gmpWcFromPc[2]] == 2

			*psT++ = s;
			nOverflow += gcxTile;
			if (nOverflow == 256) {
				nOverflow = 0;
				s++;
			} else if (nOverflow > 256) {
				nOverflow -= 256;
			} else if (nOverflow + gcxTile - 1 >= 256) {
				s++;
			}
		}
		gmmgr.WritePtr(gmpPcFromWc, j * 2 * (kwcMax / knSplit), gpbScratch, 2 * (kwcMax / knSplit));
	}

	// Initialize pixel -> world coord mapping table

	WCoord *pwcT = (WCoord *)gpbScratch;
	for (int i = 0; i <= 64 * 32; i++) {
		*pwcT++ = (WCoord)((i * 256L) / gcxTile);
	}

	// These "+ 1"s are to support the mapping of inclusive/exclusive rects
	// that lay at the extreme of the allowed coordinate range

	gmpWcFromPc = (WCoord *)gmmgr.AllocPtr((2 * 64 * 32) + 1);
	if (gmpWcFromPc == NULL)
		return false;
	gmmgr.WritePtr(gmpWcFromPc, 0, gpbScratch, (2 * 64 * 32) + 1);

	return true;
}

bool Game::InitMemMgr()
{
	// 8/30/03 settings
	// There are 12 save game slots typically + 1 auto-save slot
	// Reserve 20K * 13 for save games (260K ideally)

	// For cache use: There is approximately 75K of fixed allocs
	// Hi-res runs smoothly at 1M cache with room to spare
	// Lo-res settings are (1M - 75K) * 16 / 24 (approximately)
	// Should grab more than enough space if possible to run smoothly

// How much to reserve from dynamic memory for dynamic allocations - the rest
// will be used by the storage memory manager. This is more than enough; we don't get
// anywhere close this much on low-end Palms.

#ifdef __CPU_68K
// Single player only
#define kcbDynReserve ((dword)384 * 1024)
#else
// Single and multi player
#define kcbDynReserve ((dword)512 * 1024)
#endif

#define kcbSaveGame ((dword)20 * 1024)
#define kcbMaxSaveGame ((dword)13 * kcbSaveGame)
#define kcbMinSaveGame ((dword)6 * kcbSaveGame)

// Fixed allocs from storage mem

#define kcbFixed ((dword)150 * 1024)

// Min / max needed for storage memory manager

#define kcbMinHires ((dword)512 * 1024)
#define kcbWarnHires ((dword)768 * 1024)
#define kcbMaxHires ((dword)1024 * 1024)

#define kcbMinLores ((kcbMinHires - kcbFixed) * 2 / 3 + kcbFixed)
#define kcbWarnLores ((kcbWarnHires - kcbFixed) * 2 / 3 + kcbFixed)
#define kcbMaxLores ((kcbMaxHires - kcbFixed) * 2 / 3 + kcbFixed)

	bool fHires = (m_amm[m_immCurrent].nSizeData > 16);
	dword cbMin, cbWarn, cbMax;
	if (fHires) {
		cbMin = kcbMinHires;
		cbWarn = kcbWarnHires;
		cbMax = kcbMaxHires;
	} else {
		cbMin = kcbMinLores;
		cbWarn = kcbWarnLores;
		cbMax = kcbMaxLores;
	}

	// How much storage memory is available?

	dword cbDyn;
	dword cbStorage;
	MemMgr::GetInitSize(kcbDynReserve, &cbDyn, &cbStorage);
	dword cbAvail = cbDyn + cbStorage;

	// Prioritize storage memory size over # of save games, but keep at least
	// a min save game size
	// Calc space to reserve for save games

	dword cbSaveGameReserve = kcbMinSaveGame;
	if (cbAvail > cbMax) {
		if (cbAvail - cbMax >= kcbMaxSaveGame)
			cbSaveGameReserve = kcbMaxSaveGame;
	}

	// Calc what is left over

	dword cbAlloc = 0;
	if (cbAvail > cbSaveGameReserve)
		cbAlloc = cbAvail - cbSaveGameReserve;

	// If there is space left over, take it up to cbMax * 1 1/2

	dword cbCeiling = cbMax * 3 / 2;
	if (cbAlloc > cbCeiling)
		cbAlloc = cbCeiling;

	// Error if not enough memory

	if (cbAlloc < cbMin) {
		HostNotEnoughMemory(true, cbStorage, cbMin + cbSaveGameReserve - cbDyn);
		return false;
	}

	// Warn if below threshold

	if (cbAlloc < cbWarn)
		HostMessageBox(TEXT("For better performance, increase the amount of memory available!"));

	// Now initialize

	dword cbTotal;
	gmmgr.Init(kcbDynReserve, cbAlloc, &cbTotal);

	return true;
}

#define kcmmGrow 32

void Game::AddModeMatches(int nDepthData, int nSizeData, int nDepthOrGreater, int cxWidthOrGreater)
{
    HostOutputDebugString("AddModeMatches %d, %d, %d, %d",
            nDepthData, nSizeData, nDepthOrGreater, cxWidthOrGreater);

	int cmodes = gpdisp->GetModeCount();
	for (int n = 0; n < cmodes; n++) {
		ModeInfo mode;
		gpdisp->GetModeInfo(n, &mode);
        HostOutputDebugString("mode %d cx %d cy %d nDepth %d nDegree %d",
                n, mode.cx, mode.cy, mode.nDepth, mode.nDegreeOrientation);
		if (mode.nDepth < nDepthOrGreater)
			continue;
		if (mode.cx < cxWidthOrGreater)
			continue;
		if (mode.cy < cxWidthOrGreater)
			continue;
		if (m_cmm == m_cmmAlloc) {
			ModeMatch *pmm = new ModeMatch[m_cmmAlloc + kcmmGrow];
			if (pmm == NULL)
				continue;
			if (m_amm != NULL) {
				memcpy(pmm, m_amm, sizeof(ModeMatch) * m_cmm);
				delete[] m_amm;
			}
			m_amm = pmm;
			m_cmmAlloc += kcmmGrow;
		}

		m_amm[m_cmm].imode = n;
		m_amm[m_cmm].nDepthData = nDepthData;
		m_amm[m_cmm].nSizeData = nSizeData;
		m_cmm++;
	}
}

#define kfMatchWidthBounded 1
#define kfMatchExactDepth 2
#define kfMatchNative 4

int Game::FindBestModeMatch2(int nDepthData, int nSizeData, int nDepthMode, int cxWidthModeMin, int cxWidthModeMax, byte bfMatch)
{
	// Also, the largest Y is returned at a given width

	int nBest = -1;
	int cyLargestLast = -1;
	int cxLargestLast = -1;
	int nDepthClosestLast = 0x7fff;
	for (int n = 0; n < m_cmm; n++) {
		// Is this match for the requested data depth / data size?

		ModeMatch *pmm = &m_amm[n];
		if (pmm->nDepthData != nDepthData || pmm->nSizeData != nSizeData)
			continue;

		// Native modes only?

		ModeInfo mode;
		gpdisp->GetModeInfo(pmm->imode, &mode);
		if (bfMatch & kfMatchNative) {
			if (!mode.fNative)
				continue;
		}

		// Is mode width compatible?

		if (bfMatch & kfMatchWidthBounded) {
			if (mode.cx < cxWidthModeMin || mode.cx > cxWidthModeMax)
				continue;
		} else {
			if (mode.cx < cxWidthModeMin)
				continue;
		}

		// Is depth compatible?

		if (bfMatch & kfMatchExactDepth) {
			// Must be exact depth

			if (mode.nDepth != nDepthMode)
				continue;
		} else {
			// Must be closest depth that is >= mode.nDepth

			if (mode.nDepth < nDepthMode)
				continue;
		}

		// Found a criteria match.
		// Remember closest depth

		if (mode.nDepth < nDepthClosestLast) {
			nDepthClosestLast = mode.nDepth;
			nBest = n;
		}

		// Remember largest width

		if (mode.cx > cxLargestLast) {
			cxLargestLast = mode.cx;
			nBest = n;
		}

		// Remember largest Y

		if (mode.cx == cxLargestLast && mode.cy > cyLargestLast) {
			cyLargestLast = mode.cy;
			nBest = n;
		}
	}

	return nBest;
}

int Game::FindBestModeMatch(int nSizeDataAbove)
{
	// Now find best match if we don't already have a match

	static byte s_abfMatchOrder[] = {
		kfMatchWidthBounded | kfMatchExactDepth, // width bounded exact depth
		kfMatchWidthBounded, // width bounded closest depth
		kfMatchExactDepth, // closest width exact depth
		0, // closest width closest depth emulated
	};

	struct DataModePairs {
		byte nDepthData;
		byte nSizeData;
		int nDepthMode;
		int cxWidthModeMin;
		int cxWidthModeMax;
	};

	static DataModePairs s_admp[] = {
		// Color high res first

        { 24, 32, 24, 320, 9999 },
#if 0
		{ 8, 32, 8, 320, 9999 },
		{ 8, 24, 8, 320, 9999 },
		{ 8, 20, 8, 240, 319 },
		{ 8, 16, 8, 160, 239 },

		// Grayscale next at 8bpp (since grayscale data is stored 8bpp this is faster)

		{ 4, 24, 8, 320, 9999 },
		{ 4, 20, 8, 240, 319 },
		{ 4, 16, 8, 160, 239 },

		// Grayscale next at 4bpp

		{ 4, 24, 4, 320, 9999 },
		{ 4, 20, 4, 240, 319 },
		{ 4, 16, 4, 160, 239 },
#endif
	};

	// Go through each data / mode pairing and perform searches based on the match order
	// Find the best match

	int immBest = -1;
	for (int n = 0; n < ARRAYSIZE(s_admp); n++) {
		DataModePairs *pdmp = &s_admp[n];
		if (pdmp->nSizeData > nSizeDataAbove) {
			for (int j = 0; j < ARRAYSIZE(s_abfMatchOrder); j++) {
				immBest = FindBestModeMatch2(pdmp->nDepthData, pdmp->nSizeData, pdmp->nDepthMode, pdmp->cxWidthModeMin, pdmp->cxWidthModeMax, s_abfMatchOrder[j]);
				if (immBest != -1)
					break;
			}
			if (immBest != -1)
				break;
		}
	}
	return immBest;
}

bool Game::InitDisplay(int immRequested)
{
	// Create a display.

	gpdisp = HostCreateDisplay();
	if (gpdisp == NULL)
		return false;	

    gpsprm = gpdisp->GetSpriteManager();

	// Find all raw data / mode matches

    ddword ddwSizes24bpp = IsDataPresent(24);
	ddword ddwSizes8bpp = IsDataPresent(8);
	ddword ddwSizes4bpp = IsDataPresent(4);
	if (ddwSizes24bpp == 0 && ddwSizes8bpp == 0 && ddwSizes4bpp == 0) {
		HostMessageBox(TEXT("Incorrect game graphics version or no game graphics installed"));
		return false;
	}

    if (ddwSizes24bpp & (((ddword)1) << 32))
		AddModeMatches(24, 32, 24, 320); // 24 depth data, 32 size data, 24 or higher dst, 320 or higher dst

	if (ddwSizes8bpp & (((ddword)1) << 32))
		AddModeMatches(8, 32, 8, 320); // 8 depth data, 24 size data, 8 or higher dst, 320 or higher dst
	if (ddwSizes8bpp & (((ddword)1) << 24))
		AddModeMatches(8, 24, 8, 320); // 8 depth data, 24 size data, 8 or higher dst, 320 or higher dst
	if (ddwSizes8bpp & (((ddword)1) << 20))
		AddModeMatches(8, 20, 8, 240);
	if (ddwSizes8bpp & (((ddword)1) << 16))
		AddModeMatches(8, 16, 8, 160);

	if (ddwSizes4bpp & (((ddword)1) << 24))
		AddModeMatches(4, 24, 4, 320); // 4 depth data, 24 size data, 4 or higher dst, 320 or higher dst
	if (ddwSizes4bpp & (((ddword)1) << 20))
		AddModeMatches(4, 20, 4, 240);
	if (ddwSizes4bpp & (((ddword)1) << 16))
		AddModeMatches(4, 16, 4, 160);

#if 0 // def DEBUG
	// Print out mode matches

	HostMessageBox("Mode Matches:");
	ModeInfo mode;
	for (int n = 0; n < m_cmm; n++) {
		ModeMatch *pmm = &m_amm[n];
		gpdisp->GetModeInfo(pmm->imode, &mode);
		HostMessageBox("%dx%dx%d cyGraffiti=%d, depthData=%d, sizeData=%d", mode.cx, mode.cy, mode.nDepth, mode.cyGraffiti, pmm->nDepthData, pmm->nSizeData);
	}

	// Print out modes

	HostMessageBox("Modes:");
	for (n = 0; n < gpdisp->GetModeCount(); n++) {
		gpdisp->GetModeInfo(n, &mode);
		HostMessageBox("%dx%dx%d cyGraffiti=%d", mode.cx, mode.cy, mode.nDepth, mode.cyGraffiti);
	}
#endif

	// Find best match

	int immBest = FindBestModeMatch(0);

	// If we a not requesting a specific size, choose one

	int immNew = immRequested;
	if (immNew == -1) {
		// If we found a best, check to see if it matches what is best in prefs. If it doesn't match best
		// in prefs, then assume there has been a config change and ignore prefs, otherwise try to use
		// prefs

		if (immBest != -1) {
			bool fMatchesPrefs = true;
			ModeInfo mode;
			gpdisp->GetModeInfo(m_amm[immBest].imode, &mode);
			if (mode.cx != gpprefs->GetInteger(knPrefCxModeBest))
				fMatchesPrefs = false;
			if (mode.cy != gpprefs->GetInteger(knPrefCyModeBest))
				fMatchesPrefs = false;
			if (mode.nDepth != gpprefs->GetInteger(knPrefDepthModeBest))
				fMatchesPrefs = false;
			if (mode.nDegreeOrientation != gpprefs->GetInteger(knPrefDegreeOrientationBest))
				fMatchesPrefs = false;
			if (m_amm[immBest].nDepthData != gpprefs->GetInteger(knPrefDepthDataBest))
				fMatchesPrefs = false;
			if (m_amm[immBest].nSizeData != gpprefs->GetInteger(knPrefSizeDataBest))
				fMatchesPrefs = false;
			if (fMatchesPrefs) {
				// Looks like there hasn't been a config change (new data / new device) so try
				// to find the last settings.

				for (int immT = 0; immT < m_cmm; immT++) {
					ModeMatch *pmmT = &m_amm[immT];
					ModeInfo modeT;
					gpdisp->GetModeInfo(pmmT->imode, &modeT);
					if (modeT.cx != gpprefs->GetInteger(knPrefCxModeLast))
						continue;
					if (modeT.cy != gpprefs->GetInteger(knPrefCyModeLast))
						continue;
					if (modeT.nDepth != gpprefs->GetInteger(knPrefDepthModeLast))
						continue;
					if (modeT.nDegreeOrientation != gpprefs->GetInteger(knPrefDegreeOrientationLast))
						continue;
					if (pmmT->nDepthData != gpprefs->GetInteger(knPrefDepthDataLast))
						continue;
					if (pmmT->nSizeData != gpprefs->GetInteger(knPrefSizeDataLast))
						continue;
					immNew = immT;
					break;
				}

				if (immNew == -1) {
					// Couldn't find what was requested. Use best

					immNew = immBest;
				}
			} else {
				// Doesn't match prefs, there has been a config change. Use what we think is best
				// rather than prefs

				immNew = immBest;
			}
		}
	}
	if (immNew == -1) {
		HostMessageBox(TEXT("No suitable graphics mode found for installed graphics data"));
		return false;
	}

	// If a mode was not explicitly chosen, bring up helpful suggestions about resolution / data size

	if (immRequested == -1) {
#if 0
//doesn't work since if the higher res data isn't in memory then the option to use it won't be considered

		// See if there is a mode that'll support higher-res graphics

		int immBetter = FindBestModeMatch(m_amm[immNew].nSizeData);
		if (immBetter != -1 && immBetter != immNew)
			HostMessageBox(TEXT("This device supports higher resolution game graphics."));
#endif

		// Give a message if grayscale data is being shown on a color device

		if (m_amm[immNew].nDepthData < 8) {
			for (int n = 0; n < gpdisp->GetModeCount(); n++) {
				ModeInfo mode;
				gpdisp->GetModeInfo(n, &mode);
				if (mode.nDepth >= 8) {
					HostMessageBox(TEXT("The game graphics database is grayscale and your device supports color. You may wish to install a color game graphics database."));
					break;
				}
			}
		}
	}

	// Try to set this mode

#if defined(WIN) && !defined(CE)
	// If changing modes, use the default scale (-1)

	int nScale = gprefsInit.nScale;
	if (immRequested != -1)
		nScale = -1;
	if (!gpdisp->SetMode(m_amm[immNew].imode, nScale))
		return false;
#else
	if (!gpdisp->SetMode(m_amm[immNew].imode))
		return false;
#endif

	// Worked, remember new and best

	m_immCurrent = immNew;
	m_immBest = immBest;

	// Get clipping dib

	gpbmClip = gpdisp->GetClippingDib();

	// Set the global fixed colors table

	switch (m_amm[m_immCurrent].nDepthData) {
#if 0
	case 4:
		gaclrFixed = gaclr4bpp;
		gfGrayscale = true;
		break;

	case 8:
		gaclrFixed = gaclr8bpp;
		gfGrayscale = false;
		break;
#endif
    case 24:
    case 32:
		gaclrFixed = gaclr24bpp;
		gfGrayscale = false;
		break;

	default:
		Assert("Whoa! Unsupported color depth");
		break;
	}

	return true;
}

void Game::RequestModeChange(int immNew)
{
	// If using this already, nothing to do

	if (immNew == m_immCurrent)
		return;

	// Stop the app; this'll cause it to restart with the new selection

	gevm.SetAppStopping();
	gimmReinitialize = immNew;
}

bool Game::InitMultiFormMgr()
{
	// Create the MultiFormMgr

	gpmfrmm = new MultiFormMgr();
	Assert(gpmfrmm != NULL, "out of memory!");
	if (gpmfrmm == NULL)
		return false;
	if (!gpmfrmm->Init(gpdisp->GetBackDib(), false))
		return false;

	// Grab dib size

	DibBitmap *pbm = gpdisp->GetBackDib();
	if (pbm == NULL)
		return false;
	Size sizDib;
	pbm->GetSize(&sizDib);

	// Initialize some global metrics used for drawing things as appropriate
	// for the data size being used.

	switch (m_amm[m_immCurrent].nSizeData) {
	case 16:
		gcxFullScreenFormDiv10 = 160 / 10;
		break;

	case 20:
		gcxFullScreenFormDiv10 = 240 / 10;
		break;

	case 24:
    case 32:
		gcxFullScreenFormDiv10 = 320 / 10;
		break;
	}
	gcxyBorder = PcFromFc(1);

	// HT runs on known and future unknown screen modes. Because of this the main game form
	// is dynamically sized and repositioned based on knowledge of how the game form is
	// layed out.

	TBitmap *ptbmMenuButton = CreateTBitmap("menuup.png");
	if (ptbmMenuButton == NULL) {
		return false;
    }
	Size sizMenuButton;
	ptbmMenuButton->GetSize(&sizMenuButton);
	delete ptbmMenuButton;

	// cySim is the simulation area
	// cyInput is the control stip at the bottom
	// cyInputForm is present for capturing input in the graffiti area

	int cyInput = sizMenuButton.cy;
	int cySim = sizDib.cy - cyInput;
	ModeInfo mode;
	gpdisp->GetMode(&mode);
	int cyInputForm = cyInput + mode.cyGraffiti;

	// We know the playfield size

	m_sizPlayfield.cx = sizDib.cx;
	m_sizPlayfield.cy = cySim;

	// Add two partitions

	DibBitmap *pbmSim = pbm->Suballoc(0, cySim);
	if (pbmSim == NULL)
		return false;
	gpfrmmSim = CreateFormMgr(pbmSim);
	if (gpfrmmSim == NULL) {
		delete pbmSim;
		return false;
	}
	gpupdSim = gpfrmmSim->GetUpdateMap();
	Rect rc;
	rc.Set(0, 0, m_sizPlayfield.cx, cySim);
	gpmfrmm->AddFormMgr(gpfrmmSim, &rc);
	DibBitmap *pbmInput = pbm->Suballoc(cySim, cyInput);
	if (pbmInput == NULL)
		return false;
	gpfrmmInput = CreateFormMgr(pbmInput);
	if (gpfrmmInput == NULL) {
		delete pbmInput;
		return false;
	}
	gpfrmmInput->SetFlags(gpfrmmInput->GetFlags() | kfFrmmNoScroll);
	rc.Set(0, cySim, m_sizPlayfield.cx, cySim + cyInputForm);
	gpmfrmm->AddFormMgr(gpfrmmInput, &rc);

    // Tell the display about the two formmgr's that make
    // up the screen. Some displays will want this for managing
    // the front dib specially.

    gpdisp->SetFormMgrs(gpfrmmSim, gpfrmmInput);

	return true;
}

void Game::ParseVersion(char *pszVersion, int *pnShipVersion, int *pnMajorVersion, char *pchMinorVersion)
{
	// Parse the version. 
	// A.B[c], for example 1.0a
	// A is ship version
	// B is major version
	// C is minor version

	// Scan for ship version. Allow 0., or no ship version specified

	int ichAfterDot = 0;
	int c = IniScanf(pszVersion, "%d.%+", pnShipVersion, &ichAfterDot);
	if (c == 0) {
		*pnShipVersion = 0;
		ichAfterDot = 0;
	}

	// Scan for major version. Do it this way to support 1.01 as less than 1.10

	*pnMajorVersion = 0;
	int ichAfterMajor = ichAfterDot;
	if (pszVersion[ichAfterMajor] >= '0' && pszVersion[ichAfterMajor] <= '9') {
		*pnMajorVersion += (pszVersion[ichAfterMajor] - '0') * 10;
		ichAfterMajor++;
	}
	if (pszVersion[ichAfterMajor] >= '0' && pszVersion[ichAfterMajor] <= '9') {
		*pnMajorVersion += (pszVersion[ichAfterMajor] - '0');
		ichAfterMajor++;
	}

	// Scan for minor version. Allow none

	char *pszAfterMajor = &pszVersion[ichAfterMajor];
	if (*pszAfterMajor >= 'a' && *pszAfterMajor <= 'z') {
		*pchMinorVersion = *pszAfterMajor;
	} else {
		*pchMinorVersion = 0;
	}
}

bool Game::GetFormattedVersionString(char *pszVersion, char *pszOut)
{
	int nShipVersion, nMajorVersion;
	char chMinorVersion;
	ggame.ParseVersion(pszVersion, &nShipVersion, &nMajorVersion, &chMinorVersion);
	if (nShipVersion == 0 && nMajorVersion == 0 && chMinorVersion == 0)
		return false;

	if (nMajorVersion % 10 == 0) {
		if (chMinorVersion == 0) {
			sprintf(pszOut, "v%d.%d", nShipVersion, nMajorVersion / 10);			
		} else {
			sprintf(pszOut, "v%d.%d%c", nShipVersion, nMajorVersion / 10, chMinorVersion);	
		}
	} else {
		if (chMinorVersion == 0) {
			sprintf(pszOut, "v%d.%d%d", nShipVersion, nMajorVersion / 10, nMajorVersion % 10);
		} else {
			sprintf(pszOut, "v%d.%d%d%c", nShipVersion, nMajorVersion / 10, nMajorVersion % 10, chMinorVersion);
		}
	}
	return true;
}

bool Game::CheckDatabaseVersion(const char *pszDir, char *pszPdb,
        bool fUpwardCompatOK)
{
	if (!gpakr.Push(pszDir, pszPdb))
		return false;

#ifdef DEV_BUILD
	// If DEV_BUILD, assume all database versions are compatible so that we can run in
	// mismatched situations on purpose

	gpakr.Pop();
	return true;
#else

	FileMap fmap;
	char *pszDataVersion = (char *)gpakr.MapFile("version.txt", &fmap);
	if (pszDataVersion == NULL) {
		gpakr.Pop();
		return false;
	}

	// Parse the version

	int nVersionDataShip;
	int nVersionDataMajor;
	char chVersionDataMinor;
	ParseVersion(pszDataVersion, &nVersionDataShip, &nVersionDataMajor, &chVersionDataMinor);
	gpakr.UnmapFile(&fmap);
	gpakr.Pop();

	// Compare versions

	return IsVersionCompatibleWithExe(nVersionDataShip, nVersionDataMajor, chVersionDataMinor, fUpwardCompatOK);
#endif
}

bool Game::IsVersionCompatibleWithExe(int nVersionCompareShip, int nVersionCompareMajor, char chVersionCompareMinor, bool fUpwardCompatOK)
{
	// Unstamped EXEs can load any version database

	if (stricmp(gszVersion + 1, "++VERSION+++") == 0) // this wierd +1 stuff is so the version stamper doesn't stamp this string
		return true;

	int nVersionShip, nVersionMajor;
	char chVersionMinor;
	ParseVersion(gszVersion, &nVersionShip, &nVersionMajor, &chVersionMinor);

	if (fUpwardCompatOK) {
		// If ship version less, not ok

		if (nVersionShip < nVersionCompareShip)
			return false;

		// If ship version equal, check major

		if (nVersionShip == nVersionCompareShip) {
			if (nVersionMajor < nVersionCompareMajor)
				return false;
		}
	} else {
		// Ship versions must match

		if (nVersionShip != nVersionCompareShip)
			return false;

		// Major versions must match too.

		if (nVersionMajor != nVersionCompareMajor)
			return false;
	}

	// Minor versions can be different.

	return true;
}

ddword Game::IsDataPresent(int cBpp)
{
    const char *pszMainDataDir = HostGetMainDataDir();
    char szPdb[kcbFilename];
	char szDir[MAX_PATH];
	ddword ddwSizes = 0;

    // tile sizes to check
    int catsz[] = {
        // 16,
        // 20,
        // 24,
        32
    };

    for (int i = 0; i < ARRAYSIZE(catsz); i++) {

        // Data may be a pdb

        sprintf(szPdb, "htdata%d%i.pdb", cBpp, catsz[i]);
        if (CheckDatabaseVersion(pszMainDataDir, szPdb, false)) {
            ddwSizes |= (((ddword)1) << catsz[i]);
            continue;
        }

        // Data may be a directory

        if (pszMainDataDir == NULL) {
            sprintf(szDir, "htdata%d%i", cBpp, catsz[i]);
        } else {
            sprintf(szDir, "%s/htdata%d%i", pszMainDataDir, cBpp, catsz[i]);
        }
        if (CheckDatabaseVersion(szDir, NULL, false))
            ddwSizes |= (((ddword)1) << catsz[i]);
    }
    
	return ddwSizes;
}

// PlayLevel closes and deletes pstmSavedGame at its first opportunity or
// in the event of an error. Either way when it returns pstmSavedGame has
// been deleted.

int Game::PlayLevel(MissionIdentifier *pmiid, Stream *pstmSavedGame, int nRank)
{
	Dictionary dictPvarsRetry;

	m_szNextLevel[0] = 0;

	if (pstmSavedGame != NULL) {
		// Load MissionIdentifier

        pstmSavedGame->Read(&m_miid.packid, sizeof(m_miid.packid));
        pstmSavedGame->ReadString(m_miid.szLvlFilename,
                sizeof(m_miid.szLvlFilename));
        
		// Load next level filename

		pstmSavedGame->ReadString(m_szNextLevel, sizeof(m_szNextLevel));

		// Load pvars

		m_dictPvars.LoadState(pstmSavedGame);

		// Prep for level retry

		dictPvarsRetry.Init(&m_dictPvars);

	} else {
        // This is the mission we want to load
        m_miid = *pmiid;

		// Clear pvars

		m_dictPvars.Clear();

		// hack for carrying forward a rank from the demo

		if (nRank > 0) {
			char szT[20];
			itoa(nRank, szT, 10);
			m_dictPvars.Set("rank", szT);
		}

	}

	// Loop while the player retries the level or as new levels are set via SetNextLevel.

	int nGo = knGoSuccess;
	do {
		// Randomness is good but not REAL randomness.

#ifdef STRESS
		if (gfStress) {
			SetRandomSeed(gtimm.GetTickCount());
		} else {
			SetRandomSeed(0x29a);
        }
#else
		SetRandomSeed(gfMultiplayer ? gtimm.GetTickCount() : 0x29a);
#endif
        // Mount the pdb with the mission pack

        if (!gppackm->Mount(gpakr, &m_miid.packid)) {
            pstmSavedGame->Close();
            delete pstmSavedGame;
            nGo = knGoInitFailure;
            break;
        }

        // The level can be changed by a SetNextMisson action. This is how
        // we manage progressing from one level to the next.

		if (pstmSavedGame != NULL) {
			if (!gplrm.LoadState(pstmSavedGame)) {
                gppackm->Unmount(gpakr, &m_miid.packid);
				pstmSavedGame->Close();
				delete pstmSavedGame;
				nGo = knGoInitFailure;
				break;
			}
		} else {
            // Prep the Player instances that govern the local player and the
            // computer players using the SideInfo contained in the level.

			gplrm.Init(m_miid.szLvlFilename);
		}

        // Play the level until the player loses, exits, or advances to the
        // next level

        nGo = RunSimulation(pstmSavedGame, m_miid.szLvlFilename, 0, 0, NULL);

        // Only use the saved game stream the first time through. Any
        // restarts start from the beginning of the level.

        pstmSavedGame = NULL;

        gppackm->Unmount(gpakr, &m_miid.packid);

		switch (nGo) {
		case knGoAppStop:
		case knGoInitFailure:
		case knGoLoadSavedGame:
			return nGo;

		case knGoSuccess:

			// If no next level, we're done.

			if (m_szNextLevel[0] == 0) {
				if (gfDemo &&
                        (stricmp(m_miid.szLvlFilename, "d_03.lvl") == 0)) {

                    // if player just finished D3, save rank in prefs so we
                    // know to start from M3 when they buy a new copy. If they
                    // played D3 as a challenge mission, their rank will be -1

					char szT[kcbPvarValueMax];
					if (ggame.GetVar("rank", szT, sizeof(szT)))
						gnDemoRank = atoi(szT);
					if (gnDemoRank < 0)
						gnDemoRank = 0;

					SavePreferences();
				}				
				nGo = knGoAbortLevel;
			} else {
                // Mark this mission complete!

                gpcptm->MarkComplete(&m_miid);

                // Copy over new mission filename

				strncpyz(m_miid.szLvlFilename, m_szNextLevel,
                        sizeof(m_miid.szLvlFilename));

				// Successful level completion carries pvars forward

				dictPvarsRetry.Init(&m_dictPvars);
				m_szNextLevel[0] = 0;
			}
			break;

        // Retrying a level reverts pvars back to the state they were at upon
        // level entry

		case knGoRetryLevel:
			m_dictPvars.Init(&dictPvarsRetry);
			break;

		default:
			// Clear pvars

			m_dictPvars.Clear();
			dictPvarsRetry.Clear();
			break;
		}

	} while (nGo != knGoAbortLevel);

	return nGo;
}

// RunSimulation will Close and delete the passed-in Stream before returning

int Game::RunSimulation(Stream *pstm, char *pszLevel, word wfRole,
        dword gameid, Chatter *chatter)
{
	DialogForm *pfrm = (DialogForm *)gpmfrmm->LoadForm(gpiniForms, kidfLoading, new DialogForm());
	if (pfrm != NULL) {
		pfrm->SetBackgroundColorIndex(kiclrBlack);
		pfrm->SetTitleColor(GetColor(kiclrBlack));
		pfrm->SetBorderColorIndex(kiclr0CyanSide);
		gpmfrmm->DrawFrame(false);
	}

	// Retain whether or not this is a multiplayer game. Inquiring minds will 
	// want to know later.

	m_wf &= ~(kfGameMultiplayer | kfGameRoleServer);
	if (wfRole & kfRoleMultiplayer) {
		m_wf |= kfGameMultiplayer;
		gfMultiplayer = true;

		// Pvars shouldn't carry into multiplayer games.

		m_dictPvars.Clear();
	} else {
		gfMultiplayer = false;
	}
	if (wfRole & kfRoleServer)
		m_wf |= kfGameRoleServer;

	gsndm.WaitSilence();
	bool fSuccess = InitSimulation(pstm, pszLevel, wfRole, gameid, chatter);

	if (pstm != NULL) {
		if (!pstm->IsSuccess())
			fSuccess = false;

		pstm->Close();
		delete pstm;
	}

	delete pfrm;

	if (!fSuccess) {
		gfMultiplayer = false;
		return knGoInitFailure;
	}

	// Clear fog if asked ("-nofog" command line switch)

//temp
#if 0
	gfClearFog = true;
#endif

	if (gfClearFog)
		gsim.GetLevel()->GetFogMap()->RevealAll(gpupdSim);

	// Game loop

    m_fUpdateTriggers = false;

	int nGo;
	Event evt;
	while (true) {
		if (!gevm.GetEvent(&evt, -1, false)) {
            continue;
        }

		if (FilterEvent(&evt))
			continue;

		if (evt.eType == appStopEvent) {
			// Warn about multiplayer if the player pressed the home button

			if (m_wf & kfGameMultiplayer) {
				if (!AskResignGame())
					continue;
			}
			if (evt.dw != knAppExit)
				SaveReinitializeGame();
			nGo = knGoAppStop;
			break;
		}

		if (evt.eType == gameOverEvent) {
			nGo = (int)evt.dw;
			break;
		}

		gevm.DispatchEvent(&evt);
	}

	ExitSimulation();

    // Eat potential cancel mode messages that due to MP exit ordering got
    // posted after gameOverEvent

	if (gevm.PeekEvent(&evt, 0)) {
		if (evt.eType == cancelModeEvent)
			gevm.GetEvent(&evt, 0);
	}

	// Assume not multiplayer anymore

	gfMultiplayer = false;
	m_wf &= ~(kfGameMultiplayer | kfGameRoleServer);

	return nGo;
}

bool Game::AskResignGame(bool fTellHost)
{
	bool fAppStopping = gevm.IsAppStopping();
    bool fAsk = true;
#if defined(IPHONE) || defined(__IPHONEOS__) || defined (__ANDROID__)
    // When an iPhone or Android game exits, it exits without the opportunity to
    // confirm with the user. So if iPhone and the app is stopping, just
    // resign.
    if (fAppStopping) {
        fAsk = false;
    }
#endif

	Assert(m_wf & kfGameMultiplayer);

    if (fAsk) {
        // If app stopping is set

        if (fAppStopping) {
            gevm.ClearAppStopping();
        }

        // Ask user

        char sz[128];
        if (m_wf & kfGameRoleServer) {
            gpstrtbl->GetString(kidsExitHost, sz, sizeof(sz));
        } else {
            gpstrtbl->GetString(kidsExitClient, sz, sizeof(sz));
        }

        // If the user wants to keep going, return with "app stopping" cleared

        if (!HtMessageBox(kidfMessageBoxQuery, kfMbWhiteBorder | kfMbKeepTimersEnabled, "Warning", sz))
            return false;

        // If app stopping set it again

        if (fAppStopping) {
            gevm.SetAppStopping();
        }
    }

	if (fTellHost && gptra != NULL) {
		// Tell host

		PlayerResignNetMessage prng;
		prng.pid = gpplrLocal->GetId();
        gptra->SendNetMessage(&prng);
	}
	return true;
}

bool Game::AskObserveGame()
{
	bool fYes = false;
	gpplrLocal->SetFlags(gpplrLocal->GetFlags() | kfPlrObserver);

	SimUIForm *psui = ggame.GetSimUIForm();
	psui->ClearSelection();

	if (!gevm.IsAppStopping()) {
		if (psui->GetRole() & kfRoleServer) {
			fYes = HtMessageBox(kidfMessageBoxQuery, kfMbKeepTimersEnabled | kfMbWhiteBorder, 
				"GAME OVER", "Do you want to continue as an observer? Since this device hosting, answering no will stop the game.");
		} else {
			fYes = HtMessageBox(kidfMessageBoxQuery, kfMbKeepTimersEnabled | kfMbWhiteBorder, 
				"GAME OVER", "Do you want to continue as an observer?");
		}

		if (fYes) {
            psui->SetObserving();
		}
	}

	return fYes;
}

bool Game::SaveReinitializeGame()
{
    if (m_fSkipSaveReinitialize) {
        return false;
    }

	// We don't save "reinitialize" games when playing multiplayer

	if (gfMultiplayer)
		return false;

	Status("** Saving Game for Return **");
	Stream *pstm = HostNewSaveGameStream(knGameReinitializeSave, "Reinitialize");
	Assert(pstm != NULL);
	if (pstm != NULL) {
		ggame.SaveGame(pstm);  // SaveGame closes and deletes the stream
		return true;
	}

	return false;
}

bool Game::InitSimulation(Stream *pstm, char *pszLevel, word wfRole,
        dword gameid, Chatter *chatter) {

	// Perform one-time Simulation initialization. This might seem like a 
	// strange place to do it but the goal is to put off the expensive stuff
	// it does so it doesn't slow game launch time.

    m_gameid = gameid;

	if (m_fSimUninitialized) {
		Status("Simulation Init (one-time)...");
		if (!gsim.OneTimeInit())
			return false;
		m_fSimUninitialized = false;
	}

	Status("Simulation Init (per-level)...");
	if (!gsim.PerLevelInit())
		return false;

	// Initialize the Simulation StateMachineMgr

	if (!gsmm.Init(&gsim)) {
		gsim.PerLevelExit();
		return false;
	}

	// Either loading a saved game or a new level

	if (pstm == NULL) {
		// Load the first level

		if (!gsim.LoadLevel(pszLevel)) {
			gsim.PerLevelExit();
			return false;
		}
	} else {
		if (!gsim.LoadState(pstm)) {
			gsim.PerLevelExit();
			return false;
		}
	}

	// Load the sim UI form

	m_pfrmSimUI = (SimUIForm *)gpfrmmSim->LoadForm(gpiniForms, kidfSimUI, new SimUIForm(wfRole, gameid, chatter));
	if (m_pfrmSimUI == NULL) {
		Assert("LoadForm(SimUIForm) failed");
		gsim.PerLevelExit();
		return false;
	}

	m_pfrmSimUI->CalcLevelSpecificConstants();

	// Load the input UI form

	m_pfrmInputUI = (InputUIForm *)gpfrmmInput->LoadForm(gpiniForms, kidfInputUI, new InputUIForm(chatter));
	if (m_pfrmInputUI == NULL) {
		Assert("LoadForm(InputUIForm) failed");
		delete m_pfrmSimUI;
		m_pfrmSimUI = NULL;
		gsim.PerLevelExit();
		return false;
	}

	// Load minimap form

	m_pfrmMiniMap = gpmfrmm->LoadForm(gpiniForms, kidfMiniMap, new Form());
	if (m_pfrmMiniMap == NULL) {
		Assert("LoadForm(Minimap) failed");
		delete m_pfrmInputUI;
		m_pfrmInputUI = NULL;
		delete m_pfrmSimUI;
		m_pfrmSimUI = NULL;
		gsim.PerLevelExit();
		return false;
	}

	// This form doesn't want key events

	m_pfrmMiniMap->SetFlags(m_pfrmMiniMap->GetFlags() | kfFrmNoFocus);

	// Position minimap

	MiniMapControl *pctlMiniMap = (MiniMapControl *)m_pfrmMiniMap->GetControlPtr(kidcMiniMap);
	Assert(pctlMiniMap != NULL);
	pctlMiniMap->SetPosition(0, 0);
	Rect rcMiniMap;
	pctlMiniMap->GetRect(&rcMiniMap);
	Size sizDibT;
	gpmfrmm->GetDib()->GetSize(&sizDibT);
	Rect rcT;
	rcT.Set(sizDibT.cx - rcMiniMap.Width(), sizDibT.cy - rcMiniMap.Height(), sizDibT.cx, sizDibT.cy);
	m_pfrmMiniMap->SetRect(&rcT);

	// This hack is so the Simulation's first Update occurs before its first
	// paint. By doing it this way the MissionLoaded trigger gets to determine
	// the first image the player sees. Otherwise the map will paint first, 
	// then when the update timer goes off 80 milliseconds later the 
	// MissionLoaded trigger will go off and pop up an ecom, reposition the
	// view, etc.

	if ((wfRole & kfRoleMultiplayer) == 0)
		m_pfrmSimUI->OnTimer(0);

    // Tell the sprite manager the clipping rects

    Rect rcClip1;
    rcClip1.left = 0;
    rcClip1.top = 0;
    rcClip1.right = m_sizPlayfield.cx - rcMiniMap.Width();
    rcClip1.bottom = m_sizPlayfield.cy;
    Rect rcClip2;
    rcClip2.left = rcClip1.right;
    rcClip2.top = 0;
    rcClip2.right = m_sizPlayfield.cx;
    rcClip2.bottom = rcT.top;
    SpriteManager *psprm = gpdisp->GetSpriteManager();
    if (psprm != NULL) {
        psprm->SetClipRects(&rcClip1, &rcClip2);
    }

	return true;
}

void Game::ClearDisplay()
{
	if (gpdisp == NULL)
		return;
	DibBitmap *pbmBack = gpdisp->GetBackDib();
	if (pbmBack == NULL)
		return;
    if (gaclrFixed != NULL)
        pbmBack->Clear(GetColor(kiclrBlack));
	if (gpmfrmm != NULL) {
		gpmfrmm->InvalidateRect(NULL);
		gpmfrmm->DrawFrame(false, false);
		gpmfrmm->InvalidateRect(NULL);
	}
}

void Game::ExitSimulation()
{
	Status("Exit Simulation (per-level)...");
	gsim.PerLevelExit();
	gsmm.ClearDelayedMessages();
	delete m_pfrmSimUI;
	m_pfrmSimUI = NULL;
	delete m_pfrmInputUI;
	m_pfrmInputUI = NULL;
	delete m_pfrmMiniMap;
	m_pfrmMiniMap = NULL;

	ClearDisplay();
}

void Game::Exit()
{
#if defined(WIN) && !defined(CE)
	// If the game exits without pressing F8, stop the avi recording automatically

	if (gpavir != NULL) {
		delete gpavir;
		gpavir = NULL;
	}
#endif

	// Save preferences

	SavePreferences();

    delete gpprefs;
    gpprefs = NULL;

	Status("Exit Simulation (one-time)...");
	gsim.OneTimeExit();

	m_pfrmSimUI = NULL;
	delete gpmfrmm;
	gpmfrmm = NULL;
	delete gpiniForms;
	gpiniForms = NULL;
	delete gpiniGame;
	gpiniGame = NULL;

	if (m_wf & kfGameInitFonts) {
		for (int ifnt = 0; ifnt < kcFonts; ifnt++) {
			delete gapfnt[ifnt];
			gapfnt[ifnt] = NULL;
		}
		m_wf &= ~kfGameInitFonts;
	}

	gsndm.Exit();

	ButtonControl::ExitClass();
	CheckBoxControl::ExitClass();
	PipMeterControl::ExitClass();
	ListControl::ExitClass();
	DamageMeterControl::ExitClass();
	
	gshl.Exit();

	delete gpstrtbl;
	gpstrtbl = NULL;

    delete gptam;
    gptam = NULL;

	// Pop off all data files

	while (gpakr.Pop())
		;

	if (gmpWcFromPc != NULL) {
		gmmgr.FreePtr(gmpWcFromPc);
		gmpWcFromPc = NULL;
	}
	if (gmpPcFromWc != NULL) {
		gmmgr.FreePtr(gmpPcFromWc);
		gmpPcFromWc = NULL;
	}

	delete gpdisp;
	gpdisp = NULL;

	delete[] m_amm;
	m_amm = NULL;
	m_cmm = 0;
	m_cmmAlloc = 0;

	gplrm.Reset();

	gcam.Exit();
	gmmgr.Exit();

	// Depends on gpbScratch so clean up before deleting

#if defined(PIL) && defined(TRACE_TO_DB_LOG)
	DbLogExit();
#endif

	delete[] gpbScratch;
	gpbScratch = NULL;

	m_fSimUninitialized = true;
}

void Game::ScheduleUpdateTriggers() {
    m_fUpdateTriggers = true;

    // Force GetEvent to return a message. This will cause FilterEvent
    // to get called, where the check for m_fUpdateTriggers will be made,
    // to update triggers immediately.

    Event evt;
    evt.eType = updateTriggersEvent;
    gevm.PostEvent(&evt);
}

void Game::UpdateTriggers() {
    // Always run triggers this way, so that trigger execution is in sync
    // with multiplayer updates. This also allows triggers to go into modal
    // loops, while the simulation continues (with special care).

    if (m_fUpdateTriggers) {
        m_fUpdateTriggers = false;
		if (gsim.GetLevel() != NULL) {
			if (gsim.GetLevel()->GetTriggerMgr() != NULL)
				gsim.GetLevel()->GetTriggerMgr()->Update();
		}
    }
}

bool Game::FilterEvent(Event *pevt)
{
#ifdef INCL_VALIDATEHEAP
	gmmgr.Validate();
#endif

    UpdateTriggers();

	switch (pevt->eType) {
    case checkGameOverEvent:
        if (m_pfrmSimUI != NULL) {
            m_pfrmSimUI->CheckMultiplayerGameOver((Pid)pevt->dw);
        }
        break;
        
	case gamePaintEvent:
		// Keep track of # of paints for fps stat

		extern short gcPaint;
		gcPaint++;

		// Draw

		gpmfrmm->DrawFrame(true);
		return true;

	case gameSuspendEvent:
		// OS alert is showing.

		Suspend();
		return true;

    case transportEvent:
        if (gptra != NULL) {
            gptra->OnEvent(pevt);
        }
        return true;

    case mpEndMissionWinEvent:
        EndMissionAction::OnMPEndMissionActionEvent(knWinLoseTypeWin,
                pevt->dw);
        break;

    case mpEndMissionLoseEvent:
        EndMissionAction::OnMPEndMissionActionEvent(knWinLoseTypeLose,
                pevt->dw);
        break;

    case mpShowObjectivesEvent:
        ShowObjectivesAction::OnMPShowObjectivesEvent(pevt->dw);
        break;

    case disableSoundEvent:
        gsndm.SaveStateAndClear();
        break;

    case enableSoundEvent:
        gsndm.RestoreState();
        break;
	}

	return false;
}

char *gszPaused = "PAUSED";

void Game::Suspend()
{
	// Force back buffer to be valid

	gpmfrmm->DrawFrame(true);

	// Darken back buffer

	DibBitmap *pbmBack = gpdisp->GetBackDib();
	Size sizDib;
	pbmBack->GetSize(&sizDib);
	pbmBack->Shadow(0, 0, sizDib.cx, sizDib.cy);

	// Draw "Paused" sign in the middle
	// TODO: Make prettier

	Font *pfnt = gapfnt[kifntTitle];
	int cxText = pfnt->GetTextExtent(gszPaused);
	int cyText = pfnt->GetHeight();
	int cxBox = cxText * 3;
	int cyBox = cyText * 3;
	int x = (sizDib.cx - cxBox) / 2;
	int y = (sizDib.cy - cyBox) / 2;
	pbmBack->Fill(x, y, cxBox, cyBox, GetColor(kiclrBlack));
	pfnt->DrawText(pbmBack, gszPaused, x + cxText, y + cyText);

	// Call host. This is a modal loop. When it returns, the game isn't
	// paused anymore

	gsndm.SaveStateAndClear();
	HostSuspendModalLoop(pbmBack);
	gsndm.RestoreState();

	// Ok, invalidate everything so it all redraws

	gpmfrmm->InvalidateRect(NULL);
}

void Game::GetPlayfieldSize(Size *psiz)
{
	*psiz = m_sizPlayfield;
}

SimUIForm *Game::GetSimUIForm()
{
	return m_pfrmSimUI;
}

InputUIForm *Game::GetInputUIForm()
{
	return m_pfrmInputUI;
}

Form *Game::GetMiniMapForm()
{
    return m_pfrmMiniMap;
}

#define knVerGameState 9
int Game::PlaySavedGame(Stream *pstm)
{
	// Overall, look at build version first

	char szVersion[32];
	szVersion[0] = 0;
	pstm->ReadString(szVersion, sizeof(szVersion));

	// Read version

	bool fStale = false;
	byte nVer = pstm->ReadByte();
	if (nVer != knVerGameState)
		fStale = true;

	// Read platform

	if (!fStale) {
		byte bPlatform = 0;
		if (nVer >= 6)
			bPlatform = pstm->ReadByte();

		// Check version

		if (!CheckSaveGameVersion(szVersion, bPlatform))
			fStale = true;
	}

	// If old, bail

	if (fStale) {
		pstm->Close();
		delete pstm;
		HtMessageBox(kfMbWhiteBorder | kfMbClearDib, "Error", "Save game out of date!");
		return knGoFailure;
	}

	// PlayLevel will close and delete pstm, guaranteed.

	return PlayLevel(NULL, pstm);
}

void Game::SaveGame(Stream *pstm)
{
	if (pstm == NULL) {
		if (!PickSaveGameStream(&pstm))
			return;
	}

	bool fSuccess = false;
	if (pstm != NULL) {
		// Write build version string first
		// NOTE: these first 3 entries are assumed in CheckSaveGameVersion() and DeleteStaleSaveGames()!!

		pstm->WriteString(gszVersion);

		// Save version

		pstm->WriteByte(knVerGameState);

		// Save platform byte. Needed for platform compatibility check (68K->ARM transition)

#ifdef PIL
#ifdef PNO
		// ARM saved game

		pstm->WriteByte(1);
#else
		// 68K saved game

		pstm->WriteByte(0);
#endif
#else
		// Whatever, doesn't matter since it isn't Palm

		pstm->WriteByte(0);
#endif

		// Save mission identifier

        pstm->Write(&m_miid.packid, sizeof(m_miid.packid));
        pstm->WriteString(m_miid.szLvlFilename);

		// Save next level filename

		pstm->WriteString(m_szNextLevel);

		// Save pvars

		m_dictPvars.SaveState(pstm);

		// Save player info 

		gplrm.SaveState(pstm);

		// Save simulation

		gsim.SaveState(pstm);

		// Check for error

		fSuccess = pstm->IsSuccess();
	}

	if (!fSuccess)
		HtMessageBox(0, "Save Game", "Error saving game! This device is too low on memory to save this game. Save over an existing same game or free some space and try again.");

	if (pstm != NULL) {
		pstm->Close();
		delete pstm;
	}
}

void Game::SetNextLevel(char *pszLevel)
{
    // Perform simple lower case

    char szT[sizeof(m_szNextLevel)];
	strncpyz(szT, pszLevel, sizeof(szT));
	char *pszT = szT;
	while (*pszT != 0) {
		if (*pszT >= 'A' && *pszT <= 'Z')
			(*pszT) += 'a' - 'A';
		pszT++;
	}

    // Some missions don't have correct "next level" filenames,
    // in that .lvl isn't there. Append if it's not there.

    int cch = (int)strlen(szT);
    if (sizeof(szT) - 1 - cch >= 4) {
        bool fAppend = false;
        if (cch < 4) {
            fAppend = true;
        } else if (szT[cch - 4] != '.' ||
                szT[cch - 3] != 'l' ||
                szT[cch - 2] != 'v' ||
                szT[cch - 1] != 'l') {
            fAppend = true;
        }
        if (fAppend) {
            strncpyz(&szT[cch], ".lvl",
                    sizeof(szT) - cch);
        }
    }
    strncpyz(m_szNextLevel, szT, sizeof(m_szNextLevel));
}

char *Game::GetNextLevel()
{
	return m_szNextLevel;
}

bool Game::IsMultiplayer()
{ 
	return (m_wf & kfGameMultiplayer) != 0; 
}

void Game::SetGameSpeed(int t)
{
	gtGameSpeed = t;
	if (m_pfrmSimUI != NULL)
		gtimm.SetTimerRate(m_pfrmSimUI, gtGameSpeed);
}

// Preferences support.

Preferences *gpprefs;

bool Game::LoadPreferences()
{
	// Try to load preferences.

    gpprefs = PrefsFromFile();

    // Failed? Initialize preferences to default values

    if (gpprefs == NULL) {
        gpprefs = PrefsFromDefaults();
        if (gpprefs == NULL)
            return false;
    }

    return true;
}

void Game::SavePreferences()
{
    // Only if we've gone through initialization ok

	if (!(m_wf & kfGameInitDone))
		return;

	// Always save the latest version of the preferences

	gpprefs->Set(knPrefVersion, knVersionPreferencesLatest);
    gpprefs->Set(kszPrefUsername, gszUsername);
    gpprefs->Set(kszPrefPassword, gszPassword);
    gpprefs->Set(kszPrefToken, gszToken);
    gpprefs->Set(kfPrefAnonymous, gfAnonymous);

	Date date;
	HostGetCurrentDate(&date);
    gpprefs->Set(knPrefYearLastRun, date.nYear);
    gpprefs->Set(knPrefMonthLastRun, date.nMonth);
    gpprefs->Set(knPrefDayLastRun, date.nDay);

    gpprefs->Set(knPrefVolume, gsndm.GetVolume());
    gpprefs->Set(kfPrefSoundMuted, !gsndm.IsEnabled());
    // gpprefs->Set(kfPrefIgnoreBluetoothWarning, gfIgnoreBluetoothWarning);
    gpprefs->Set(kwfPrefPerfOptions, BigWord((gwfPerfOptions & kfPerfAll) | (kfPerfMax & ~kfPerfAll)));
    gpprefs->Set(knPrefGameSpeed, gtGameSpeed);
    gpprefs->Set(kfPrefLassoSelection, gfLassoSelection);
    gpprefs->Set(knPrefHueOffset, gnHueOffset);
    gpprefs->Set(knPrefSatMultiplier, gnSatMultiplier);
    gpprefs->Set(knPrefLumOffset, gnLumOffset);
    gpprefs->Set(kwfPrefHandicap, gwfHandicap);
    gpprefs->Set(knPrefDemoRank, gnDemoRank);
    gpprefs->Set(knPrefScrollSpeed, gnScrollSpeed);
    gpprefs->Set(kszPrefAskUrl, gszAskURL);
    gpprefs->Set(kszPrefDeviceId, gszDeviceId);

	if (gpdisp == NULL || m_immBest == -1) {
        gpprefs->Set(knPrefCxModeBest, 0);
        gpprefs->Set(knPrefCyModeBest, 0);
        gpprefs->Set(knPrefDepthModeBest, 0);
        gpprefs->Set(knPrefDepthDataBest, 0);
        gpprefs->Set(knPrefSizeDataBest, 0);
        gpprefs->Set(knPrefDegreeOrientationBest, 0);
	} else {
		ModeInfo mode;
		gpdisp->GetModeInfo(m_amm[m_immBest].imode, &mode);
        gpprefs->Set(knPrefCxModeBest, mode.cx);
        gpprefs->Set(knPrefCyModeBest, mode.cy);
        gpprefs->Set(knPrefDepthModeBest, mode.nDepth);
        gpprefs->Set(knPrefDepthDataBest, m_amm[m_immBest].nDepthData);
        gpprefs->Set(knPrefSizeDataBest, m_amm[m_immBest].nSizeData);
        gpprefs->Set(knPrefDegreeOrientationBest, mode.nDegreeOrientation);
	}
	if (gpdisp == NULL || m_immCurrent == -1) {
        gpprefs->Set(knPrefCxModeLast, 0);
        gpprefs->Set(knPrefCyModeLast, 0);
        gpprefs->Set(knPrefDepthModeLast, 0);
        gpprefs->Set(knPrefDepthDataLast, 0);
        gpprefs->Set(knPrefSizeDataLast, 0);
        gpprefs->Set(knPrefDegreeOrientationLast, 0);
	} else {
		ModeInfo mode;
		gpdisp->GetModeInfo(m_amm[m_immCurrent].imode, &mode);
        gpprefs->Set(knPrefCxModeLast, mode.cx);
        gpprefs->Set(knPrefCyModeLast, mode.cx);
        gpprefs->Set(knPrefDepthModeLast, mode.nDepth);
        gpprefs->Set(knPrefDepthDataLast, m_amm[m_immCurrent].nDepthData);
        gpprefs->Set(knPrefSizeDataLast, m_amm[m_immCurrent].nSizeData);
        gpprefs->Set(knPrefDegreeOrientationLast, mode.nDegreeOrientation);
	}

    gpprefs->Set(kszPrefKey, (const char *)gkey.ab);
    gpprefs->Set(knPrefUpdateDisplay, gcmsDisplayUpdate);

#if defined(WIN) && !defined(CE)
	if (gpdisp == NULL) {
		gpprefs->Set(knPrefScale, -1);
	} else {
        gpprefs->Set(knPrefScale, gpdisp->GetScale());
	}
#endif

    gpprefs->Save();
}

bool Game::GetVar(const char *pszName, char *pszBuff, int cbBuff)
{
	// 'system variable'

	if (*pszName == '$') {
		if (stricmp(pszName, "$ranktitle") == 0) {
			GetRankTitle(pszBuff, cbBuff);

		} else if (stricmp(pszName, "$grayscale") == 0) {
			strncpyz(pszBuff, gfGrayscale ? "1" : "0", cbBuff);

        } else if (stricmp(pszName, "$iphone") == 0) {
#if defined(IPHONE) || defined(__IPHONEOS__)
			strncpyz(pszBuff, "1", cbBuff);
#else
			strncpyz(pszBuff, "0", cbBuff);
#endif        

		} else if (stricmp(pszName, "$difficulty") == 0) {
			char *pszT = "0";
			switch (gwfHandicap) {
			case kfHcapEasy:
				pszT = "1";
				break;

			case kfHcapNormal:
				pszT = "2";
				break;

			case kfHcapHard:
				pszT = "3";
				break;
			}

			strncpyz(pszBuff, pszT, cbBuff);

		} else if (stricmp(pszName, "$demo") == 0) {
			strncpyz(pszBuff, gfDemo ? "1" : "0", cbBuff);

		} else {

			// Bogus request

			strncpyz(pszBuff, "?var?", cbBuff);
			return false;
		}

	// 'persistent variable'

	} else {
		const char *pszValue = m_dictPvars.Get(pszName);
		if (pszValue == NULL) {
			strncpyz(pszBuff, "?pvar?", cbBuff);
			return false;
		}
		strncpyz(pszBuff, pszValue, cbBuff);
	}

	return true;
}

bool Game::SetVar(const char *pszName, const char *pszValue)
{
	Assert(*pszName != '$', "Setting of sysvars not allowed.");
	return m_dictPvars.Set(pszName, pszValue);
}

} // namespace wi
