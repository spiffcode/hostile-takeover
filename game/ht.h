#ifndef __HT_H__
#define __HT_H__

namespace wi {

#define HOSTILE_TAKEOVER

// Beta timeout
//#define BETA_TIMEOUT

#ifdef DEBUG
#define INCL_TRACE
#define MP_DEBUG	// Multiplayer debugging
#endif

// Build types
// These flags are passed in by makefiles / devenv configurations

//#define ESD_BUILD // ESD release builds
//#define RETAIL_BUILD // Retail release builds
//#define DEV_BUILD // Developer build

// ESD_BUILD settings

#if defined(ESD_BUILD)
#define DRM
#endif

// RETAIL_BUILD settings

#if defined(RETAIL_BUILD)
#define DRM
#define DRM_KEYONLY
#endif

// DEV_BUILD settings

#if defined(DEV_BUILD)
#ifdef DEBUG
//#define INCL_VALIDATEHEAP
//#define INCL_TESTS
//#define INCL_MEMTRACE

#if defined(WIN) && !defined(CE)
//#define MP_DEBUG_SHAREDMEM
#define DEBUG_HELPERS
#endif
#endif

// MP stress

#if !defined(PIL) && !defined(CE) && !defined(IPHONE) && !defined(SDL)
#define MP_STRESS
#endif

// Enable stress testing
#define STRESS

// Enable this to allow little arrows to be drawn showing the paths being followed
#define DRAW_PATHS

// Enable this to draw a box around each tile a unit believes it is occupying
//#define DRAW_OCCUPIED_TILE_INDICATOR

// Enable this to draw a white pixel at the upper left corner of each tile
//#define MARK_TILE_BOUNDARIES

// Enable this to see which tiles are occupied according to the terrain map
//#define MARK_OCCUPIED_TILES

// Enable this to allow stats display to be toggled via the Find button
// <broken>
//#define STATS_DISPLAY

// Draw rects around invalid tiles
#define DRAW_UPDATERECTS

// Enable this to have HostOutputDebugString echo to hotsync log on Palm devices
//#define TRACE_TO_HOTSYNC_LOG

// Enable this to have HostOutputDebugString echo to database log
#define TRACE_TO_DB_LOG

// Maintain and check a ring buffer of old allocs
#if defined(WIN) && defined(DEBUG)
//#define CHECK_OLD_ALLOCS
#endif
#endif // defined(DEV_BULD)

// Things that are permanently on

#define DRAW_LINES // Enable this to allow lines to be drawn to visualize unit targeting
#define STATUS_LINE // Enable this to get certain status displayed at the bottom of the screen
//#define RALLY_POINTS

// Enable this to sychronize Updates between server and all clients and validate
// the Update results
//#define DETECT_SYNC_ERRORS

#ifdef DEBUG_HELPERS
class TriggerViewer;
class UnitGroupViewer;
class DelayedMessageViewer;
class CommandQueueViewer;
#endif

} // namespace wi
    
#include "inc/basictypes.h"
//#include "base/log.h"
#include "base/misc.h"
#include "mpshared/mpht.h"
#include "mpshared/misc.h"
#include "mpshared/enum.h"
#include "mpshared/ini.h"
#include "mpshared/decompress.h"
#include "game/missionlist.h"
#include "mpshared/netmessage.h"
#include "game/dragrect.h"
#include "base/thread.h"
#include "yajl/wrapper/jsontypes.h"

namespace wi {

class StateFrame;
class StateTracker;
class RefMap;

typedef void *FileHandle;	// hf
typedef word CacheHandle; // hc

// Game version

#define knVersion 1

#define MAKEDWORD(x) ((((dword)x) << 24) | (((dword)x) << 16) | (((dword)x) << 8) | (x))

#if defined(PIL) && defined(__CPU_68K) && defined(__GNUC__)
#define secCode1
#define secCode2 __attribute__((section("code2")))
#define secCode3 __attribute__((section("code3")))
#define secCode4 __attribute__((section("code4")))
#define secCode5 __attribute__((section("code5")))
#define secCode6 __attribute__((section("code6")))
#define secCode7 __attribute__((section("code7")))
#define secCode8 __attribute__((section("code8")))
#define secCode9 __attribute__((section("code9")))
#define secCode10 __attribute__((section("code10")))
#define secCode11 __attribute__((section("code11")))
#define secCode12 __attribute__((section("code12")))
#define secCode13 __attribute__((section("code13")))
#define secCode14 __attribute__((section("code14")))
#define secCode15 __attribute__((section("code15")))
#define secCode16 __attribute__((section("code16")))
#define secCode17 __attribute__((section("code17")))
#else
#define secCode1
#define secCode2
#define secCode3
#define secCode4
#define secCode5
#define secCode6
#define secCode7
#define secCode8
#define secCode9
#define secCode10
#define secCode11
#define secCode12
#define secCode13
#define secCode14
#define secCode15
#define secCode16
#define secCode17
#endif

#define secGame secCode9
#define secHost secCode17
#define secRect secCode5
#define secDibBitmap secCode5
#define secTBitmap secCode16
#define secIni secCode16
#define secFormMgr secCode5
#define secForm secCode10
#define secControl secCode2
#define secTimer secCode16
#define secTimerMgr secCode11
#define secEventMgr secCode4
#define secEmptyForm secCode4
#define secFont secCode6
#define secButtonControl secCode4
#define secPresetButtonControl secCode8
#define secLabelControl secCode4
#define secEditControl secCode4
#define secHelpControl secCode16
#define secHelpForm secCode13
#define secListControl secCode6
#define secSliderControl secCode13
#define secMisc secCode16
#define secGameOptionsForm secCode2
#define secInputPanelForm secCode2
#define secTileMap secCode2
#define secSimulation secCode12
#define secSimUIForm secCode8
#define secGob secCode12
#define secTankGob secCode5
#define secInfantryGob secCode5
#define secStructures secCode3
#define secLevel secCode11
#define secStateMachine secCode13
#define secAnimation secCode12
#define secFogMap secCode4
#define secTerrainMap secCode2
#define secSoundMgr secCode6
#define secSoundDevice secCode6
#define secColorMgr secCode2
#define secPackFile secCode13
#define secCacheMgr secCode13
#define secCheckBoxControl secCode11
#define secDisplay secCode7
#define secWaveOut secCode2
#define secMemMgr secCode16
#define secMultiplayer secCode7
#define secCommandQueue secCode11
#define secComm secCode7
#define secPlayer secCode13
#define secBitmapControl secCode2
#define secMiner secCode6
#define secUpdateMap secCode4
#define secGraffitiScrollControl secCode10
#define secInputUIForm secCode10
#define secMiniMapControl secCode10
#define secThunks secCode11
#define secArmThunks secCode11
#define secLoadSave secCode5
#define secStream secCode5
#define secOvermindGob secCode8
#define secGobMgr secCode4
#define secAnimGob secCode11
#define secScorchGob secCode11
#define secSurfaceDecalGob secCode4
#define secSceneryGob secCode4
#define secUnitGob secCode15
#define secTrigger secCode16
#define secAction secCode14
#define secCondition secCode11
#define secUnitGroup secCode13
#define secBuildMgr secCode14
#define secShell secCode13
#define secRawBitmap secCode13
#define secEcom secCode12
#define secPipMeterControl secCode13
#define secDamageMeterControl secCode13
#define secTowers secCode13
#define secStringTable secCode5
#define secAlertControl secCode16
#define secArtilleryGob secCode5
#define secCreditsControl secCode16
#define secPowerControl secCode16
#define secRadar secCode14
#define secWarehouse secCode14
#define secReplicatorGob secCode4
#define secObjectives secCode14
#define secCutScene secCode14
#define secBuildQueue secCode12
#define secBuilderGob secCode12
#define secDrm secCode16
#define secBtTransport secCode17
#define secTexAtlasMgr secCode13
#define secPreferences secCode9

// Performance options

const word kfPerfRocketShots = 0x0001;
const word kfPerfRocketTrails = 0x0002;
const word kfPerfRocketImpacts = 0x0004;
const word kfPerfShots = 0x0008;
const word kfPerfShotImpacts = 0x0010;
const word kfPerfSelectionBrackets = 0x0020;
const word kfPerfSmoke = 0x0040;
const word kfPerfEnemyDamageIndicator = 0x0080;
const word kfPerfScorchMarks = 0x0100;
const word kfPerfSymbolFlashing = 0x0200;
const word kfPerfAll = 0x03ff;
const word kfPerfMax = 0xffff;

// Handicap flags and values

const word kfHcapIncreasedFirePower = 0x0001;
const word kfHcapDecreasedTimeToBuild = 0x0002;
const word kfHcapIncreasedMinerLoadValue = 0x0004;
const word kfHcapShowEnemyBuildProgress = 0x0010;	// build progress & upgrading
const word kfHcapShowEnemyResourceStatus = 0x0020;	// power & credit symbol flash
const word kfHcapIncreasedArmor = 0x0040;

const word kfHcapEasy = kfHcapDecreasedTimeToBuild | kfHcapIncreasedArmor | kfHcapIncreasedFirePower | kfHcapIncreasedMinerLoadValue | kfHcapShowEnemyBuildProgress | kfHcapShowEnemyResourceStatus;
const word kfHcapNormal = kfHcapDecreasedTimeToBuild | kfHcapIncreasedArmor | kfHcapShowEnemyBuildProgress | kfHcapShowEnemyResourceStatus;
const word kfHcapHard = kfHcapShowEnemyBuildProgress;
const word kfHcapDefault = kfHcapNormal;

const int knDecreasedTimeToBuildPercent = -20;
const int knIncreasedFirePowerPercent = 20;
const int knIncreasedMinerLoadValuePercent = 20;
const int knDecreasedDamagePercent = 90;

// Color type

typedef struct {
    byte r;
    byte g;
    byte b;
} Color; // clr

// Network stuff

class ITransportCallback;
class IRoomCallback;
class ILobbyCallback;
class NetMessage;
class Transport;

const int kcbTransportName = 100;

enum TransportType {
	ktratBluetoothPAN,
	ktratBluetoothSer,
	ktratIR,
	ktratIP,
	ktratX,
};

const dword knTransportOpenResultSuccess = 0;
const dword knTransportOpenResultFail = 1;
const dword knTransportOpenResultNoNetwork = 2;
const dword knTransportOpenResultCantConnect = 3;
const dword knTransportOpenResultNotResponding = 4;
const dword knTransportOpenResultProtocolMismatch = 5;
const dword knTransportOpenResultServerFull = 6;

struct TransportDescription // trad
{
	TransportType trat;
	char szName[kcbTransportName];
	dword (*pfnOpen)(TransportDescription *ptrad, Transport **pptra);
	dword dwTransportSpecific;
};

struct DeleteRecord {
	DeleteRecord *pdrNext;
	bool fDeleted;
};

#define knWaitStrConnectToHost 1
#define knWaitStrClientDisconnecting 2
#define knWaitStrDisconnectingClients 3
#define knWaitStrDisconnectingClient 4
#define knWaitStrClosingTransport 5
#define knWaitStrBeginGameSearch 6
#define knWaitStrAdvertisingGame 7

const int kcbBluetoothMtuMin = 100;	// something >= our largest message but less than BT_L2CAP_MTU (672)
const int kctradMax = 10;

class Event;
class IGameCallback;
class Transport // tra
{
public:
    Transport();
	virtual ~Transport() {}
	virtual dword Open() = 0;
	virtual void Close() = 0;
    virtual bool IsClosed() = 0;
    virtual dword Login(const char *username, const char *token) = 0;
    virtual dword SignOut() = 0;
    virtual const char *GetAnonymousUsername() = 0;
    virtual dword JoinLobby(ILobbyCallback *plcb) = 0;
    virtual bool LeaveLobby() = 0;
    virtual dword CreateRoom(const char *roomname, const char *password,
            dword *roomid) = 0;
    virtual dword CanJoinRoom(dword roomid, const char *password) = 0;
    virtual dword JoinRoom(dword roomid, const char *password,
            IRoomCallback *prcb) = 0;
    virtual bool SendChat(const char *chat) = 0;
	virtual dword CreateGame(GameParams *params, PackId *ppackidUpgrade,
            dword *gameid) = 0;
    virtual dword CanJoinGame(dword gameid) = 0;
    virtual void LeaveRoom(dword hint) = 0;
    virtual dword JoinGame(dword gameid, dword roomid) = 0;
    virtual bool SendNetMessage(NetMessage *pnm) = 0;
    virtual void LeaveGame() = 0;
    virtual void OnEvent(Event *pevt) = 0;
    virtual void UpdateAllies(Side side, SideMask sidmAllies) = 0;
    virtual void DisconnectSharedAccounts() = 0;
	virtual ITransportCallback *SetCallback(ITransportCallback *ptcb);
    virtual ITransportCallback *GetCallback();
	virtual IGameCallback *SetGameCallback(IGameCallback *pgcb);
	virtual IGameCallback *GetGameCallback();

protected:
    ITransportCallback *m_ptcb;
    ILobbyCallback *m_plcb;
    IRoomCallback *m_prcb;
    IGameCallback *m_pgcb;
};

class TransportMgr // tram
{
public:
	virtual int GetTransportDescriptions(TransportDescription *atrad, int ctradMax);
};
extern TransportMgr gtram;

class IGameCallback {
public:
    virtual void OnReceiveChat(const char *player, const char *chat) = 0;
    virtual void OnNetMessage(NetMessage **ppnm) = 0;
    virtual void OnGameDisconnect() = 0;
};

class IRoomCallback {
public:
    virtual void OnAddGame(const char *player, dword gameid,
            const GameParams& params, dword minplayers, dword maxplayers,
            const char *title, dword ctotal) = 0;
    virtual void OnRemoveGame(dword gameid, dword ctotal) = 0;
    virtual void OnGameInProgress(dword gameid) = 0;
    virtual void OnGamePlayerNames(dword gameid, dword cnames,
            const PlayerName *anames) = 0;
    virtual void OnAddPlayer(const char *player) = 0;
    virtual void OnRemovePlayer(dword hint, const char *player) = 0;
    virtual void OnReceiveChat(const char *player, const char *chat) = 0;
    virtual void OnStatusComplete() = 0;
};

class ILobbyCallback {
public:
    virtual void OnLurkerCount(dword count) = 0;
    virtual void OnAddRoom(const char *name, dword roomid, bool priv,
            dword cPlayers, dword cGames) = 0;
    virtual void OnRemoveRoom(dword roomid) = 0;
    virtual void OnUpdateRoom(dword roomid, dword cPlayers, dword cGames) = 0;
    virtual void OnReceiveChat(const char *player, const char *chat) = 0;
};

class ITransportCallback {
public:
	virtual void OnStatusUpdate(char *pszStatus) = 0;
    virtual void OnConnectionClose() = 0;
    virtual void OnShowMessage(const char *message) = 0;
};

// Timer support

class Timer;
class TimerMgr // timm
{
public:
	TimerMgr() secTimerMgr;
	~TimerMgr() secTimerMgr;

	void AddTimer(Timer *ptmr, long ct) secTimerMgr;
	void RemoveTimer(Timer *ptmr) secTimerMgr;
	void SetTimerRate(Timer *ptmr, long ct) secTimerMgr;
    void BoostTimer(Timer *ptmr, long ct);
	long ScanDispatch(long tCurrent) secTimerMgr;
	virtual long GetTickCount() secTimerMgr;
	void Enable(bool fEnable) {
		m_fEnabled = fEnable;
	}
	void TriggerTimer(Timer *ptmr) secTimerMgr;
    bool IsAdded(Timer *ptmr) secTimerMgr;

private:
	Timer *m_ptmrFirst;
	Timer *m_ptmrNotifying;
	Timer *m_ptmrTriggerNext;
	bool m_fForceScan;
	bool m_fEnabled;
};
extern TimerMgr gtimm;

// Timer

class Timer // tmr
{
public:
    Timer() { m_ptimm = NULL; }
    virtual ~Timer();
	virtual void OnTimer(long tCurrent) = 0;

private:
    TimerMgr *m_ptimm;
	Timer *m_ptmrNext;
	Timer *m_ptmrPrev;
	long m_ctRate;
	long m_tTrigger;

	friend class TimerMgr;
};

// CONSIDER: add id to Timer instead

class ITimeout {
public:
	virtual void OnTimeout(int id) = 0;
};

class TimeoutTimer : public Timer {
public:
	TimeoutTimer() secMisc;

	~TimeoutTimer() {
		if (m_ptimo != NULL) {
			Stop();
        }
	}

	void Start(ITimeout *ptimo, int cms, int id = 0, bool fOneShot = true) {
		if (m_ptimo != NULL) {
			Stop();
        }
        m_id = id;
		m_fOneShot = fOneShot;
		m_ptimo = ptimo;
		gtimm.AddTimer(this, cms / 10);
	}

	void Stop() {
		if (m_ptimo == NULL) {
			return;
        }
		gtimm.RemoveTimer(this);
		m_ptimo = NULL;
	}

    bool IsStarted() {
        return m_ptimo != NULL;
    }

	virtual void OnTimer(long tCurrent) {
		ITimeout *ptimo = m_ptimo;
		if (m_fOneShot) {
			Stop();
        }
		ptimo->OnTimeout(m_id);
	}

private:
	ITimeout *m_ptimo;
	bool m_fOneShot;
    int m_id;
};

// Point

struct Point // pt
{
	int x, y;
};

// Rectangle class

class WRect;
class TRect;

class Rect // rc
{
public:
	int left;
	int top;
	int right;
	int bottom;

	bool Intersect(Rect *prcSrc1, Rect *prcSrc2) secRect;
	void Set(Point pt1, Point pt2) secRect;
	bool Subtract(Rect *prcSrc1, Rect *prcSrc2) secRect;
	void Add(Rect *prcSrc1, Rect *prcSrc2) secRect;
	void FromWorldRect(WRect *pwrc) secRect;
	void FromTileRect(TRect *ptrc) secRect;
	void Union(Rect *prc) secRect;
	bool Equal(Rect *prc) secRect;
	void GetCenter(Point *ppt) secRect;
    int GetDistance(int x, int y) secRect;

	void Offset(int dx, int dy)
	{
		left += dx;
		right += dx;
		top += dy;
		bottom += dy;
	}

	void Inflate(int cx, int cy)
	{
		left -= cx;
		right += cx;
		top -= cy;
		bottom += cy;
	}

	bool PtIn(int x, int y)
	{
		return (x >= left && x < right && y >= top && y < bottom);
	}

	bool RectIn(Rect *prc)
	{
		return (prc->left >= left && prc->top >= top && prc->right <= right && prc->bottom <= bottom);
	}

	void Set(int l, int t, int r, int b)
	{
		left = l;
		top = t;
		right = r;
		bottom = b;
	}

	void SetEmpty()
	{
		left = 0;
		right = 0;
		bottom = 0;
		top = 0;
	}

	bool IsEmpty()
	{
		return ((left >= right) || (top >= bottom));
	}
	int Width()

	{
		return right - left;
	}

	int Height()
	{
		return bottom - top;
	}
};
    
} // namespace wi
    
#include <htplatform.h>
#include "game/sprite.h"
#include "soundeffects.h"
#include "mpshared/packfile.h"
#include <rip.h>
#include "game/res.h"

namespace wi {

// PalmOS sdk 3.5 has min/max but sdk 4.0 doesn't

#if 0
#undef min
#undef max
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

// Ensures the same types are being compared
template<class T> inline T _min(const T a, const T b) { return (a > b) ? b : a; }
template<class T> inline T _max(const T a, const T b) { return (a < b) ? b : a; }

#define sign(x) ((x) > 0 ? 1 : (x) < 0 ? -1 : 0)

// For forward references

class Level;
class Gob;
class SimUIForm;
typedef int GobType; // gt
typedef word UpgradeMask;
struct StructConsts;
class UnitGob;
class Player;
class Trigger;
class UnitGroup;

const StateMachineId ksmidNull = (word)-1;

#ifdef MP_DEBUG_SHAREDMEM
extern Side gsideCurrent;
extern Gid ggidCurrent;
extern long gcupdCurrent;
extern GobType ggtCurrent;
extern bool gfInsideUpdate;
#endif
extern bool gfMultiplayer;
extern bool gfLockStep;

#ifdef DEBUG_HELPERS
char *PszFromUnitMask(UnitMask um);
char *PszFromCaSideMask(word wfCaSideMask);
#endif

class UpdateInterval // unvl
{
public:
	UpdateInterval() {
		m_cSkip = 0;
		m_cDecrementsLast = 0;
		m_cDecrements = 0;
	}

	void MinSkip() {
		m_cSkip = 0;
	}

	void MinSkip(int cSkip) {
		m_cSkip = _min(m_cSkip, cSkip);
	}

	bool Decrement() {
		if (gfMultiplayer) {
            // Can't gracefully support update intervals in the multiplayer
            // version due to synchronization issues caused by differing
            // client-side state such as visibility and gpplrLocal conditionals

			return true;
		}

		if (m_cSkip <= 0) {
			m_cSkip = 0x7fff;
			m_cDecrementsLast = m_cDecrements;
			m_cDecrements = 0;
			return true;
		}

		// Count Decs separately since m_cSkip can be arbitrarily Min'ed

		m_cSkip--;
		m_cDecrements++;
		return false;
	}

	int GetUpdateCount() {
		// A skip of 0 (meaning next update) is an update count of 1

		return m_cDecrementsLast + 1;
	}

private:
	int m_cSkip;
	int m_cDecrementsLast;

#ifdef DEV_BUILD
public:
#endif
	int m_cDecrements;
};

// World coordinates are high-byte = tile coord, low-byte = sub-tile coord
// Coordinate types and macros

const short ktcMax = 64;
const short kpcMax = ktcMax * 32;	// + 1 to allow for conversion of inclusive/exclusive rects
const WCoord kwcMax = ktcMax * 256;
const WCoord kwcTile = 0x0100;
const WCoord kwcTileHalf = kwcTile / 2;
const WCoord kwcTile16th = kwcTile / 16;

#ifdef IPHONE
// IPhone titlebar area eats input; this is compensation
#define kwcScrollLeftExtra WcFromTile16ths(5) 
#else
#define kwcScrollLeftExtra 0
#endif

#define kwcScrollBorderSize WcFromTile16ths(5)
#define kwcScrollStepSize	kwcTileHalf

extern short *gmpPcFromWc;		// [64 * 256];	// 16,384 * 2 = 32,768 bytes
extern WCoord *gmpWcFromPc;		// [64 * 32];	// 2,048 * 2 = 4,096 bytes

inline TCoord TcFromWc(WCoord wc) {
	return wc >> 8;
}

inline int PcFracFromUpc(int pc) {
	return gmpPcFromWc[gmpWcFromPc[pc] & 0xff];
}

inline WCoord WcFromPc(int pc) {
	// gmpWcFromPc has one extra entry to allow an exclusive bottom/right coord
	// to be mapped, hence the "<=" below

	Assert(abs(pc) <= kpcMax);
	if (pc >= 0)
		return gmpWcFromPc[pc];
	else
		return -gmpWcFromPc[-pc];
}

inline int PcFromWc(WCoord wc) {
	Assert(abs(wc) < kwcMax);
	if (wc >= 0)
		return gmpPcFromWc[wc];
	else
		return -gmpPcFromWc[-wc];
}

inline TCoord TcFromPc(int pc) {
	return TcFromWc(WcFromPc(pc));
}

inline WCoord WcFromUpc(int pc) {
	Assert(pc >= 0 && pc < kpcMax);
	return gmpWcFromPc[pc];
}

inline int PcFromUwc(WCoord wc) {
	Assert(wc >= 0 && wc < kwcMax);
	return gmpPcFromWc[wc];
}

inline TCoord TcFromUpc(int pc) {
	return TcFromWc(WcFromUpc(pc));
}

inline WCoord WcFromTc(TCoord tc) {
	return tc << 8;
}

inline WCoord WcFromTile16ths(int n) {
	return kwcTile16th * n;
}

inline WCoord WcTrunc(WCoord wc) {
	return wc & 0xff00;
}

inline WCoord WcFrac(WCoord wc) {
	return wc & 0x00ff;
}

inline int PcFromTc(TCoord tc) {
	return PcFromWc(WcFromTc(tc));
}

extern int gcxFullScreenFormDiv10;

inline int PcFromFc(int fc) {
	return (fc * gcxFullScreenFormDiv10) / 16;
}

struct TPoint // pt
{
	TCoord tx, ty;
};

class WRect : public Rect
{
public:
	void Set(WPoint wpt1, WPoint wpt2) secRect;
#if 0 // UNUSED
	void FromPixelRect(Rect *prc) secRect;
#endif
	void FromTileRect(TRect *ptrc) secRect;
};

class TRect : public Rect
{
public:
	void FromWorldRect(WRect *wrc) secRect;
};

// Specialized TRect. Its specialty is being 1/2 the size

struct TRectSmall
{
	char txLeft;
	char tyTop;
	char txRight;
	char tyBottom;

	void Union(TRectSmall *ptrc) {
		if (ptrc->txLeft >= ptrc->txRight || ptrc->tyTop >= ptrc->tyBottom)
			return;
		if (txLeft > ptrc->txLeft)
			txLeft = ptrc->txLeft;
		if (tyTop > ptrc->tyTop)
			tyTop = ptrc->tyTop;
		if (txRight < ptrc->txRight)
			txRight = ptrc->txRight;
		if (tyBottom < ptrc->tyBottom)
			tyBottom = ptrc->tyBottom;
	}

	void Offset(char tx, char ty) {
		txLeft += tx;
		tyTop += ty;
		txRight += tx;
		tyBottom += ty;
	}
};


// NOTE: predefined iclr constants (e.g., kiclrBlack) are in res.h
// except for these non-index-type virtual colors

#define kiclrShadow -1
#define kiclrShadow2x -2

// Fixed point number types and macros

#define INTBITS 11
#define FRACBITS 5
#define INTBITS32 (INTBITS + 16)

typedef long fix32; // INTBITS32 integer bits, FRACBITS fractional bits
typedef short fix;	// INTBITS integer bits, FRACBITS fractional bits

#define itofx(x) ((x) << 5)						// Integer to fixed point
#define itofx32(x) (((long)(x)) << 5)						// Integer to fixed point
#define ftofx(x) ((fix)((x) * 32))				// Float to fixed point
#define ftofx32(x) ((fix32)((x) * 32))				// Float to fixed point
#define dtofx(x) ((fix)((x) * 32))				// Double to fixed point
#define fxtoi(x) ((x) >> 5)						// Fixed point to integer
#define fx32toi(x) ((x) >> 5)						// Fixed point to integer
#define fxtof(x) ((float) (x) / 32)				// Fixed point to float
#define fxtod(x) ((double)(x) / 32)				// Fixed point to double
#define mulfx(x, y) ((((fix32)(x)) * (y)) >> 5)	// Multiply a fixed by a fixed
#define mulfx32(x, y) ((((fix32)(x)) * (y)) >> 5)	// Multiply a fixed by a fixed
#define divfx(x, y) ((((fix32)(x)) << 5) / (y))	// Divide a fixed by a fixed
#define divfx32(x, y) ((((fix32)(x)) << 5) / (y))	// Divide a fixed by a fixed
#define addfx(x, y) ((fix)(x) + (fix)(y))
#define addfx32(x, y) ((fix32)(x) + (fix32)(y))
#define subfx(x, y) ((x) - (y))
#define fracfx(x) ((x) & ((1 << FRACBITS) - 1)) // Fraction of a fixed point
fix32 sqrtfx(fix32 x) secMisc;
unsigned int isqrt(unsigned long val) secMisc;
int isqrt(int val1, int val2) secMisc;

#ifdef OLD_LINE_ITERATOR
// For iterating along a line at arbitrary (possibly fractional) increments

class LineIterator {
public:
	void Init(int x1, int y1, int x2, int y2, fix nStep) secMisc;

	bool Step() {
		if (m_cStepsRemaining == 0)
			return false;

		m_cStepsRemaining--;
		m_fx = addfx(m_fx, m_fdx);
		m_fy = addfx(m_fy, m_fdy);
		return true;
	}

	int GetX() {
		return fxtoi(m_fx);
	}

	int GetY() {
		return fxtoi(m_fy);
	}

	fix GetFixedX() {
		return m_fx;
	}

	fix GetFixedY() {
		return m_fy;
	}

private:
	fix m_fx, m_fy;
	fix m_fdx, m_fdy;
	int m_cStepsRemaining;
};
#endif	// OLD_LINE_ITERATOR

// For iterating along a line at arbitrary increments

class WLineIterator {
public:
	void Init(WCoord wx1, WCoord wy1, WCoord wx2, WCoord wy2, int nIncr) secMisc;

	bool Step() {
		if (m_cStepsRemaining == 0)
			return false;

		m_cStepsRemaining--;
		m_wx += m_wdx;
		m_wy += m_wdy;
		return true;
	}

	int GetWX() {
		return m_wx;
	}

	int GetWY() {
		return m_wy;
	}

	int GetStepsRemaining() {
		return m_cStepsRemaining;
	}
private:
	// OPT: can save space by using caller's m_wx, m_wy
	WCoord m_wx, m_wy;
	WCoord m_wdx, m_wdy;
	int m_cStepsRemaining;
};

// Handy functions for dealing with time and update conversions

const int kcmsUpdate = 80;		// 80ms per Update
const int kctUpdate = 8;
const int kctMapScrollRate = 8;	// TUNE:

inline int UpdFromSec(int csec) {
	// This is to catch any overflows. If we find ourselves concerned with
	// intervals > 327 seconds we can add a cast to LONG in here.
	Assert(csec < (32768 / 100));

	// We assume kcmsUpdate can be divided by 10 without loss of precision
	Assert(((kcmsUpdate / 10) * 10) == kcmsUpdate);

	return (csec * 100) / (kcmsUpdate / 10);
}

inline int UpdFromMsec(int cms) {
	return cms / kcmsUpdate;
}

// Transparent bitmap

struct ScanData // sd
{
	word ibaiclr;
	word cbaiclrUnpacked;
};

struct TBitmapEntry // tbe
{
	word cx;
	word cy;
	word yBaseline;
	word ibsd;
	word cbsd;
};
#define kcbTBitmapEntry (sizeof(word) * 5)

struct TBitmapHeader // tbh
{
	word ctbm;
	TBitmapEntry atbe[1];
};

// DibBitmap

#define kfDibFreeMem 0x0001
#define kfDibBackWindow 0x0002
#define kfDibWantScrolls 0x0004

class SubBitmap;
class DibBitmap
{
public:
	DibBitmap() secDibBitmap;
	virtual ~DibBitmap() secDibBitmap;

    virtual bool Init(char *pszFn) secDibBitmap;
    virtual bool Init(dword *pb, int cx, int cy) secDibBitmap;

    virtual void Blt(DibBitmap *pbmSrc, Rect *prcSrc, int xDst, int yDst) secDibBitmap;
    virtual void BltTiles(DibBitmap *pbmSrc, UpdateMap *pupd, int yTopDst) secDibBitmap;
    virtual void GetSize(Size *psiz) secDibBitmap;
    virtual void Fill(int x, int y, int cx, int cy, Color clr) secDibBitmap;
    virtual void Fill(int x, int y, int cx, int cy, dword clr) secDibBitmap;
    virtual void FillTo(class DibBitmap *pbmDst, int xDst, int yDst,
        int cxDst, int cyDst, int xOrigin = 0, int yOrigin = 0) secDibBitmap;
    virtual void Clear(Color clr) secDibBitmap;
    virtual void Shadow(int x, int y, int cx, int cy) secDibBitmap;
    virtual void DrawLine(short x1, short y1, short x2, short y2, Color clr) secDibBitmap;
    virtual void Scroll(Rect *prcSrc, int xDst, int yDst) secDibBitmap;

    virtual SubBitmap *Suballoc(int yTop, int cy) secDibBitmap;
    virtual SubBitmap *Suballoc(Rect rc) secDibBitmap;

    word GetFlags() {
        return m_wf;
    }
    void SetFlags(word wf) {
		m_wf = wf;
	}

#if defined(SDL)
    SDL_Texture *Texture() { return m_texture; }
    int Width() { return m_cx; }
    int Height() { return m_cy; }
#endif

private:
#if defined(SDL)
    SDL_Texture *m_texture;
    SDL_PixelFormat *m_ppfmt;
#endif
    word m_wf;
    int m_cx;
    int m_cy;

    friend class TBitmap;
};
DibBitmap *LoadDibBitmap(char *pszFn) secDibBitmap;
DibBitmap *CreateDibBitmap(dword *pb, int cx, int cy) secDibBitmap;

class SubBitmap : public DibBitmap
{
public:
    SubBitmap(Rect rc) : m_rc(rc) { }
    ~SubBitmap() {}

    void Blt(DibBitmap *pbmSrc, Rect *prcSrc, int xDst, int yDst);
    void Fill(int x, int y, int cx, int cy, Color clr);
    void Clear(Color clr);
    void DrawLine(short x1, short y1, short x2, short y2, Color clr);
private:
    Rect m_rc;
};

// Texture bitmap
// Stores information about a bitmap in a texture atlas

class TBitmap
{
public:
    TBitmap() secTBitmap;
    ~TBitmap() secTBitmap;

    bool Init(char *pszFn, int x, int y, int cx, int cy, int cxOrig, int cyOrig,
        int ccLeft, int ccTop, int *anSideMap) secTBitmap;

    int GetBaseline() secTBitmap;
    int GetAtlas(Side side) secTBitmap;
    void GetTextureSize(Size *psiz) secTBitmap;
    void GetSize(Size *psiz) secTBitmap;
    void GetPosition(Point *ppos) secTBitmap;

    void BltTo(class DibBitmap *pimgDst, int xDst, int yDst, Rect *prcSrc = NULL) secTBitmap;
    void BltTo(class DibBitmap *pimgDst, int xDst, int yDst, Side side, Rect *prcSrc = NULL) secTBitmap;
    void FillTo(class DibBitmap *pimgDst, int xDst, int yDst,
        int cxDst, int cyDst, int xOrigin = 0, int yOrigin = 0) secTBitmap;

    char *GetFileName() { return m_pszFn; }

    int ClippedLeft() { return m_ccLeft; }
    int ClippedTop() { return m_ccTop; }

    int Width() { return m_cxOrig; }
    int Height() { return m_cyOrig; }

private:
    DibBitmap *Flash() secTBitmap;
    
    int m_x, m_y, m_cx, m_cy;
    int m_cxOrig, m_cyOrig;
    int m_ccLeft, m_ccTop; // clipped
    int *m_anSideMap;
    char *m_pszFn;
};
TBitmap *CreateTBitmap(char *pszName) secTBitmap;

// Texture Atlas Manager

class TexAtlasMgr
{
public:
    TexAtlasMgr() secTexAtlasMgr;
    ~TexAtlasMgr() secTexAtlasMgr;

    bool Init() secTexAtlasMgr;
    TBitmap *CreateTBitmap(char *pszName) secTexAtlasMgr;
    void BltTo(TBitmap *ptbmSrc, DibBitmap *pbmDst, int xDst, int yDst, Side side, Rect *prcSrc) secTexAtlasMgr;

private:
    int m_natlases;
    json::JsonMap *m_json;
    DibBitmap **m_pbmAtlases;
};

// Font

#include <map>
typedef std::map<std::string, TBitmap *> FontMap;

class Font // fnt
{
public:
	Font();
	~Font();

	bool Load(char *pszFont);

	int GetHeight() {
        return m_cy;
	}

	int GetGlyphOverlap() {
		return m_nGlyphOverlap;
	}

	int GetLineOverlap() {
		return m_nLineOverlap;
	}

	int GetTextExtent(const char *psz);
	int GetTextExtent(const char *psz, int cch);
	int DrawText(DibBitmap *pbm, char *psz, int x, int y, int cch = -1,
            Color *pclr = NULL);
	void DrawText(DibBitmap *pbm, char *psz, int x, int y, int cx,
            int cy, bool fEllipsis = false);
    void DrawTextWithEllipsis(DibBitmap *pbm, char *psz, int cch,
            int x, int y, int cx, bool fForce = false);
	int CalcMultilineHeight(char *psz, int cxMultiline);
	int CalcBreak(int cx, char **psz, bool fChop = true);

private:
	char *FindNextNonBreakingChar(char *psz);
    TBitmap *GetTBitmap(char sz);
    bool TBitmapExists(char sz);

	int m_nGlyphOverlap;
	int m_nLineOverlap;
    int m_cxEllipsis;
    int m_cy;
    FontMap m_map;
    TBitmap *m_ptbmDefault;
};
Font *LoadFont(char *pszFont);

struct TileSetHeader // tseth
{
	word cTiles;
	word cxTile;
	word cyTile;
};
#define kcbTileSetHeader 6

struct MiniTileSetHeader // mtseth
{
	word offNext;
	word cTiles;
	word cxTile;
	word cyTile;
};

#define kcchFnTset 28
struct TileMapHeader // tmaph
{
	char szFnTset[kcchFnTset];
	word ctx;
	word cty;
};

#define kfTmapMapped 1
#define kfMiniTsetMapped 2

#define kcTileSetsMax 64
class TileMap // tmap
{
public:
	TileMap() secTileMap;
	~TileMap() secTileMap;

	bool Load(char *psz, Size *psizPlayfield) secTileMap;
	void GetMapSize(Size *psiz) secTileMap;
	void GetTCoordMapSize(Size *psiz) {
		psiz->cx = BigWord(m_ptmaph->ctx);
		psiz->cy = BigWord(m_ptmaph->cty);
	}
	void GetTileSize(Size *psiz) secTileMap;
	void Draw(DibBitmap *pbm, int x, int y, int cx, int cy, int xMap, int yMap, byte *pbFogMap, UpdateMap *pupd) secTileMap;
	MiniTileSetHeader *GetMiniTileSetHeader(int nScale) secTileMap;

private:
	FileMap m_fmapTmap;
	TileMapHeader *m_ptmaph;
	FileMap m_afmapTset[kcTileSetsMax];
	TileSetHeader *m_aptseth[kcTileSetsMax];
	MiniTileSetHeader *m_pmtseth;
	FileMap m_fmapMiniTset;
	int m_cTileSets;
	int m_cTiles;
	int m_cxTile;
	int m_cyTile;
	word m_wf;

public:
	dword **m_apbTileData;
	word *m_pwMapData;
	int m_ctx;
	int m_cty;
    DibBitmap *m_pbmDraw;
    DibBitmap **m_tiles;
};
TileMap *LoadTileMap(char *pszFn, Size *psizPlayfield) secTileMap;

// Fog map

struct RevealPattern // rvlp
{
	byte ctx;
	byte cty;
	byte ab[1];
};

#define kbOpaque 0
#define kbEmpty 15

const word kbfFogMask = 0x000f;
const int kcFogShift = 0;
#define IsFogOpaque(bFog) (((bFog) & kbfFogMask) == kbOpaque)

const word kbfGalaxiteMask = 0x00f0;
const int kcGalaxiteShift = 4;
#define HasGalaxite(bFog) (((bFog) & kbfGalaxiteMask) != 0)

//#define HasWall(wFog)     (((wFog) & kwfWallMask) != 0)
//const word kwfWallMask = 0x0f00;
//const int kcWallShift = 8;

class Stream;
class AnimationData;
class FogMap // fogm
{
public:
	FogMap() secFogMap;
	~FogMap() secFogMap;

	bool Init(Size *psizTile, Size *psizMap) secFogMap;
	bool LoadState(Stream *pstm) secFogMap;
	bool SaveState(Stream *pstm) secFogMap;
    bool IsCovered(TRect *ptrc) secFogMap;
	void RevealAll(UpdateMap *pupd) secFogMap;
	void Reveal(TCoord txMap, TCoord tyMap, RevealPattern *prvlp, UpdateMap *pupd, WCoord wxView, WCoord wyView) secFogMap;
	void Reveal(TRect *ptrc, UpdateMap *pupd, WCoord wxView, WCoord wyView) secFogMap;
	void Draw(DibBitmap *pbm, int xMap, int yMap, UpdateMap *pupd) secFogMap;
	void DrawGalaxite(DibBitmap *pbm, int xMap, int yMap, UpdateMap *pupd, byte *pbTrMap) secFogMap;

	byte *GetMapPtr() {
		return m_pbMap;
	}

	// Galaxite methods

	int GetGalaxite(TCoord tx, TCoord ty) secFogMap;
	void SetGalaxite(int nGx, TCoord tx, TCoord ty) secFogMap;
	bool DecGalaxite(TCoord tx, TCoord ty) secFogMap;
	void IncGalaxite(TCoord tx, TCoord ty) secFogMap;
	bool FindNearestGalaxite(TCoord tx, TCoord ty, TPoint *ptpt, bool fIgnoreFog) secFogMap;

	// Wall methods

	int GetWallHealth(TCoord tx, TCoord ty) secFogMap;
	void SetWallHealth(int nHealth, TCoord tx, TCoord ty) secFogMap;


private:
	int m_ctxMap;
	int m_ctyMap;
	int m_cxTile;
	int m_cyTile;
	byte m_mpSrcDstResult[256];
	byte *m_pbMap;
	TBitmap *m_aptbm[16];
	TBitmap *m_aptbmGalax[9];
	AnimationData *m_panidWalls;
};

// Directions

typedef char Direction; // dir

const Direction kdirInvalid = -1;
const Direction kdirN = 0;
const Direction kdirNE = 1;
const Direction kdirE = 2;
const Direction kdirSE = 3;
const Direction kdirS = 4;
const Direction kdirSW = 5;
const Direction kdirW = 6;
const Direction kdirNW = 7;

typedef int Direction16; // dir16
const Direction16 kdir16Invalid = -1;
const Direction16 kdir16N = 0;
const Direction16 kdir16NE = 2;
const Direction16 kdir16E = 4;
const Direction16 kdir16SE = 6;
const Direction16 kdir16S = 8;
const Direction16 kdir16SW = 10;
const Direction16 kdir16W = 12;
const Direction16 kdir16NW = 14;

// TerrainMap

struct PathHead // pathh
{
	PathHead *ppathhNext;
	int txLast;
	int tyLast;
	int cSteps;
	word offHead;
	word wScoreKnown;
	word wScore;
};

#define kbfMobileUnit 0x80
#define kbfStructure 0x40
#define kbfReserved 0x20
#define kbfLinked 0x10
#define kbfHead 0x08
#define kbfDirMask 0x07

class Path;
class TrackPoint
{
public:
	bool Init(Path *ppath, TCoord txFrom, TCoord tyFrom, int itptStart, int ctptFurtherStop) secTerrainMap;
	void InitFrom(TrackPoint *ptrkp) secTerrainMap;
	bool IsProgress(TrackPoint *ptrkpA) secTerrainMap;
	bool IsCloser(TrackPoint *ptrkpA) secTerrainMap;
	bool IsBetterSort(TrackPoint *ptrkpA) secTerrainMap;
	void GetClosestPoint(TPoint *ptpt) {
		*ptpt = m_tptClosest;
	}
	int GetClosestPointIndex() {
		return m_itptClosest;
	}
	void GetAfterPoint(TPoint *ptpt) {
		*ptpt = m_tptAfter;
	}
	void GetInitialPoint(TPoint *ptpt) {
		*ptpt = m_tptInitial;
	}

protected:
	TPoint m_tptInitial;
	TPoint m_tptClosest;
	TPoint m_tptBefore;
	TPoint m_tptAfter;
	int m_itptClosest;
	int m_n2Before;
	int m_n2After;
};

class TerrainMap;
class Path
{
public:
	Path() secTerrainMap;
	~Path() secTerrainMap;

	bool Init(TerrainMap *ptrmap, TCoord txStart, TCoord tyStart, Direction *adir, int cdir) secTerrainMap;
	int GetCount() secTerrainMap;
	bool GetPoint(int itpt, TPoint *ptpt, byte bf) secTerrainMap;
	bool GetPointRaw(int itpt, TPoint *ptpt) secTerrainMap;
	void SetCacheIndex(int itpt) secTerrainMap;
	int FindClosestPoint(TCoord txFrom, TCoord tyFrom, int itptStart, int ctptFurtherStop, TPoint *ptptClosest) secTerrainMap;
	bool LoadState(TerrainMap *ptrmap, Stream *pstm) secTerrainMap;
	bool SaveState(Stream *pstm) secTerrainMap;
	bool Append(Path *ppath) secTerrainMap;
	bool TrimEnd(int itptStart) secTerrainMap;
	Path *Clone() secTerrainMap;
	void GetStartPoint(TPoint *ptpt) secTerrainMap;
#ifdef DRAW_PATHS
	void Draw(DibBitmap *pbm, int xView, int yView, Side side) secTerrainMap;
#endif

protected:
	void CalcTo(int idir, TCoord *ptx, TCoord *pty) secTerrainMap;

	TerrainMap *m_ptrmap;
	int m_cdirs;
	Direction *m_adir;
	TCoord m_txStart, m_tyStart;
	int m_idirCache;
	TCoord m_txCache, m_tyCache;
};
Path *CreatePath(TerrainMap *ptrmap, TCoord txStart, TCoord tyStart, TPoint *ptpt, int ctpt) secTerrainMap;
Path *CreatePath(TerrainMap *ptrmap, TCoord txStart, TCoord tyStart, Direction *adir, int cdir) secTerrainMap;

// 0 = area
// 1 = open
// 2 = wall
// 3 = blocked

#define kttArea 0
#define kttOpen 1
#define kttWall 2
#define kttBlocked 3

struct TerrainMapHeader // trmaph
{
	word ctx;
	word cty;
};

class TerrainMap // trmap
{
public:
	TerrainMap() secTerrainMap;
	~TerrainMap() secTerrainMap;

	bool Init(char *pszFn) secTerrainMap;
	bool LoadState(Stream *pstm) secTerrainMap;
	bool SaveState(Stream *pstm) secTerrainMap;
	void SetFlags(int tx, int ty, int ctx, int cty, byte bf) secTerrainMap;
	bool TestFlags(int tx, int ty, int ctx, int cty, byte bf) secTerrainMap;
	void ClearFlags(int tx, int ty, int ctx, int cty, byte bf) secTerrainMap;
	bool IsOccupied(int tx, int ty, int ctx, int cty, byte bf) secTerrainMap;
	Path *FindPath(int txFrom, int tyFrom, int txTo, int tyTo, byte bfTerrainAvoid) secTerrainMap;
	Path *FindLinePath(TCoord txFrom, TCoord tyFrom, TCoord txTo, TCoord tyTo, byte bfTerrainAvoid) secTerrainMap;
	bool IsLineOccupied(TCoord txFrom, TCoord tyFrom, TCoord txTo, TCoord tyTo, byte bfTerrainAvoid) secTerrainMap;
	bool FindFirstUnoccupied(TCoord txFrom, TCoord tyFrom, TCoord txTo, TCoord tyTo, TCoord *ptxFree, TCoord *ptyFree) secTerrainMap;
	bool FindLastUnoccupied(TCoord txFrom, TCoord tyFrom, TCoord txTo, TCoord tyTo, TCoord *ptxFree, TCoord *ptyFree) secTerrainMap;
	bool IsBlocked(TCoord tx, TCoord ty, byte bf) secTerrainMap;
	bool GetFlags(int tx, int ty, byte *pbf) secTerrainMap;
	int GetTerrainType(TCoord tx, TCoord ty) secTerrainMap;

	byte *GetMapPtr() {
		return (byte *)(m_ptrmaph + 1);
	}

	TCoord GetWidth() {
		return m_ctx;
	}
	TCoord GetHeight() {
		return m_cty;
	}

#ifdef MP_DEBUG
	dword GetChecksum() secTerrainMap;
#endif

private:
	PathHead *AddPathHead(word wScoreKnown, word wScore, int cSteps, word offHead, int txNew, int tyNew) secTerrainMap;
	void RemovePathHead(PathHead *ppathh) secTerrainMap;
	Path *MakePath(TCoord txStart, TCoord tyStart, word off) secTerrainMap;

	FileMap m_fmap;
	TerrainMapHeader *m_ptrmaph;
	byte *m_abBuffer;
	int m_cbBuffer;
	int m_ctx;
	int m_cty;
	word *m_pwSquared;
	PathHead *m_ppathhList;
	PathHead *m_ppathhFreeList;
	int m_mpDirDelta[8];
	static word s_anScoreTerrain[4];
	static word s_anScoreVector[8];
	int m_cNodes;

	friend class Path;
};

// Animation & AnimationData

struct FrameData // frmd
{
    char szName[64];       // name of bm
    char szName2[64];      // name of bm2
	word ibm;			   // bitmap index
	word ibm2;			   // second bitmap index
	byte cHold;			   // frame delay
	char xOrigin;		   // x offset for drawing
	char yOrigin;		   // y offset for drawing
	char xOrigin2;		   // x offset for drawing second bitmap
	char yOrigin2;		   // y offset for drawing second bitmap
	byte bCustomData1;	   // first custom data value
	byte bCustomData2;	   // second custom data value
};
#define kcbFrameData 139

class StripData // stpd
{
private:
	char szName[26];	// name of the Strip
	byte cDelay;		// inter-frame delay (in ??? units)
	byte bfFlags;		// e.g., fLoop
	word cfrmd;			// count of FrameData structures
	FrameData afrmd[1];	// array of FrameData structures

public:
	const char *GetName() {
		return szName;
	}

	int GetDefaultDelay() {
		return cDelay;
	}

	byte GetFlags() {
		return bfFlags;
	}

	word GetFrameCount() {
		return BigWord(cfrmd);
	}

	FrameData *GetFrameData(int ifrmd) {
        return (FrameData *)(((byte *)afrmd) + ifrmd * kcbFrameData);
		//return &afrmd[ifrmd];
	}
};

// NOTE: All values are stored big-endian

struct AnimationFileHeader // anih
{
	dword cstpd;        // count of StripData structures
	dword aoffStpd[1];  // array of offsets to StripData structures
};

class AnimationData // anid
{
public:
	AnimationData() secAnimation;
	~AnimationData() secAnimation;

	bool Init(const char *pszAniName) secAnimation;
	int GetStripCount() secAnimation;
	int GetFrameCount(int nStrip) secAnimation;
	int GetStripIndex(const char *pszStripName) secAnimation;
	void GetFrameOrigin(int nStrip, int nFrame, Point *pptOrigin) secAnimation;
	void GetSpecialPoint(int nStrip, int nFrame, Point *pptSpecial) secAnimation;
	void GetBounds(int nStrip, int nFrame, Rect *prc) secAnimation;
	void DrawFrame(int nStrip, int nFrame, DibBitmap *pbm, int x, int y, Side side, Rect *prcSrc = NULL) secAnimation;
	int GetStripDelay(int nStrip) secAnimation;
	int GetFrameDelay(int nStrip, int nFrame) secAnimation;

private:
	TBitmap **m_aptbm;
	FileMap m_fmap;
	AnimationFileHeader *m_panih;
    int m_ctbm;
};
AnimationData *LoadAnimationData(const char *pszAniName) secAnimation;

const word kfAniLoop = 0x0001;
const word kfAniDone = 0x0002;
const word kfAniIgnoreFirstAdvance = 0x0004;
const word kfAniResetWhenDone = 0x0008;
const word kfAniFreeAnimationData = 0x8000;

class Animation // ani
{
public:
	Animation() {
		m_panid = NULL;
		m_nStrip = 0;
		m_nFrame = 0;
		m_cDelay = 0;
		m_wf = kfAniDone;
	}

	~Animation() {
		if (m_wf & kfAniFreeAnimationData)
			delete m_panid;
	}

	void Init(AnimationData *panid) secAnimation;
	bool Start(int nStrip = 0, int nFrame = 0, word wf = 0) secAnimation;
	bool Start(const char *pszStripName, word wf = 0) secAnimation;
	bool LoadState(Stream *pstm) secAnimation;
	bool SaveState(Stream *pstm) secAnimation;
#ifdef MP_DEBUG_SHAREDMEM
	void MPValidate(Animation *paniRemote);
#endif

	void Stop() {
		m_wf |= kfAniDone;
	}

	bool IsDone() {
		return (m_wf & kfAniDone) != 0;
	}

	void SetStrip(int nStrip) {
		Assert(nStrip >= 0 && nStrip < m_panid->GetStripCount());
		m_nStrip = nStrip;
	}

	void SetStrip(const char *pszStripName) secAnimation;

	int GetStrip() {
		return m_nStrip;
	}

	void SetFrame(int nFrame) {
		Assert(nFrame >= 0 && nFrame < m_panid->GetFrameCount(m_nStrip));
		m_nFrame = nFrame;
	}

	int GetFrame() {
		return m_nFrame;
	}

	void SetDelay(int cDelay) {
		m_cDelay = cDelay;
	}

	void GetSpecialPoint(Point *pptSpecial, int nFrame = -1) {
		if (nFrame < 0)
			nFrame = m_nFrame;
		m_panid->GetSpecialPoint(m_nStrip, nFrame, pptSpecial);
	}

	void GetBounds(Rect *prc) {
		m_panid->GetBounds(m_nStrip, m_nFrame, prc);
	}

	void Draw(DibBitmap *pbm, int x, int y, Side side = ksideNeutral, Rect *prcSrc = NULL) {
		m_panid->DrawFrame(m_nStrip, m_nFrame, pbm, x, y, side, prcSrc);
	}

	word GetFlags() {
		return m_wf;
	}

	int GetRemainingFrameTime() {
		return m_cCountdown;
	}

	AnimationData *GetAnimationData() {
		return m_panid;
	}

	bool Advance(int cAdvance) secAnimation;
	long GetRemainingStripTime() secAnimation;

private:
	AnimationData *m_panid;
	int m_nStrip;
	int m_nFrame;
	word m_wf;
	byte m_cCountdown;
	byte m_cDelay;
};

// Event Manager

struct Event { // evt
	int idf;
	int eType;
	union {
		int chr;
	};
	int x;
	int y;
	dword dw;
    dword ff;
    long ms;
};
#define kfEvtFinger 0x00000001
#define kfEvtCoalesce 0x00000002

// Custom events

#define gamePaintEvent 0x6000
#define penHoldEvent 0x6001
#define gameOverEvent 0x6002
#define penHoverEvent 0x6003
#define updateTriggersEvent 0x6004
#define gameSuspendEvent 0x6005
#define gameResumeEvent 0x6006
#define resizeEvent 0x6007
#define hidePinEvent 0x6008
#define cancelModeEvent 0x6009
#define nullEvent 0x600a
#define transportEvent 0x600b
#define runUpdatesNowEvent 0x600c
#define askStringEvent 0x600d
#define mpEndMissionWinEvent 0x600e
#define mpEndMissionLoseEvent 0x600f
#define mpShowObjectivesEvent 0x6010
#define checkGameOverEvent 0x6011
#define connectionCloseEvent 0x6012
#define showMessageEvent 0x6013
#define enableSoundEvent 0x6014
#define disableSoundEvent 0x6015

// gameOverEvent constants (placed in Event::dw)

enum {
	knGoSuccess,
	knGoFailure,
	knGoInitFailure,
	knGoAbortLevel,
	knGoRetryLevel,
	knGoAppStop,
	knGoLoadSavedGame,
};
#define knAppExit 0xBEEFFEED // modifer to AppStopEvent, used only in windows for WM_CLOSE.

#define kcevtPostMax 20

#define kfRedrawDirty 0x01
#define kfRedrawMax 0x02
#define kfRedrawBeforeTimer 0x04
#define kfRedrawBeforeInput 0x08
#define kfRedrawPaintSkipped 0x10

struct FlickVector {
    int GetMagnitude() {
        return dx * dx + dy * dy;
    }
    int dx;
    int dy;
    dword cms;
};
#define kcevtPenHistory 32 // keep power of 2
#define kcmsFlickQuantum 150
    
class EventMgr // evm
{
public:
	EventMgr() secEventMgr;
	~EventMgr() secEventMgr;

	bool PeekEvent(Event *pevt, long cWait = 0) secEventMgr;
	bool GetEvent(Event *pevt, long cWait = -1, bool fCheckPaints = true) secEventMgr;
	void PostEvent(Event *pevt, bool fCoalesce = true) secEventMgr;
	void PostEvent(int eType, bool fCoalesce = true) secEventMgr;
	bool DispatchEvent(Event *pevt) secEventMgr;
	void Init() secEventMgr;
	void SetPenEventInterval(word ctInterval) secEventMgr;
	void ClearAppStopping() secEventMgr;
    bool GetFlickVector(int nPen, FlickVector *pfliv);

	void SetAppStopping() {
		m_fAppStopping = true;
	}

	bool IsAppStopping() {
		return m_fAppStopping;
	}

	void SetRedrawFlags(word wfRedraw)
	{
		m_wfRedraw |= wfRedraw;
	}

	void ClearRedrawFlags(word wfRedraw)
	{
		m_wfRedraw &= ~wfRedraw;
	}

	word GetRedrawFlags()
	{
		return m_wfRedraw;
	}

private:
    bool CheckPaintFPS() secEventMgr;
    void UpdatePenHistory(Event *pevt) secEventMgr;    
    bool QueryPenHistory(int nPen, long t, Point *ppt);

    int m_ievtPen1Next;
    Event m_aevtPen1History[kcevtPenHistory];
    int m_ievtPen2Next;
    Event m_aevtPen2History[kcevtPenHistory];    
    bool m_fPenHistoryInitialized;
	Event m_evtPeek;
	bool m_fPeekKeep;
	Event m_aevtQ[kcevtPostMax];
	int m_cevt;
	bool m_fAppStopping;
	word m_wfRedraw;
	bool m_fTimingPenHold;
	long m_tPenDown;
	int m_xHold, m_yHold;
	word m_ctMoveEventInterval;
	word m_nctMoveEventFraction;
	long m_tLastMoveEvent;
};
extern EventMgr gevm;

//---------------------------------------------------------------------------
// StateMachine
// Portions Copyright (C) Steve Rabin, 2000

class StateMachine;
class StateMachineMgr;

// NOTE: be sure to update the corresponding string table in StateMachine.cpp

enum State { // st
	kstReservedNull,
	kstReservedZombie,
	kstReservedGlobal,
	kstGuard,
	kstMove,
	kstAttack,
	kstChase,
	kstIdle,
	kstDying,
	kstBuildOtherCompleting,
	kstBeingBuilt,
	kstHuntEnemies,
	kstProcessorGetMiner,
	kstProcessorPutMiner,
	kstProcessorTakeGalaxite,
	kstMinerMoveToProcessor,
	kstMinerRotateForEntry,
	kstMinerMine,
	kstMinerFindGalaxite,
	kstMinerFaceGalaxite,
	kstMinerApproachGalaxite,
	kstMinerSuck,
	kstMinerStepAside,
	kstChangeStatePendingFireComplete,
	kstContinueActionPendingFireComplete,
};

STARTLABEL(StateNames)
	LABEL(kstReservedNull)
	LABEL(kstReservedZombie)
	LABEL(kstReservedGlobal)
	LABEL(kstGuard)
	LABEL(kstMove)
	LABEL(kstAttack)
	LABEL(kstChase)
	LABEL(kstIdle)
	LABEL(kstDying)
	LABEL(kstBuildOtherCompleting)
	LABEL(kstBeingBuilt)
	LABEL(kstHuntEnemies)
	LABEL(kstProcessorGetMiner)
	LABEL(kstProcessorPutMiner)
	LABEL(kstProcessorTakeGalaxite)
	LABEL(kstMinerMoveToProcessor)
	LABEL(kstMinerRotateForEntry)
	LABEL(kstMinerMine)
	LABEL(kstMinerFindGalaxite)
	LABEL(kstMinerFaceGalaxite)
	LABEL(kstMinerApproachGalaxite)
	LABEL(kstMinerSuck)
	LABEL(kstMinerStepAside)
	LABEL(kstChangeStatePendingFireComplete)
	LABEL(kstContinueActionPendingFireComplete)
ENDLABEL(StateNames)

STARTLABEL(GobTypes)
    LABEL(kgtNone)
    LABEL(kgtShortRangeInfantry)
    LABEL(kgtLongRangeInfantry)
    LABEL(kgtHumanResourceCenter)
    LABEL(kgtSurfaceDecal)
    LABEL(kgtScenery)
    LABEL(kgtAnimation)
    LABEL(kgtReactor)
    LABEL(kgtProcessor)
    LABEL(kgtStructure)
    LABEL(kgtUnit)
    LABEL(kgtGalaxMiner)
    LABEL(kgtHeadquarters)
    LABEL(kgtResearchCenter)
    LABEL(kgtVehicleTransportStation)
    LABEL(kgtRadar)
    LABEL(kgtLightTank)
    LABEL(kgtMediumTank)
    LABEL(kgtMachineGunVehicle)
    LABEL(kgtRocketVehicle)
    LABEL(kgtTakeoverSpecialist)
    LABEL(kgtWarehouse)
    LABEL(kgtMobileHeadquarters)
    LABEL(kgtOvermind)
    LABEL(kgtTankShot)
    LABEL(kgtRocket)
    LABEL(kgtMachineGunTower)
    LABEL(kgtRocketTower)
    LABEL(kgtScorch)
    LABEL(kgtSmoke)
    LABEL(kgtPuff)
    LABEL(kgtBullet)
    LABEL(kgtArtillery)
    LABEL(kgtArtilleryShot)
    LABEL(kgtAndy)
    LABEL(kgtReplicator)
    LABEL(kgtActivator)
    LABEL(kgtFox)
    LABEL(kgtAndyShot)
ENDLABEL(GobTypes)

// Helper macros for sending messages
// (would be better as inline functions but I don't trust gcc to do the right thing)

#if 0 // not used
#define SendHitMsg(_gidReceiver, _pplrAttacker, _nDamage) { \
	Message msgT; \
	msgT.mid = kmidHit; \
	msgT.smidSender = _gidAttacker; \
	msgT.smidReceiver = _gidReceiver; \
	msgT.Hit.gidAssailant = _pplrAttacker->GetId(); \
	msgT.Hit.sideAttacker = _pplrAttacker->GetSide(); \
	msgT.Hit.nDamage = _nDamage; \
	gsmm.SendMsg(&msgT); \
}
#endif

#define SendMoveAction(_gidReceiver, _wx, _wy, _tcRadius, _wcMoveDistPerUpdate) { \
	Message msgT; \
	msgT.mid = kmidMoveAction; \
	msgT.smidSender = kgidNull; \
	msgT.smidReceiver = _gidReceiver; \
	msgT.MoveCommand.wptTarget.wx = _wx; \
	msgT.MoveCommand.wptTarget.wy = _wy; \
	msgT.MoveCommand.gidTarget = kgidNull; \
	msgT.MoveCommand.wptTargetCenter.wx = _wx; \
	msgT.MoveCommand.wptTargetCenter.wy = _wy; \
	msgT.MoveCommand.tcTargetRadius = _tcRadius; \
	msgT.MoveCommand.wcMoveDistPerUpdate = _wcMoveDistPerUpdate; \
	gsmm.SendMsg(&msgT); \
}

#define SendGuardAreaAction(_gidReceiver, _nArea) { \
	Message msgT; \
	msgT.mid = kmidGuardAreaAction; \
	msgT.smidSender = kgidNull; \
	msgT.smidReceiver = _gidReceiver; \
	msgT.GuardAreaCommand.nArea = _nArea; \
	gsmm.SendMsg(&msgT); \
}

#define SendGuardVicinityAction(_gidReceiver) { \
	Message msgT; \
	msgT.mid = kmidGuardVicinityAction; \
	msgT.smidSender = kgidNull; \
	msgT.smidReceiver = _gidReceiver; \
	gsmm.SendMsg(&msgT); \
}

#define SendMineCommand(_gidReceiver, _wx, _wy) { \
	Message msgT; \
	msgT.mid = kmidMineCommand; \
	msgT.smidSender = kgidNull; \
	msgT.smidReceiver = _gidReceiver; \
	msgT.MoveCommand.wptTarget.wx = _wx; \
	msgT.MoveCommand.wptTarget.wy = _wy; \
	msgT.MoveCommand.gidTarget = kgidNull; \
	msgT.MoveCommand.wptTargetCenter.wx = _wx; \
	msgT.MoveCommand.wptTargetCenter.wy = _wy; \
	msgT.MoveCommand.tcTargetRadius = 0; \
	msgT.MoveCommand.wcMoveDistPerUpdate = 0; \
	gsmm.SendMsg(&msgT); \
}

#define SendHuntEnemiesAction(_gidReceiver, _um) { \
	Message msgT; \
	msgT.mid = kmidHuntEnemiesAction; \
	msgT.smidSender = kgidNull; \
	msgT.smidReceiver = _gidReceiver; \
	msgT.HuntEnemiesCommand.um = _um; \
	gsmm.SendMsg(&msgT); \
}

// NOTE: The upgrade bits must not overlap the ksum bits used for structures
// because they are used as prerequisites. The InProgress version don't matter.

const word kfUpgradeHrc = 0x0001;
const word kfUpgradeVts = 0x0002;
const word kfUpgradeHrcInProgress = 0x0004;
const word kfUpgradeVtsInProgress = 0x0008;

struct DelayedMessage // dmsg
{
	Message msg;
	DelayedMessage *pdmsgNext;
};

const int knHandled = 1;
const int knNotHandled = 0;
const int knDeleted = -1;

// StateMachine block macros. One version with embedded tracing, one without.

#define BeginStateMachine   if (st == kstReservedGlobal) { if(0) {
#define State(stTest)       return knHandled; } } else if (st == stTest) { if(0) {

#if defined(DEBUG_HELPERS)
void Log(StateMachine *psm, Message *pmsg);
void Log(StateMachine *psm, State stOld, State stNew);

#define OnEnter             return knHandled; } else if (pmsg->mid == kmidReservedEnter) { \
							Log(this, pmsg);
#define OnExit              return knHandled; } else if (pmsg->mid == kmidReservedExit) { \
							Log(this, pmsg);
#define OnUpdate            return knHandled; } else if (pmsg->mid == kmidReservedUpdate) { \
							Log(this, pmsg); \
							if (m_fDebug) DebugBreak();
#define OnMsg(midTest)		return knHandled; } else if (pmsg->mid == midTest) { \
							Log(this, pmsg); \
							if (m_fDebug) DebugBreak();
#define SetState(stNext)    { m_stNext = stNext; m_fForceStateChange = true; m_unvl.MinSkip(); \
							Log(this, m_st, stNext); }
#define DiscardMsgs			return knHandled; } else { \
							Log(this, pmsg); \
							if (m_fDebug) DebugBreak();
#else
#define OnEnter             return knHandled; } else if (pmsg->mid == kmidReservedEnter) {
#define OnExit              return knHandled; } else if (pmsg->mid == kmidReservedExit) {
#define OnUpdate            return knHandled; } else if (pmsg->mid == kmidReservedUpdate) {
#define OnMsg(midTest)		return knHandled; } else if (pmsg->mid == midTest) {
#define SetState(stNext)    { m_stNext = stNext; m_fForceStateChange = true; m_unvl.MinSkip(); }
#define DiscardMsgs			return knHandled; } else {
#endif

#define EndStateMachineInherit(base)     return knHandled; } } else { \
							return (int)##base::ProcessStateMachineMessage(st, pmsg); } return (int)##base::ProcessStateMachineMessage(st, pmsg);
#if 0
#define EndStateMachine		return knHandled; } } else { Assert("Invalid State"); \
                            return knNotHandled; } return knNotHandled;
#else
#define EndStateMachine		return knHandled; } } else { return knNotHandled; } return knNotHandled;
#endif

class StateMachine // sm
{
	friend class StateMachineMgr;

public:
	StateMachine() secStateMachine;
	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secStateMachine;

#if defined(DEBUG_HELPERS)
	virtual char *GetName() = 0;
#endif

	UpdateInterval *GetUpdateInterval() {
		return &m_unvl;
	}

protected:
    int ProcessMessage(State st, Message *pmsg) {
        return ProcessStateMachineMessage(st, pmsg);
    }

	UpdateInterval m_unvl;
#ifdef MP_STRESS
public:
#endif
	State m_st;
#ifdef MP_STRESS
protected:
#endif
	State m_stNext;
	bool m_fForceStateChange;
#ifdef DEBUG_HELPERS
public:
	friend void Log(StateMachine *psm, Message *pmsg);
	bool m_fDebug;
	void EnableDebug(bool fDebug) {
		m_fDebug = fDebug;
	}

	friend void paint();
#endif
};

class StateMachineMgr // smm
{
public:
	StateMachineMgr() secStateMachine;
	~StateMachineMgr() secStateMachine;
	bool Init(TimerMgr *ptimm) secStateMachine;
	bool SaveState(Stream *pstm) secStateMachine;
	bool LoadState(Stream *pstm) secStateMachine;
	void ClearDelayedMessages() secStateMachine;
	void SendMsg(MessageId mid, StateMachineId smidReceiver) secStateMachine;
	void SendMsg(MessageId mid, StateMachineId smidSender, StateMachineId smidReceiver) secStateMachine;
	void SendMsg(Message *pmsg) secStateMachine;
	void PostMsg(Message *pmsg) secStateMachine;
	void PostMsg(MessageId mid, StateMachineId smidSender, StateMachineId smidReceiver) secStateMachine;
	void SendDelayedMsg(Message *pmsg, long tDelay) secStateMachine;
	void SendDelayedMsg(MessageId mid, long tDelay, StateMachineId smidSender, StateMachineId smidReceiver) secStateMachine;
	void DispatchDelayedMessages() secStateMachine;
	void RouteMessage(Message *pmsg) secStateMachine;
	virtual StateMachineId GetId(StateMachine *psm) = 0;
	virtual StateMachine *GetStateMachine(StateMachineId smid) = 0;

private:
	void ProcessStateChange(StateMachine *psm) secStateMachine;
	bool StoreDelayedMessage(Message *pmsg) secStateMachine;

private:
	DelayedMessage *m_pdmsgHead;
	TimerMgr *m_ptimm;

#ifdef DEBUG_HELPERS
	friend class DelayedMessageViewer;
#endif

#ifdef TRACKSTATE
    void TrackState(StateFrame *frame, Message *pmsg);
#endif 
};

const int kcmsgCommandQueueMax = 300;

class CommandQueue // cmdq
{
public:
	CommandQueue() secCommandQueue;
	~CommandQueue() secCommandQueue;

	bool Init(int cmsgMax = kcmsgCommandQueueMax) secCommandQueue;
	void Exit() secCommandQueue;

	bool LoadState(Stream *pstm) secCommandQueue;
	bool SaveState(Stream *pstm) secCommandQueue;
	void Enqueue(MessageId mid, StateMachineId smidReceiver) secCommandQueue;
	void Enqueue(Message *pmsg) secCommandQueue;

	void Clear() {
        m_cmsg = 0;
    }

    Message *GetFirst() {
        return m_amsg;
    }

	int GetCount() {
		return m_cmsg;
	}

private:
	void SetEntry(Message *pqmsg) secCommandQueue;

private:
	int m_cmsg;
	int m_cmsgMax;
	Message *m_amsg;
};


#define kcbPvarNameMax 21
#define kcbPvarValueMax kcbFilename

struct DictionaryEntry // de
{
	DictionaryEntry *pdeNext;
	char szName[kcbPvarNameMax];
	char szValue[kcbPvarValueMax];
};

class Dictionary // dict
{
public:
	Dictionary() secMisc;
	~Dictionary() secMisc;
	bool Init(Dictionary *pdict) secMisc;
	void Clear() secMisc;
	const char *Get(const char *pszName) secMisc;
	bool Set(const char *pszName, const char *pszValue) secMisc;
	bool Remove(const char *pszName) secMisc;
	bool LoadState(Stream *pstm) secMisc;
	bool SaveState(Stream *pstm) secMisc;

private:
	DictionaryEntry **Find(const char *pszName) secMisc;

	int m_cde;
	DictionaryEntry *m_pdeHead;
};

// Game

struct ModeMatch
{
	int imode;
	int nDepthData;
	int nSizeData;
};

#define kfGameRoleServer 2
#define kfGameMultiplayer 4
#define kfGameInitFonts 0x10
#define kfGameInitBitmap 0x20
#define kfGameInitDone 0x40

class MultiFormMgr;
class InputUIForm;
class Form;
class Chatter;
class Game // game
{
public:
	Game() secGame;
	~Game() secGame;

	bool Init(int imm) secGame;
	void Exit() secGame;
	void Shell() secGame;
	int RunSimulation(Stream *pstm, char *pszLevel, word wfRole, dword gameid,
            Chatter *chatter);
	bool FilterEvent(Event *pevt) secGame;
	void GetPlayfieldSize(Size *psiz) secGame;
	void RequestModeChange(int imm) secGame;
	SimUIForm *GetSimUIForm() secGame;
	InputUIForm *GetInputUIForm() secGame;
    Form *GetMiniMapForm() secGame;
	void SaveGame(Stream *pstm = NULL) secGame;
	char *GetNextLevel() secGame;
	void SetNextLevel(char *pszLevel) secGame;
	int PlayLevel(MissionIdentifier *pmiid, Stream *pstmSavedGame = NULL,
            int nRank = 0) secGame;
	int PlaySavedGame(Stream *pstm) secGame;
	bool IsMultiplayer() secGame;
	void SetGameSpeed(int t) secGame;
	void SavePreferences() secGame;
	bool SaveReinitializeGame() secGame;
	bool GetFormattedVersionString(char *pszVersion, char *pszOut) secGame;
	void ClearDisplay() secGame;
	bool GetVar(const char *pszName, char *pszBuff, int cbBuff) secGame;
	bool SetVar(const char *pszName, const char *pszValue) secGame;
	void CalcUnitCountDeltas(Level *plvl) secGame;
	bool IsVersionCompatibleWithExe(int nVersionCompareShip, int nVersionCompareMajor, char chVersionCompareMinor, bool fUpwardCompatOK) secGame;
	void ParseVersion(char *pszVersion, int *pnShipVersion, int *pnMajorVersion, char *pchMinorVersion) secGame;
	bool AskResignGame(bool fTellHost = true) secGame;
	bool AskObserveGame() secGame;
	bool CheckDatabaseVersion(const char *pszDir, char *pszPdb,
            bool fUpwardCompatOK) secGame;

	// ModeMatch helpers

	int GetModeMatchBest() {
		return m_immBest;
	}

	int GetModeMatchCurrent() {
		return m_immCurrent;
	}

	int GetModeMatchCount() {
		return m_cmm;
	}

	void GetModeMatch(int imm, ModeMatch *pmm) {
		*pmm = m_amm[imm];
	}

	word GetFlags() {
		return m_wf;
	}

    const MissionIdentifier& GetLastMissionIdentifier() {
        return m_miid;
    }

    void SkipSaveReinitializeGame() {
        m_fSkipSaveReinitialize = true;
    }

    dword GetGameId() {
        return m_gameid;
    }

    void ScheduleUpdateTriggers();
    void UpdateTriggers();

private:
	bool CheckMemoryAvailable() secGame;
	void Suspend() secGame;
	bool LoadPreferences() secGame;
	bool InitDisplay(int imm) secGame;
	int FindBestModeMatch2(int nDepthData, int nSizeData, int nDepthMode, int cxWidthModeMin, int cxWidthModeMax, byte bfMatch) secGame;
	int FindBestModeMatch(int nSizeDataAbove) secGame;
	void AddModeMatches(int nDepthData, int nSizeData, int nDepthOrGreater, int cxWidthOrGreater) secGame;
	bool LoadGameData() secGame;
    bool InitTexAtlasMgr() secGame;
	bool InitCoordMappingTables() secGame;
	bool InitSimulation(Stream *pstm, char *pszLevel, word wfRole,
            dword gameid, Chatter *chatter);
	void ExitSimulation() secGame;
	ddword IsDataPresent(int cBpp) secGame;
	bool InitMemMgr() secGame;
	bool InitMultiFormMgr() secGame;

    dword m_gameid;
	int m_cmm;
	int m_cmmAlloc;
	ModeMatch *m_amm;
	int m_immCurrent;
	int m_immBest;
	SimUIForm *m_pfrmSimUI;
	InputUIForm *m_pfrmInputUI;
	Form *m_pfrmMiniMap;
	Size m_sizPlayfield;
	word m_wf;
	bool m_fSimUninitialized;
	char m_szNextLevel[kcbFilename];
    MissionIdentifier m_miid;
	Dictionary m_dictPvars;
    bool m_fSkipSaveReinitialize;
    bool m_fUpdateTriggers;
};
extern Game ggame;

void GameMain(char *psz) secGame;

enum PlayMode {
	kpmSavedGame,
	kpmNormal
};

class Shell
{
public:
	Shell() secShell;
	bool Init() secShell;
	void Exit() secShell;
	void Launch(bool fLoadReinitializeSave = false,
            MissionIdentifier *pmiid = NULL) secShell;
	int PlayGame(PlayMode pm, MissionIdentifier *pmiid, Stream *pstm,
            int nRank) secShell;

private:
    bool DoPlay();
	bool PlayChallengeLevel(bool fStory = false) secShell;
	bool BeginNewGame() secShell;
	bool PlaySinglePlayer(const PackId *ppackid) secShell;
	bool PlayMultiplayer(const PackId *ppackid) secShell;
    void DownloadMissionPack() secShell;
};
extern Shell gshl;

#define knStageNone 0
#define knStageCollect 1
#define knStageCheckDraw 2

// UpdateMap

struct MapInfo
{
	int cxLeftTile;
	int cyTopTile;
	int cxRightTile;
	int cyBottomTile;
	int ctxInside;
	int ctyInside;
};

class UpdateMap
{
public:
	UpdateMap() secUpdateMap;
	~UpdateMap() secUpdateMap;

	bool Init(Size *psiz) secUpdateMap;
	void Reset() secUpdateMap;
	void InvalidateRect(Rect *prc = NULL) secUpdateMap;
	void InvalidateMapTileRect(TRectSmall *ptrc) secUpdateMap;
	bool EnumUpdateRects(bool fFirst, Rect *prcBounds, Rect *prc) secUpdateMap;
	bool Scroll(int dx, int dy) secUpdateMap;
	bool *GetInvalidMap() secUpdateMap;
	void GetMapSize(Size *psiz) secUpdateMap;
	MapInfo *GetMapInfo() secUpdateMap;
	bool IsInvalid() secUpdateMap;
	void SetViewOrigin(int xOrigin, int yOrigin) secUpdateMap;
	void Validate() secUpdateMap;
	void InvalidateTile(TCoord tx, TCoord ty) secUpdateMap;
	void StartMergeDamagedInvalid() secUpdateMap;
	void EndMergeDamagedInvalid() secUpdateMap;

	bool IsDamagedInvalid()
	{
		return m_fInvalidDamage;
	}

	bool IsRectInvalid(Rect *prc)
	{
		TRectSmall trc;
		CalcTileRect(prc, &trc);
		return IsTileRectInvalid(&trc);
	}

	bool IsRectInvalidAndTrackDamage(Rect *prc)
	{
		TRectSmall trc;
		CalcTileRect(prc, &trc);
		bool fNewInvalid;
		return IsTileRectInvalidAndTrackDamage(&trc, &fNewInvalid);
	}
	bool IsMapTileRectInvalidAndTrackDamage(TRectSmall *ptrc, bool *pfNewInvalid) secUpdateMap;

private:
	bool EnumRowRects(bool fFirst, Rect *prcBounds, Rect *prc) secUpdateMap;
	void InvalidateTileRect(TRectSmall *ptrc) secUpdateMap;
	bool IsTileRectInvalidAndTrackDamage(TRectSmall *ptrc, bool *pfNewInvalid) secUpdateMap;
	bool IsTileRectInvalid(TRectSmall *ptrc) secUpdateMap;
	void CalcTileRect(Rect *prc, TRectSmall *ptrc) secUpdateMap;
	void SetMapOffset(int xMapOffset, int yMapOffset, bool fInvalidate = true) secUpdateMap;

	Rect m_rcDib;
	int m_ctx;
	int m_cty;
	int m_txOrigin;
	int m_tyOrigin;
	int m_xOriginView;
	int m_yOriginView;
	bool *m_afInvalid;
	bool *m_afInvalidDamage;
	int m_xMapOffset;
	int m_yMapOffset;
	bool m_fInvalid;
	bool m_fInvalidDamage;
	bool m_fMergeDamage;
	MapInfo m_mnfo;
};
    
class FlickScroller // flics
{
public:
    FlickScroller();
    bool Init(int nPen, float flMultiplier = 1.0f,
              float flDecayPercent = 0.01f,
              float cmsDecaySpan = kcmsFlickQuantum / 10, bool fChoose = true);
    void Clear();
    bool GetPosition(Point *ppt);
    bool HasMagnitude();
    
private:
    bool CheckMagnitude(float dx, float dy);
    
    FlickVector m_fliv;
    long m_msStart;
    float m_flMultiplier;
    float m_flDecayPercent;
    dword m_cmsDecaySpan;
    bool m_fHasMagnitude;
};  

// Form Manager

#define kcFormsMax 16

#define kfFrmmNoScroll 1

class Form;
class FormMgr // frmm
{
public:
	FormMgr() secFormMgr;
	virtual ~FormMgr() secFormMgr;

	virtual bool Init(DibBitmap *pbm, bool fFreeDib) secFormMgr;
	virtual void AddForm(Form *pfrm) secFormMgr;
	virtual void RemoveForm(Form *pfrm) secFormMgr;
	virtual bool EcomSuppressed() secFormMgr;
	virtual bool CookEvent(Event *pevt) secFormMgr;
	virtual Form *GetFormPtr(word idf) secFormMgr;
	virtual Form *LoadForm(IniReader *pini, word idf, Form *pfrm) secFormMgr;
	virtual void Paint(bool fScrolled, Rect *prcOpaqueStart) secFormMgr;
	virtual void Scroll(int dx, int dy) secFormMgr;
	virtual bool HasCapture() secFormMgr;
    virtual Form *GetFormCapture() secFormMgr;
	virtual Form *GetModalForm() secFormMgr;
	virtual void InvalidateRect(Rect *prc) secFormMgr;
	virtual DibBitmap *GetDib() secFormMgr;
	virtual void CalcOpaqueRect(Form *pfrmStop, Rect *prcOpaqueStart, Rect *prcResult) secFormMgr;
	virtual Form *GetFocus() secFormMgr;
    virtual void FrameStart() secFormMgr;
    virtual void FrameComplete() secFormMgr;
    virtual void BreakCapture();
    
	void SetFlags(word wf) {
		m_wf = wf;
	}
	word GetFlags() {
		return m_wf;
	}
	int GetFormCount() {
		return m_cfrm;
	}

protected:
    Form *FindPen2Form();
	bool CookPenEvent(Event *pevt) secFormMgr;
	bool CookKeyEvent(Event *pevt) secFormMgr;
	virtual bool BltTo(DibBitmap *pbmDst, int yTop, bool fScrolled) secFormMgr;
	virtual bool ScrollBits() secFormMgr;

	word m_wf;
	word m_idfCapture;
    int m_cCaptureDowns;
	int m_cfrm;
	Form *m_apfrm[kcFormsMax];
	DibBitmap *m_pbm;
	UpdateMap *m_pupd;
	bool m_fFreeDib;
	int m_dxScrollAccumulate;
	int m_dyScrollAccumulate;

public:
	UpdateMap *GetUpdateMap() {
		return m_pupd;
	}

	friend class MultiFormMgr;
};

#define kcFormMgrMax 5
class MultiFormMgr : public FormMgr
{
public:
	MultiFormMgr() secFormMgr;
	virtual ~MultiFormMgr() secFormMgr;

	void AddFormMgr(FormMgr *pfrmm, Rect *prc) secFormMgr;
	void RemoveFormMgr(FormMgr *pfrmm) secFormMgr;
	void DrawFrame(bool fForceBackBufferValid, bool fPaint = true) secFormMgr;
	void CheckSetRedrawDirty() secFormMgr;
    bool IsInvalid() secFormMgr;
	bool GetFormMgrRect(FormMgr *pfrmm, Rect *prc) secFormMgr;

#ifdef STATS_DISPLAY
	int GetUpdateRectCount() secFormMgr;
#endif

	virtual void AddForm(Form *pfrm) secFormMgr;
	virtual void RemoveForm(Form *pfrm) secFormMgr;
	virtual bool EcomSuppressed() secFormMgr;
	virtual bool CookEvent(Event *pevt) secFormMgr;
	virtual Form *GetFormPtr(word idf) secFormMgr;
	virtual bool Paint(bool fForceBackBufferValid) secFormMgr;
	virtual void InvalidateRect(Rect *prc) secFormMgr;
	virtual void CalcOpaqueRect(Form *pfrmStop, Rect *prcOpaqueStart, Rect *prcResult) secFormMgr;
	virtual Form *GetFocus() secFormMgr;

private:
#ifdef DRAW_UPDATERECTS
	void DrawUpdateRects(UpdateMap *pupd, int yTop) secFormMgr;
#endif
	virtual bool BltTo(DibBitmap *pbmDst, int yTop, bool fScrolled) secFormMgr;
    bool StealCapturePen2(Event *pevt, FormMgr *pfrmmChildCapture);

	int m_cfrmm;
	FormMgr *m_apfrmm[kcFormMgrMax];
	Rect m_arcFormMgr[kcFormMgrMax];
    Event m_evtPen1Down;
};

// Form

#define kcControlsMax 32
#define kfFrmDeleted 0x01
#define kfFrmDoModal 0x02
#define kfFrmVisible 0x04
#define kfFrmPenInside 0x08
#define kfFrmScaleCoords 0x40
#define kfFrmAutoTakedown 0x80
#define kfFrmTranslucent 0x100
#define kfFrmNoFocus 0x200
#define kfFrmShowSound 0x400
#define kfFrmNoEcom 0x800
#define kfFrmTopMost 0x1000
#define kfFrmDemandPen2 0x2000

#define knNotifySelectionChange 0
#define knNotifySelectionTap 1

class IControlEventHandler {
public:
	virtual void OnControlSelected(word nSelect) = 0;
	virtual bool OnControlHeld(word idc) = 0;
	virtual void OnControlNotify(word idc, int nNotify) = 0;
};

class Control;
class Form : public IControlEventHandler
{
public:
	Form() secForm;
	virtual ~Form() secForm;

	virtual bool Init(FormMgr *pfrmm, IniReader *pini, word idf) secForm;
    virtual bool OnFilterEvent(Event *pevt) { return false; }
	virtual bool EventProc(Event *pevt) secForm;
	virtual bool DoModal(int *pnResult = NULL, Sfx sfxShow = ksfxGuiFormShow, Sfx sfxHide = ksfxGuiFormHide) secForm;
	virtual bool OnHitTest(Event *pevt) secForm;
	virtual bool OnKeyTest(Event *pevt) secForm;
	virtual bool OnPenEvent(Event *pevt) secForm;
	virtual void OnUpdateMapInvalidate(UpdateMap *pupd, Rect *prcOpaque) secForm;
	virtual void OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd) secForm;
	virtual void OnPaint(DibBitmap *pbm) secForm;
	virtual void OnPaintControls(DibBitmap *pbm, UpdateMap *pupd) secForm;
	virtual void OnScroll(int dx, int dy) secForm;
	virtual void ScrollInvalidate(UpdateMap *pupd) secForm;
	virtual void InvalidateRect(Rect *prc) secForm;
    virtual void FrameStart() secForm;
    virtual void FrameComplete() secForm;
    virtual Control *HitTestControls(Event *pevt);
    virtual void BreakCapture();

	// IControlEventHandler methods

	virtual void OnControlSelected(word idc) secForm;
	virtual bool OnControlHeld(word idc) secForm;
	virtual void OnControlNotify(word idc, int nNotify) secForm;

	FormMgr *GetFormMgr() secForm;

	word GetId() secForm;
	word GetFlags() secForm;
	void SetFlags(word wf) secForm;
	void GetRect(Rect *prc) secForm;
	void SetRect(Rect *prc) secForm;
	void Show(bool fShow) secForm;
	void EndForm(int nResult = 0) secForm;
	Control *GetControlPtr(word idc) secForm;
	bool IsControlInside(Control *pctl) secForm;
	bool AddControl(Control *pctl) secForm;
	void SetUserDataPtr(void *pUserData) secForm;
	void *GetUserDataPtr() secForm;
    Form *FindPen2Form();

    bool HasCapture() {
        return m_pfrmm->GetFormCapture() == this;
    }

    Control *GetControlCapture() {
        return m_pctlCapture;
    }

    int GetResult() {
        return m_nResult;
    }

protected:
	bool InitFromProperties(FormMgr *pfrmm, word idf, IniReader *pini, char *pszForm) secForm;

	FormMgr *m_pfrmm;
	TBitmap *m_ptbm;
	Control *m_pctlCapture;
	Rect m_rc;
	int m_nResult;
	word m_wf;
	word m_idf;
	word m_idcDefault;
	word m_idcLast;
	int m_cctl;
	Sfx m_sfxShow;
	Sfx m_sfxHide;
	Control *m_apctl[kcControlsMax];
	int m_iclrBack;
	void* m_pUserData;

	friend class FormMgr;
};

inline FormMgr *Form::GetFormMgr()
{
	return m_pfrmm;
}

// DialogForm

#define kcyTitle 10

class DialogForm : public Form
{
public:
	DialogForm() secForm;
	~DialogForm() secForm;

	void SetBackgroundColorIndex(int iclr) secForm;
	void SetTitleColor(Color clr) secForm;
	void SetBorderColorIndex(int iclr) secForm;
	void SetClearDibFlag() secForm;

	// Form overrides

	virtual void OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd) secForm;
	virtual void OnPaint(DibBitmap *pbm) secForm;
	virtual bool OnPenEvent(Event *pevt) secForm;
	virtual bool DoModal(int *pnResult = NULL, Sfx sfxShow = ksfxGuiFormShow, Sfx sfxHide = ksfxGuiFormHide) secForm;

private:
	Color m_clrTitle;
	int m_iclrBorder;
	int m_iclrBackground;
	bool m_fClearDib;
};

inline void DialogForm::SetTitleColor(Color clr) {
	m_clrTitle = clr;
}

inline void DialogForm::SetBorderColorIndex(int iclr) {
	m_iclrBorder = iclr;
}

inline void DialogForm::SetClearDibFlag() {
	m_fClearDib = true;
}

// ShellForm

class ShellForm : public Form, public Timer
{
public:
	ShellForm() secForm;
	virtual ~ShellForm() secForm;

	// Doesn't match Form's DoModal signature so it's not really an override

	virtual bool DoModal(int *pnResult = NULL, bool fAnimate = true, bool fShowSound = true) secForm;

	// Form overrides

	virtual bool Init(FormMgr *pfrmm, IniReader *pini, word idf) secForm;
	virtual void OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd) secForm;

	// Timer interface

	virtual void OnTimer(long tCurrent) secForm;
    virtual void OnZipDone() {}

protected:
	bool m_fAnimate;

private:
	bool m_fCached;
	int m_axDst[kcControlsMax];
	bool m_fTimerEnabled;
	int m_cctlToZip;
	long m_tLast;
};

// InGameOptions

class InGameOptionsForm : public ShellForm
{
public:
	virtual bool Init(FormMgr *pfrmm, IniReader *pini, word idf);
	virtual void OnControlSelected(word idc);

private:
	void InitResettableControls();
	void UpdateLabels();

	bool m_fLassoSelection;
    bool m_fMuteSound;
	long m_tGameSpeed;
	word m_wfHandicap;
    double m_nScrollSpeed;
    int m_cmsMaxFPS;
};

// Control

#define knSelDownInside 0
#define knSelUpInside 1
#define knSelUpOutside 2
#define knSelMoveInside 3
#define knSelMoveOutside 4
#define knSelHoldInside 5

#define kfCtlVisible 1
#define kfCtlSet 2
#define kfCtlRedraw 4
#define kfCtlDisabled 8
#define kfCtlUseSide1Colors 16

class Control
{
public:
	Control() secControl;
	virtual ~Control() secControl;

	virtual bool Init(Form *pfrm, word idc, int x, int y, int cx, int cy) secControl;
	virtual bool Init(Form *pfrm, IniReader *pini, FindProp *pfind) secControl;
	virtual void OnPaint(DibBitmap *pbm) secControl;
	virtual void OnPenEvent(Event *pevt) secControl;
	virtual void OnSelect(int nSelect) secControl;
    virtual int OnHitTest(Event *pevt) secControl;
	virtual void Invalidate() secControl;
    virtual void GetFingerRect(Rect *prc) secControl;
    virtual void OnBreakCapture() secControl;

	word GetId() secControl;
	void GetRect(Rect *prc) secControl;
	void SetRect(Rect *prc, bool fCompareRect = true) secControl;
	void SetPosition(int x, int y) secControl;
	void Show(bool fShow) secControl;
	word GetFlags() secControl;
	void SetFlags(word wf) secControl;
	void SetEventHandler(IControlEventHandler *pceh) secControl;
	void Enable(bool fEnable) secControl;

protected:
	Rect m_rc;
	word m_wf;
	word m_idc;
	Form *m_pfrm;
	IControlEventHandler *m_pceh;

	friend class Form;
};

inline void Control::SetEventHandler(IControlEventHandler *pceh) {
	m_pceh = pceh;
}

// Button Control

class ButtonControl : public Control // btn
{
public:
	static bool InitClass() secButtonControl;
	static void ExitClass() secButtonControl;

	ButtonControl() secButtonControl;
	virtual ~ButtonControl() secButtonControl;

	virtual bool Init(Form *pfrm, word idc, int x, int y, int cx, int cy,
			char *pszLabel, int nfnt, char *szFnUp = NULL, char *szFnDown = NULL, char *szFnDisabled = NULL) secButtonControl;
	virtual bool Init(Form *pfrm, IniReader *pini, FindProp *pfind) secButtonControl;
	virtual void OnPaint(DibBitmap *pbm) secButtonControl;
	virtual void OnSelect(int nSelect) secButtonControl;

	void SetText(char *psz) secButtonControl;

private:
	virtual bool Init(char *pszLabel, int nfnt, char *szFnUp, char *szFnDown, char *szFnDisabled, bool fCenter) secButtonControl;

protected:
	TBitmap *m_ptbmUp;
	TBitmap *m_ptbmDown;
	TBitmap *m_ptbmDisabled;
	int m_nfnt;
	char *m_szLabel;

private:
	static TBitmap *s_ptbmLeftUp;
	static TBitmap *s_ptbmMidUp;
	static TBitmap *s_ptbmRightUp;
	static TBitmap *s_ptbmLeftDown;
	static TBitmap *s_ptbmMidDown;
	static TBitmap *s_ptbmRightDown;
};

class RadioButtonBarControl : public Control
{
public:
	RadioButtonBarControl();
	virtual ~RadioButtonBarControl();

	virtual bool Init(Form *pfrm, IniReader *pini, FindProp *pfind);
    virtual bool Init(const char *pszLabel, int ifnt, int isel);
	virtual void OnPaint(DibBitmap *pbm);
    virtual void OnPenEvent(Event *pevt);

    int GetSelectionIndex();
    void SetSelectionIndex(int isel);

private:
    void AddLabel(const char *psz, int cch);
    void GetCellRects(int icell, Rect *prcInner, Rect *prcOuter);
    void GetOuterCellSize(int icell, Size *psiz);

    int m_ifnt;
    int m_isel;
    int m_cLabels;
    const char *m_apszLabels[20];
};

class SilkButtonControl : public Control
{
public:
	~SilkButtonControl() secButtonControl;

public:
	virtual void OnSelect(int nSelect) secButtonControl;
};

inline SilkButtonControl::~SilkButtonControl() {
}

#if 0 // not used now
// PresetButtonControl

class PresetButtonControl : public ButtonControl // btn
{
public:
	virtual void OnPaint(DibBitmap *pbm) secPresetButtonControl;

	static bool InitClass() secPresetButtonControl;
	static void ExitClass() secPresetButtonControl;

private:
	virtual bool Init(char *pszLabel, int nfnt, char *szFnUp, char *szFnDown, bool fCenter) secPresetButtonControl;
};
#endif

// Checkbox Control

class CheckBoxControl : public Control // cbox
{
public:
	CheckBoxControl() secCheckBoxControl;
	virtual ~CheckBoxControl() secCheckBoxControl;

	virtual bool Init(Form *pfrm, word idc, int x, int y, char *pszLabel, int nfnt, bool fChecked) secCheckBoxControl;
	virtual bool Init(Form *pfrm, IniReader *pini, FindProp *pfind) secCheckBoxControl;
	virtual void OnPaint(DibBitmap *pbm) secCheckBoxControl;
	virtual void OnSelect(int nSelect) secCheckBoxControl;

	void SetText(char *psz) secCheckBoxControl;
	bool IsChecked() secCheckBoxControl;
	void SetChecked(bool fChecked) secCheckBoxControl;
	static bool InitClass() secCheckBoxControl;
	static void ExitClass() secCheckBoxControl;

private:
	bool Init(char *pszLabel, int nfnt, bool fChecked) secButtonControl;

	static TBitmap *s_ptbmOnUp;
	static TBitmap *s_ptbmOnDown;
	static TBitmap *s_ptbmOffUp;
	static TBitmap *s_ptbmOffDown;
	int m_ifnt;
	char m_szLabel[64];
	bool m_fChecked;
};

// Label Control

#define kfLblCenterText 0x8000
#define kfLblMultiLine 0x4000
#define kfLblRightText 0x2000
#define kfLblClipVertical 0x1000
#define kfLblEllipsis 0x0800
#define kfLblHitTest 0x0400

const int kcbLabelTextMax = 1000;

class LabelControl : public Control // lbl
{
public:
	LabelControl() secLabelControl;
	virtual ~LabelControl() secLabelControl;
	bool Init(int nfnt, char *pszLabel, char *pszFlags1, char *pszFlags2) secLabelControl;

	virtual bool Init(Form *pfrm, IniReader *pini, FindProp *pfind) secLabelControl;
	virtual void OnPaint(DibBitmap *pbm) secLabelControl;
	virtual int OnHitTest(Event *pevt) secLabelControl;

	virtual void SetText(const char *psz) secLabelControl;
	const char *GetText() secLabelControl;

protected:
	virtual void CalcRect() secLabelControl;

	char *m_szLabel;
	int m_nfnt;
};

inline const char *LabelControl::GetText() {
	return m_szLabel;
}

class EcomTextControl : public LabelControl // ect
{
public:
	EcomTextControl() secEcom;
	virtual ~EcomTextControl() secEcom;

	virtual bool Init(Form *pfrm, IniReader *pini, FindProp *pfind) secEcom;
	virtual void OnPaint(DibBitmap *pbm) secEcom;
	virtual int OnHitTest(Event *pevt) secEcom;
	virtual void SetText(char *psz) secEcom;

	// new methods

	bool ShowMoreText() secEcom;
	void ShowAll() secEcom;
	void DrawText(DibBitmap *pbm, Font *pfnt, char *psz, int x, int y, int cx, int cchMax) secEcom;

private:
	void CalcRect() secEcom;

	int m_cchCur;
	Color m_aclrEcom[4];
	long m_ctPrevTime;
};

const int kctEcomOutputInterval = kctUpdate; //	how often we call the timer

// Help Control

// define types

#define knChunkRawText 0
#define knChunkBitmap 1
#define knChunkLinkText 2
#define knChunkLargeText 3
#define knChunkGenericTag 4
#define knChunkAniData 5
#define knChunkHRTag 6

typedef struct Chunk {
	int nType;
	char *psz;
	int cch;
	DibBitmap *pbm;
	void *pv;
	char szText[30];
	bool fLargeFont;
} Chunk;

typedef struct HitTest {
	int x;
	int y;
    int nchBuffer;
	int cch;
    int dBest;
    bool fHit;
	char szText[30];
} HitTest;

// Positioning Constants
#define knFindPosRunToIndex 0
#define knFindPosAtLeastY 1
#define knFindPosAtMostY 2
#define knFindPosFingerScroll 3

typedef struct FindPositionHelper {
	int nDistY;
	int nIndex;
	int cyControl;
	int cySpan;
	bool fLargeFontSpanMet;
	int nchSpanMet;
	bool fLargeFontLastHR;
	int nchLastHR;
	bool fLargeFontFirstHR;
	int nchFirstHR;
	int nCondition;
	bool fLargeFont;
} FindPositionHelper;

#define knHelpControlBRHeight (gapfnt[kifntDefault]->GetHeight() * 3)

typedef bool (*ChunkProc)(int x,int y, int cx, int cy, int nchBuffer, int yCurrent, int nyBottom, Chunk *pchk, void *pv);

#define kfHelpScrollPosition 0x8000

class HelpControl : public Control, Timer
{
public:
	HelpControl() secHelpControl;
	~HelpControl() secHelpControl;

	bool FollowLink(const char *pszLink, int cch = -1) secHelpControl;
	void DoNextPage() secHelpControl;
	void DoPrevPage() secHelpControl;
	void DoIndex() secHelpControl;
	void DoBack() secHelpControl;
	bool SetFile(const char *pszFile) secHelpControl;

	// Control overrides

	virtual bool Init(Form *pfrm, IniReader *pini, FindProp *pfind) secHelpControl;
	virtual void OnPaint(DibBitmap *pbm) secHelpControl;
	virtual void OnPenEvent(Event *pevt) secHelpControl;

private:
    virtual void OnTimer(long tCurrent);

	bool Layout(dword nchStart, bool fLargeFont, DibBitmap *pbm, ChunkProc pfn, void *pv) secHelpControl;
	int LineHeight(bool) secHelpControl;
	int FindPrevPosition(int nchFrom, int cyAmount, bool *pfLargeFont) secHelpControl;
	int FindNextPosition(int nchFrom, int cySpan, bool *pfLargeFont,
            bool fCondition, bool fSmooth) secHelpControl;
    void GetSubRects(Rect *prcInterior, Rect *prcScrollPos = NULL);
    void DragScroll(int y);

	File *m_pfil;
    int m_cb;
	bool m_fLargeFont;
	char m_szText[129];
	int m_nchCurrent;
	int m_nchBack[10];
	int m_cyPageAmount;
    bool m_fDrag;
    int m_yDrag;
    FlickScroller m_flics;
    int m_yDragUp;
    bool m_fTimerAdded;
    HitTest m_hittest;
};

void Help(const char *pszAnchor = NULL, bool fPauseSimulation = false, const char *pszFile = NULL) secHelpControl;
void CutScene(const char *pszScene, bool fPauseSimulation) secCutScene;

// Edit Control

class EditControl : public Control // edc
{
public:
	EditControl() secEditControl;
	~EditControl() secEditControl;

	virtual bool Init(Form *pfrm, IniReader *pini, FindProp *pfind) secEditControl;
	virtual void OnPaint(DibBitmap *pbm) secEditControl;

	void SetText(const char *psz) secEditControl;
	void GetText(char *psz, int cb) secEditControl;

private:
	char m_szText[100];
	int m_nfnt;
};

inline EditControl::~EditControl() {
}

// List Control

#define kfLstcBorder 0x8000
#define kfLstcScrollPosition 0x4000
#define kfLstcKeepInteriorPositioning 0x2000

#define kfLstTabEllipsis 1
#define kfLstTabCenter 2
#define kfLstTabCenterOn 4
#define kfLstTabRight 8

struct ListItem;

class ListControl : public Control, IControlEventHandler, Timer // lstc
{
public:
	static bool InitClass() secListControl;
	static void ExitClass() secListControl;

	ListControl() secListControl;
	virtual ~ListControl() secListControl;

    Font *GetFont() secListControl;
	void Clear() secListControl;
	bool Add(ListItem *pli) secListControl;
	bool Add(const char *psz, void *pvData = NULL) secListControl;
//	ListItem *EnumItemPtr(Enum *penum) secListControl;
	int GetSelectedItemIndex() secListControl;
	ListItem *GetSelectedItem() secListControl;
	void *GetSelectedItemData() secListControl;
	void SetSelectedItemData(void *pvData) secListControl;
	bool GetSelectedItemText(char *psz, int cb) secListControl;
	bool SetSelectedItemText(const char *psz) secListControl;
	void Select(int iItem, bool fOnly = false, bool fMakeCenter = false) secListControl;
	int GetCount() secListControl;
    void SetTabStops(int x0, int x1 = -1, int x2 = -1, int x3 = -1);
    void SetTabFlags(word wf0, word wf1 = 0, word wf2 = 0, word wf3 = 0);
	virtual void GetSubRects(Rect *prcInterior, Rect *prcUpArrow = NULL,
            Rect *prcDownArrow = NULL, Rect *prcScrollPosition = NULL);
    void SetScrollPosColorIndex(int iclr) { m_iclrScrollPos = iclr; }

	// IControlEventHandler methods

	virtual void OnControlSelected(word idc) secForm;
	virtual bool OnControlHeld(word idc) secForm;
	virtual void OnControlNotify(word idc, int nNotify) secForm;

	// Control overrides

	virtual bool Init(Form *pfrm, IniReader *pini, FindProp *pfind) secListControl;
	virtual void OnPaint(DibBitmap *pbm) secListControl;
	virtual void OnSelect(int nSelect) secListControl;
	virtual void OnPenEvent(Event *pevt) secListControl;

    // Timer methods

	virtual void OnTimer(long tCurrent);

	static TBitmap *s_ptbmScrollUpUp;
	static TBitmap *s_ptbmScrollUpDown;
	static TBitmap *s_ptbmScrollDownUp;
	static TBitmap *s_ptbmScrollDownDown;

protected:
    virtual void OnPenEvent2(Event *pevt);
	virtual void DrawItem(DibBitmap *pbm, ListItem *pli, int x, int y,
            int cx, int cy) secListControl;
    virtual void DrawText(DibBitmap *pbm, char *psz, int x, int y,
            int cx, int cy, word wf);

private:
	bool NeedsScrollUpArrow() secListControl;
	bool NeedsScrollDownArrow() secListControl;
	int GetVisibleItemCount() secListControl;
    void DragScroll(int y);

private:
	ListItem *m_pliFirst, *m_pliLast;
	int m_nfnt;
	int m_cli;
	int m_iliTop;
    int m_cxEllipsis;
    int m_yDrag;
    int m_yDragUp;
    bool m_fTimerAdded;
    bool m_fDrag;
    int m_iliTopDrag;
    FlickScroller m_flics;
    int m_iclrScrollPos;
    bool m_fPenDown;

protected:
	int m_cyItem;
    int m_axTab[4];
    word m_awfTab[4];
};

const int kidcScrollDownButton = 0x7fff;
const int kidcScrollUpButton = 0x7ffe;

struct ListItem {
	ListItem *pliNext;
	union {
		char szText[80];
		struct {
			AnimationData *panid;
			int nStrip;
			int nFrame;
		} Anim;
	};
	void *pvData;
	bool fSelected;
	bool fDisabled;
};

class BuilderGob;
class BuildQueue;

// Animation List Control

class BuildListControl : public ListControl // alc
{
public:
	// ListControl overrides

	virtual bool Init(Form *pfrm, IniReader *pini, FindProp *pfind) secListControl;
	virtual void OnPaint(DibBitmap *pbm) secListControl;
	virtual void DrawItem(DibBitmap *pbm, ListItem *pli, int x, int y, int cx, int cy) secListControl;
	virtual void GetSubRects(Rect *prcInterior, Rect *prcUpArrow = NULL,
            Rect *prcDownArrow = NULL, Rect *prcScrollPos = NULL);

	bool Add(AnimationData *panid, int nStrip, int nFrame, void *pvData, bool fDisabled = false) secListControl;
	void SetQueueInfo(BuilderGob *pbldr, BuildQueue *pbq) { m_pbldr = pbldr; m_pbq = pbq;}

private:
	BuilderGob *m_pbldr;
	BuildQueue *m_pbq;
};

// Bitmap Control

class BitmapControl : public Control // bmc
{
public:
	BitmapControl() secBitmapControl;
	virtual ~BitmapControl() secBitmapControl;

	virtual bool Init(Form *pfrm, IniReader *pini, FindProp *pfind) secBitmapControl;
	virtual void OnPaint(DibBitmap *pbm) secBitmapControl;

	void SetBitmap(TBitmap *ptbm) secBitmapControl;

private:
	TBitmap *m_ptbm;
};

// Slider Control

class SliderControl : public Control // sldr
{
public:
	SliderControl() secSliderControl;
	~SliderControl() secSliderControl;

	virtual void OnPaint(DibBitmap *pbm) secSliderControl;
	virtual void OnSelect(int nSelect) secSliderControl;
	virtual void OnPenEvent(Event *pevt) secSliderControl;

	void SetRange(int nMin, int nMax) secSliderControl;
	void SetValue(int n) secSliderControl;
	int GetValue() secSliderControl;

private:
	int m_nMin, m_nMax;
	int m_nValue;
};

inline SliderControl::~SliderControl() {
};

class GraffitiScrollControl : public Control // grfs
{
public:
	GraffitiScrollControl() secGraffitiScrollControl;
	bool IsPainting() secGraffitiScrollControl;

	virtual void OnPenEvent(Event *pevt) secGraffitiScrollControl;
	virtual void OnPaint(DibBitmap *pbm) secGraffitiScrollControl;
	virtual bool Init(Form *pfrm, IniReader *pini, FindProp *pfind) secGraffitiScrollControl;

private:
	int m_nScale;
	int m_xDragStart;
	int m_yDragStart;
	WCoord m_wxViewStart;
	WCoord m_wyViewStart;
	bool m_fFrame;
};

// MiniMap Control

#define kfMmVertBorderInside 1
#define kfMmRedraw 2
#define kfMmHasPoweredRadar 4
#define kfMmPenDownTimeout 8

class MiniMapControl : public Control // mmc
{
public:
	MiniMapControl() secMiniMapControl;
	virtual ~MiniMapControl() secMiniMapControl;

    virtual int OnHitTest(Event *pevt) secMiniMapControl;
	virtual void OnPaint(DibBitmap *pbm) secMiniMapControl;
	virtual void OnPenEvent(Event *pevt) secMiniMapControl;
	virtual bool Init(Form *pfrm, IniReader *pini, FindProp *pfind) secMiniMapControl;
    virtual void OnBreakCapture();

	void Redraw() secMiniMapControl;
	void RedrawTRect(TRect *ptrc) secMiniMapControl;
	void RedrawTile(TCoord tx, TCoord ty) secMiniMapControl;
	void Update() secMiniMapControl;
	bool CalcPoweredRadar() secMiniMapControl;
	static int CalcWidth() secMiniMapControl;

private:
    void OnPenEvent2(Event *pevt);

    long m_tPenDown;
    Event m_evtPenDown;
	long m_tInvalidateLast;
	int m_nScale;
	int m_cyInputUI;
	int m_cxyBorder;
	DibBitmap *m_pbm;
	word m_wfMm;
	int m_xOff;
	int m_yOff;
	TCoord m_ctx;
	TCoord m_cty;
	dword *m_pbTileData;
	word *m_pwTileMap;
	int m_cbRowBytes;
	byte *m_pbFogMap;
    byte *m_pbTrMap;
	Color m_clrWhite;
	Color m_clrBlack;
	Color m_clrGalaxite;
    Color m_clrWall;
	Color m_aclrSide[kcSides];
};

// Pip Meter Control

class PipMeterControl : public Control // btn
{
public:
	static bool InitClass() secPipMeterControl;
	static void ExitClass() secPipMeterControl;

	PipMeterControl() secPipMeterControl;
	virtual ~PipMeterControl() secPipMeterControl;

	virtual bool Init(Form *pfrm, word idc, int x, int y, int cx, int cy,
			char *szPip = NULL) secPipMeterControl;
	virtual bool Init(Form *pfrm, IniReader *pini, FindProp *pfind) secPipMeterControl;
	virtual void OnPaint(DibBitmap *pbm) secPipMeterControl;

	void SetValue(int nValue) secPipMeterControl;

private:
	virtual bool Init(char *szPip) secPipMeterControl;

protected:
	TBitmap *m_ptbmPip;
	int m_nValue;

private:
	static TBitmap *s_ptbmPip;
};

// Damge Meter Control

struct UnitConsts;

class DamageMeterControl : public Control // btn
{
public:
	static bool InitClass() secDamageMeterControl;
	static void ExitClass() secDamageMeterControl;

	DamageMeterControl() secDamageMeterControl;
	virtual ~DamageMeterControl() secDamageMeterControl;

	virtual void OnPaint(DibBitmap *pbm) secDamageMeterControl;

	void SetUnitConsts(UnitConsts *puntc) secDamageMeterControl;

private:
	UnitConsts *m_puntc;
	static TBitmap *s_ptbmInfantry;
	static TBitmap *s_ptbmVehicle;
	static TBitmap *s_ptbmStructure;
};

// Credits Control

class CreditsControl : public Control
{
public:
	CreditsControl() {m_cCreditNeeders = 0;m_fDrawCreditSymbol = true;};
	void Update(long nCredits, word cCreditNeeders) secCreditsControl;

	virtual bool Init(Form *pfrm, IniReader *pini, FindProp *pfind) secCreditsControl;
	virtual void OnPaint(DibBitmap *pbm) secCreditsControl;

private:
	long m_nCredits;
	bool m_fDrawCreditSymbol;
	word m_cCreditNeeders;
};

// Power Control

class PowerControl : public Control
{
public:
	PowerControl() {m_fShowPowerSymbol = true; m_fPowerLow = false;};
	void Update(int nDemand, int nSupply) secPowerControl;

	virtual bool Init(Form *pfrm, IniReader *pini, FindProp *pfind) secPowerControl;
	virtual void OnPaint(DibBitmap *pbm) secPowerControl;

private:

	int m_nDemand, m_nSupply;
	int m_nDemandMax;
	bool m_fShowPowerSymbol;
	bool m_fPowerLow;
};

// InputUIForm

class InputUIForm : public Form, public Timer
{
public:
	InputUIForm(Chatter *chatter) secInputUIForm;
	~InputUIForm() secInputUIForm;
	void Update() secInputUIForm;

	// form

	virtual bool Init(FormMgr *pfrmm, IniReader *pini, word idf) secInputUIForm;
	virtual void OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd) secInputUIForm;
	virtual bool EventProc(Event *pevt) secInputUIForm;
	virtual void OnControlSelected(word idc) secInputUIForm;
	virtual bool OnControlHeld(word idc) secInputUIForm;

	// timer

	virtual void OnTimer(long tCurrent) secInputUIForm;

private:
    Chatter *m_pchatter;
	bool m_fTimerAdded;

	void InGameMenu() secInputUIForm;
	void TestOptions() secInputUIForm;
};

const int kfMasSelect = 1;
const int kfMasMove = 2;
const int kfMasAttack = 4;
const int kfMasShowMenu = 8;

// SimUIForm

struct UpdateNetMessageRecord { // unmr
    long msReceived;
    UpdateNetMessage *punm;
    UpdateNetMessageRecord *punmrNext;
};

class CommandQueue;
class MobileUnitGob;
class UnitMenu;

enum UIType { kuitFinger, kuitStylus };

const dword kfPhFinger1Down = 0x8000;
const dword kfPhFinger2Down = 0x4000;

class SimUIForm : public Form, public Timer, IGameCallback, ITransportCallback
{
public:
	SimUIForm(word wfRole, dword gameid, Chatter *chatter);
	virtual ~SimUIForm() secSimUIForm;
	void Update() secSimUIForm;
	void SendUpdateResult(int cUpdatesBlock, int cmsLatency, dword hash);
	void CalcLevelSpecificConstants() secSimUIForm;
	void InvalidateDragSelection() secSimUIForm;
    Gob *HitTestGob(int x, int y, bool fFinger, WCoord *pwx, WCoord *pwy,
            bool *pfHitSurrounding);
	void MoveOrAttackOrSelect(int x, int y, dword ff) secSimUIForm;
    void MoveOrAttackOrSelect(Gob *pgobHit, WCoord wxTarget, WCoord wyTarget,
            dword ff);
    bool HasSelectedUnits();
    void ShowUnitMenu(Gob *pgob);
    void SelectSameUnitTypes(Gob *pgob, bool fSfx);
    bool IsSelectionCommand(Gob *pgobHit);
	void ClearSelection() secSimUIForm;
    void SetUIType(UIType uit);
    void CheckMultiplayerGameOver(Pid pid);
    void SetObserving();

	word GetRole() {
		return m_wfRole;
	}

    class PenHandler {
    public:
        virtual ~PenHandler() {}
        virtual bool OnPenEvent(Event *pevt, bool fScrollOnly) = 0;
        virtual void OnPaint(DibBitmap *pbm) = 0;
        virtual void CheckScroll() = 0;
        virtual dword GetFlags() = 0;
        virtual void SetFlags(dword ff) = 0;
    };

    PenHandler *GetPenHandler() {
        return m_ppenh;
    }

	// Form methods

	virtual bool Init(FormMgr *pfrmm, IniReader *pini, word idf) secSimUIForm;
    virtual bool OnFilterEvent(Event *pevt);
	virtual bool OnPenEvent(Event *pevt) secSimUIForm;
	virtual void OnUpdateMapInvalidate(UpdateMap *pupd, Rect *prcOpaque) secSimUIForm;
	virtual void OnPaint(DibBitmap *pbm) secSimUIForm;
	virtual void OnPaintControls(DibBitmap *pbm, UpdateMap *pupd) secSimUIForm;
	virtual void OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd) secSimUIForm;
	virtual void ScrollInvalidate(UpdateMap *pupd) secSimUIForm;
	virtual bool EventProc(Event *pevt) secSimUIForm;
    virtual void FrameComplete() secSimUIForm;
	virtual void OnControlSelected(word idc);
    
	// Timer methods

	virtual void OnTimer(long tCurrent) secSimUIForm;

    void OnChatButtonBlink(bool fOn);
    void OnPlayersButton();

private:
    UpdateNetMessageRecord *m_punmrFirst;
    PenHandler *m_ppenh;
    dword m_gameid;
    Chatter *m_pchatter;

    class StylusHandler : public PenHandler {
    public:
        StylusHandler(SimUIForm *psui);
        virtual bool OnPenEvent(Event *pevt, bool fScrollOnly);
        virtual void OnPaint(DibBitmap *pbm) {}
        virtual void CheckScroll();
        virtual dword GetFlags() { return 0; }
        virtual void SetFlags(dword ff) {}
    private:
        SimUIForm *m_psui;
        bool m_fDragging;

        void OnPenDown(Event *pevt);
        void OnPenUp(Event *pevt);
        void OnPenDrag(Event *pevt);
        void OnPenHold(Event *pevt);
        void CancelModes();
    };

    class FingerHandler : public PenHandler {
    public:
        FingerHandler(SimUIForm *psui);
        ~FingerHandler();
        virtual bool OnPenEvent(Event *pevt, bool fScrollOnly);
        virtual void OnPaint(DibBitmap *pbm);
        virtual void CheckScroll();
        virtual dword GetFlags() { return m_ff; }
        virtual void SetFlags(dword ff) { m_ff = ff; }
    private:
        void OnPenDown(Event *pevt, bool fScrollOnly);
        void OnPenDown2(Event *pevt, bool fScrollOnly);
        void OnPenUp(Event *pevt, bool fScrollOnly);
        void OnPenUp2(Event *pevt, bool fScrollOnly);
        void OnPenMove(Event *pevt);
        void OnPenMove2(Event *pevt);
        void OnPenHold(Event *pevt);
        void UnhilightGob();
        void EnterNone();
        void EnterHilight(Gob *pgob);
        bool CheckSelect(Event *pevt);
        void EnterSelect(Event *pevt);
        void UpdateSelect(Event *pevt);
        bool CheckDragged(Event *pevt, int wcDrag);
        void EnterDrag(Event *pevt);
        void UpdateDrag(Event *pevt);
        void ShowUnitMenu(Gob *pgob);
        void ShowUnitTitle(Gob *pgob);
        void SetSelection();
        bool IsQuickUp(Event *pevt);

        SimUIForm *m_psui;
        int m_xDownLast, m_yDownLast;
        WCoord m_wxTarget, m_wyTarget;
        bool m_fHitSurrounding;
        Gid m_gidHitLast;
        Gid m_gidHilight;
        long m_tDoubleTap;
        int m_xDoubleTapDownLast, m_yDoubleTapDownLast;
        bool m_fTimerSet;
        bool m_fShowUnitMenu;
        UnitMenu *m_pfrmUnitTitle;
        dword m_ff;
        SelectionSprite *m_pselspr;
        dword m_maskA, m_maskB;
        int m_x1, m_y1;
        int m_x2, m_y2;
        Vec2d m_vOffsetA, m_vOffsetB;
        int m_xDrag, m_yDrag;
        WCoord m_wxViewDrag, m_wyViewDrag;
        long m_tDown;

        enum State {
            FHS_NONE, FHS_HILIGHT, FHS_SELECT, FHS_DRAG
        };
        State m_state;
    };

	MobileUnitGob *SetSelectionTargets(Gid gid, WCoord wxTarget,
            WCoord wyTarget) secSimUIForm;
	void OnLagNotify(Pid pidLagging, int cSeconds) secSimUIForm;
	void ReportLaggingPlayer(Player *pplrBehind) secSimUIForm;
	void ShowKillLaggyPlayerForm(Pid pidLaggy) secSimUIForm;
	void TrackLocalLag() secSimUIForm;
    void QueueUpdateMessage(UpdateNetMessage *punm);
    bool ProcessUpdateMessage(CommandQueue *pcmdq);
    void RunUpdatesNow();
    void CheckLagForm();
	void OnPlayerDisconnectNotify(Pid pid, int nReason);
    void OnCheckWin(Pid pid);

    // IGameCallback
    void OnReceiveChat(const char *player, const char *chat);
    void OnNetMessage(NetMessage **ppnm);
    void OnGameDisconnect();

    // ITransportCallback
	void OnStatusUpdate(char *pszStatus);
    void OnConnectionClose();
    void OnShowMessage(const char *message);

	static bool s_fReadyForPaint;
    
	bool m_fTimerAdded;
#ifdef STATS_DISPLAY
	bool m_fShowStats;
#endif
	word m_wfRole;
    CommandQueue *m_pcmdqServer;
	Form *m_pfrmWaitingForAllPlayers;
	long m_tLastCommunication;
	long m_tStartTimeout;
	Animation m_aniMoveTarget;
	WCoord m_wxMoveTarget;
	WCoord m_wyMoveTarget;
	int m_nStateMoveTarget;
    int m_cUpdatesBlock;
    long m_msUpdatesBlock;
    DialogForm *m_pfrmLag;
    Pid m_pidLagging;

#ifdef TRACKSTATE
    void ReportSyncError();
    void BeginTrackState();
    void EndTrackState();
    StateTracker *m_ptracker;
    bool m_fSyncError;
#endif
};

// Role defaults to single player client unless these flags are specified
// It is possible to be the creator, but not the server, when connecting to a dedicated game server

const word kfRoleMultiplayer = 0x0001; // client of a multiplayer game
const word kfRoleServer = 0x0002; // server of a multiplayer game
const word kfRoleCreator = 0x0004; // creator of a multiplayer game

// Pick a level form

enum LevelType // lt
{
	kltMultiplayer,
	kltChallenge,
	kltStory,
};

int CreateLevelList(char **asz) secGame;

class PickLevelForm : public ShellForm
{
public:
	PickLevelForm(LevelType lt) secGame;

	// Form overrides

	virtual bool Init(FormMgr *pfrmm, IniReader *pini, word idf) secGame;
	virtual void OnControlSelected(word idc) secGame;

public:
	char m_szLevel[kcbFilename];

private:
	LevelType m_lt;
};

class MissionList;

class SelectMissionForm : public ShellForm
{
public:
	SelectMissionForm(MissionList *pml, const MissionIdentifier *pmiidFind);
    bool GetSelectedMission(MissionIdentifier *pmiid);

	// Form overrides

	virtual bool Init(FormMgr *pfrmm, IniReader *pini, word idf) secGame;
	virtual void OnControlSelected(word idc) secGame;
	virtual void OnControlNotify(word idc, int nNotify);
    virtual bool OnPenEvent(Event *pevt);

private:
    MissionType InitLists(int iMissionSelect);
    int IndexFromMissionType(MissionType mt);
    MissionType MissionTypeFromIndex(int i);
    void SwitchMissionType(MissionType mt);
    int GetSelectedMissionIndex(ListControl *plstc);
    bool IsSelectedMissionLocked(ListControl *plstc);
    void SwitchToMissionType(MissionType mt);
    void UpdateDescription();

    int m_fMagicUnlock;
    int m_cMagic;
    MissionList *m_pml;
    const MissionIdentifier *m_pmiidFind;
    MissionType m_mt;
    ListControl *m_aplstc[3];
};

// LoadGameForm

class LoadGameForm : public ShellForm
{
public:
	LoadGameForm() secLoadSave;
	void SelectLast(bool fLast) secLoadSave;

	// Form overrides

	virtual bool Init(FormMgr *pfrmm, IniReader *pini, word idf) secLoadSave;
	virtual void OnControlSelected(word idc) secLoadSave;

private:
	int m_nGameLast;
};

class TestOptionsForm : public DialogForm
{
public:
	virtual bool Init(FormMgr *pfrmm, IniReader *pini, word idf) secGameOptionsForm;
	virtual void OnControlSelected(word idc) secGameOptionsForm;
};

class MemoryUseForm : public DialogForm
{
public:
	virtual bool Init(FormMgr *pfrmm, IniReader *pini, word idf) secGameOptionsForm;
	virtual void OnControlSelected(word idc) secGameOptionsForm;

private:
	void UpdateLabels() secGameOptionsForm;
};

//
// Triggers, Conditions, Actions and Condition/Action parameter types
//

// Condition/Action parameter types

class QualifiedNumber
{
public:
	bool Parse(char **ppsz) secTrigger;
	bool Compare(long nNumber) secTrigger;
#ifdef DEBUG_HELPERS
	char *ToString() {
		char *pszQualifier = "???";
		switch (m_nQualifier) {
		case knQualifierAtLeast:
			pszQualifier = "at least";
			break;

		case knQualifierAtMost:
			pszQualifier = "at most";
			break;

		case knQualifierExactly:
			pszQualifier = "exactly";
			break;
		}

		sprintf(s_szDebugHelpers, "%s %d", pszQualifier, m_nNumber);
		return s_szDebugHelpers;
	}

	static char s_szDebugHelpers[200];
#endif

	int m_nQualifier;
	long m_nNumber;
};

// class to run a countdown timer managed by triggers and
// shown in a SimUIForm control.

#define kfCtVisibleAtStart 1
#define kfCtRunning 2

class CountdownTimer
{
public:
	CountdownTimer() secTrigger;

	bool LoadState(Stream *pstm) secTrigger;
	bool SaveState(Stream *pstm) secTrigger;

	void SetTimer(int csecs, char *pszFormatString) secTrigger;
	bool GetTimer(int *psecs) secTrigger;
	void StartTimer(bool fStart) secTrigger;
	void ShowTimer(bool fShow) secTrigger;
	void Update() secTrigger;

	word GetFlags() {
		return m_wf;
	}

private:
	void UpdateString() secTrigger;

	int m_secs;
	char m_szFormat[80];
	word m_wf;
	long m_tLast;
};

// Conditions

class Condition // cdn
{
public:
	Condition() secCondition;
	virtual ~Condition() {}
	virtual bool Init(char *psz) secCondition;
	virtual bool IsTrue(Side side) = 0;
#ifdef DEBUG_HELPERS
	virtual bool SafeIsTrue(Side side) secCondition;
	virtual char *ToString() {
		return "Base";
	}
	static char s_szDebugHelpers[300];
#endif
	Condition *m_pcdnNext;
};

class MissionLoadedCondition : public Condition
{
public:
	// REMOVE_SOMEDAY: keep m68k-gcc from generating default constructor in .text section
	MissionLoadedCondition() secCondition;
	virtual bool IsTrue(Side side) secCondition;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Mission loaded";
	}
#endif
};

class CreditsCondition : public Condition
{
public:
	// REMOVE_SOMEDAY: keep m68k-gcc from generating default constructor in .text section
	CreditsCondition() secCondition;
	virtual bool Init(char *psz) secCondition;
	virtual bool IsTrue(Side side) secCondition;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		sprintf(s_szDebugHelpers, "%s has %s credits", PszFromCaSideMask(m_wfCaSideMask), m_qnum.ToString());
		return s_szDebugHelpers;
	}
#endif

private:
	word m_wfCaSideMask;
	QualifiedNumber m_qnum;
};

class OwnsUnitsCondition : public Condition
{
public:
	// REMOVE_SOMEDAY: keep m68k-gcc from generating default constructor in .text section
	OwnsUnitsCondition() secCondition;
	virtual bool Init(char *psz) secCondition;
	virtual bool IsTrue(Side side) secCondition;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		sprintf(s_szDebugHelpers, "%s owns %s %s", PszFromCaSideMask(m_wfCaSideMask), m_qnum.ToString(), PszFromUnitMask(m_um));
		return s_szDebugHelpers;
	}
#endif

private:
	word m_wfCaSideMask;
	QualifiedNumber m_qnum;
	UnitMask m_um;
};

class AreaContainsUnitsCondition : public Condition
{
public:
	// REMOVE_SOMEDAY: keep m68k-gcc from generating default constructor in .text section
	AreaContainsUnitsCondition() secCondition;
	virtual bool Init(char *psz) secCondition;
	virtual bool IsTrue(Side side) secCondition;
#ifdef DEBUG_HELPERS
	virtual char *ToString();
#endif

private:
	word m_wfCaSideMask;
	QualifiedNumber m_qnum;
	UnitMask m_um;
	int m_nArea;
	int m_nVersionLevel;
};

class PlaceStructureModeCondition : public Condition
{
public:
	// REMOVE_SOMEDAY: keep m68k-gcc from generating default constructor in .text section
	PlaceStructureModeCondition() secCondition;
	virtual bool IsTrue(Side side) secCondition;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		sprintf(s_szDebugHelpers, "place structure mode");
		return s_szDebugHelpers;
	}
#endif
};

class MinerCantFindGalaxiteCondition : public Condition
{
public:
	// REMOVE_SOMEDAY: keep m68k-gcc from generating default constructor in .text section
	MinerCantFindGalaxiteCondition() secCondition;
	virtual bool Init(char *psz) secCondition;
	virtual bool IsTrue(Side side) secCondition;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		sprintf(s_szDebugHelpers, "%s's Miner can't find Galaxite", PszFromCaSideMask(m_wfCaSideMask));
		return s_szDebugHelpers;
	}
#endif

private:
	word m_wfCaSideMask;
};

class GalaxiteCapacityReachedCondition : public Condition
{
public:
	// REMOVE_SOMEDAY: keep m68k-gcc from generating default constructor in .text section
	GalaxiteCapacityReachedCondition() secCondition;
	virtual bool Init(char *psz) secCondition;
	virtual bool IsTrue(Side side) secCondition;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		sprintf(s_szDebugHelpers, "%s's Galaxite capacity reached", PszFromCaSideMask(m_wfCaSideMask));
		return s_szDebugHelpers;
	}
#endif

private:
	word m_wfCaSideMask;
};

class ElapsedTimeCondition : public Condition
{
public:
	// REMOVE_SOMEDAY: keep m68k-gcc from generating default constructor in .text section
	ElapsedTimeCondition() secCondition;
	virtual bool Init(char *psz) secCondition;
	virtual bool IsTrue(Side side) secCondition;
#ifdef DEBUG_HELPERS
	virtual char *ToString();
#endif

private:
	QualifiedNumber m_qnum;
};

class SwitchCondition : public Condition
{
public:
	// REMOVE_SOMEDAY: keep m68k-gcc from generating default constructor in .text section
	SwitchCondition() secCondition;
	virtual bool Init(char *psz) secCondition;
	virtual bool IsTrue(Side side) secCondition;
#ifdef DEBUG_HELPERS
	virtual char *ToString();
#endif

private:
	int m_iSwitch;
	bool m_fOn;
};

class PeriodicTimerCondition : public Condition
{
public:
	// REMOVE_SOMEDAY: keep m68k-gcc from generating default constructor in .text section
	PeriodicTimerCondition() secCondition;
	virtual bool Init(char *psz) secCondition;
	virtual bool IsTrue(Side side) secCondition;
#ifdef DEBUG_HELPERS
	virtual bool SafeIsTrue(Side side) secCondition;
	virtual char *ToString();
#endif

private:
	int m_iTimer;
};

class CountdownTimerCondition : public Condition
{
public:
	virtual bool Init(char *psz) secCondition;
	virtual bool IsTrue(Side side) secCondition;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		sprintf(s_szDebugHelpers, "Coundown Timer is %s", m_qnum.ToString());
		return s_szDebugHelpers;
	}
#endif

private:
	QualifiedNumber m_qnum;
};

class DiscoversSideCondition : public Condition
{
public:
	// REMOVE_SOMEDAY: keep m68k-gcc from generating default constructor in .text section
	DiscoversSideCondition() secCondition;
	virtual bool Init(char *psz) secCondition;
	virtual bool IsTrue(Side side) secCondition;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		char szSideA[100];
		strcpy(szSideA, PszFromCaSideMask(m_wfCaSideMaskA));
		sprintf(s_szDebugHelpers, "%s discovers %s", szSideA, PszFromCaSideMask(m_wfCaSideMaskB));
		return s_szDebugHelpers;
	}
#endif

private:
	word m_wfCaSideMaskA;
	word m_wfCaSideMaskB;
};

class TestPvarCondition : public Condition
{
public:
	// REMOVE_SOMEDAY: keep m68k-gcc from generating default constructor in .text section
	TestPvarCondition() secCondition;
	virtual bool Init(char *psz) secCondition;
	virtual bool IsTrue(Side side) secCondition;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		sprintf(s_szDebugHelpers, "Pvar \"%s\": is %s", m_szName, m_qnum.ToString());
		return s_szDebugHelpers;
	}
#endif

private:
	char m_szName[kcbPvarNameMax];
	QualifiedNumber m_qnum;
};

class HasUpgradesCondition : public Condition
{
public:
	// REMOVE_SOMEDAY: keep m68k-gcc from generating default constructor in .text section
	HasUpgradesCondition() secCondition;
	virtual bool Init(char *psz) secCondition;
	virtual bool IsTrue(Side side) secCondition;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		sprintf(s_szDebugHelpers, "%s has ??? upgrades", PszFromCaSideMask(m_wfCaSideMask));
		return s_szDebugHelpers;
	}
#endif

private:
	word m_wfCaSideMask;
	UpgradeMask m_upgm;
};

// Actions

class TriggerAction // actn
{
public:
	TriggerAction() secAction;
	virtual ~TriggerAction() secAction;
	virtual bool LoadState(Stream *pstm) secAction;
	virtual bool SaveState(Stream *pstm) secAction;
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) = 0;
	TriggerAction *m_pactnNext;

#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Action";
	}
	static char s_szDebugHelpers[200];
#endif
};

class PreserveTriggerAction : public TriggerAction
{
public:
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
};

class WaitAction : public TriggerAction
{
public:
	WaitAction() secAction;
	virtual bool LoadState(Stream *pstm) secAction;
	virtual bool SaveState(Stream *pstm) secAction;
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString();
#endif

private:
	dword m_ctWait;
	dword m_atStartSide[kcSides];
	bool m_afWaitingSide[kcSides];
};

class CenterViewAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Center view";
	}
#endif

private:
	int m_nArea;
};

class SetNextMissionAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Set next mission";
	}
#endif

private:
	char m_szLevel[kcbFilename];
};

class EndMissionAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "End mission";
	}
#endif

   static void OnMPEndMissionActionEvent(int nWinLose, Side side);

private:
	int m_nWinLose;
};

class EcomAction : public TriggerAction
{
public:
	virtual ~EcomAction() secAction;
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Ecom";
	}
#endif

private:
	int m_nBackground;
	bool m_fMore;
	int m_nCharFrom;
	int m_nCharTo;
	char *m_pszMessage;
};

class SetAllowedUnitsAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Set allowed units";
	}
#endif

private:
	word m_wfCaSideMask;
	UnitMask m_um;
};

class SetAllowedUpgradesAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Set allowed upgrades";
	}
#endif

private:
	word m_wfCaSideMask;
	UpgradeMask m_upgm;
};

class SetUpgradesAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Set upgrades";
	}
#endif

private:
	word m_wfCaSideMask;
	UpgradeMask m_upgm;
};

class AlliesAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Allies";
	}
#endif

private:
	word m_wfCaSideMaskA;
	word m_wfCaSideMaskB;
};

class SetObjectiveAction : public TriggerAction
{
public:
	SetObjectiveAction() secAction;
	virtual ~SetObjectiveAction() secAction;

	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Set objective";
	}
#endif

private:
	word m_wfCaSideMask;
	char *m_szObjective;
};

class SetSwitchAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Set switch";
	}
#endif

private:
	int m_iSwitch;
	bool m_fOn;
};

class DefogAreaAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Defog area";
	}
#endif

private:
	int m_nArea;
};

// This is the "CreateUnitGroup-Action", not "Create-UnitGroupAction"

class CreateUnitGroupAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Create unit group";
	}
#endif

private:
	int m_nUnitGroup;
};

class HuntAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Hunt";
	}
#endif

private:
	word m_wfCaSideMask1;
	UnitMask m_um1;
	word m_wfCaSideMask2;
	UnitMask m_um2;
};

// This is the "CreateRandomUnitGroup-Action", not "Create-RandomUnitGroupAction"

class CreateRandomUnitGroupAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Create random unit group";
	}
#endif
};

// Start Countdown Action

class StartCountdownAction : public TriggerAction
{
public:
	StartCountdownAction() secAction;
	virtual ~StartCountdownAction() secAction;

	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;

private:
	int m_nSecs;
	char *m_szCountdown;
};

// Modify countdown action

class ModifyCountdownAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString();
#endif

private:
	int m_nAction;
};

class RepairAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Repair";
	}
#endif

private:
	word m_wfCaSideMask;
	bool m_fOn;
};

class EnableReplicatorAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Enable Replicator";
	}
#endif

private:
	word m_wfCaSideMask;
	bool m_fOn;
};

// Modify Credits action

class ModifyCreditsAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString();
#endif

private:
	word m_wfCaSideMask;
	int m_nAmount;
	int m_nAction;
};

// Move Units In Area action

class MoveUnitsInAreaAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "MoveUnitsInArea";
	}
#endif

private:
	word m_wfCaSideMask;
	UnitMask m_um;
	int m_nAreaSrc;
	int m_nAreaDst;
};

// Set Formal Objective Text action

const int kcchObjectiveMax = 80;
const int kcchObjectiveStatusMax = 15;
const int kcchObjectiveInfoMax = 350;

class SetFormalObjectiveTextAction : public TriggerAction
{
public:
	SetFormalObjectiveTextAction() secAction;
	virtual ~SetFormalObjectiveTextAction() secAction;

	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "SetFormalObjectiveText";
	}
#endif

private:
	int m_iObjective;
	char *m_szObjective;;
};

// Set Formal Objective Status action

class SetFormalObjectiveStatusAction : public TriggerAction
{
public:
	SetFormalObjectiveStatusAction() secAction;
	~SetFormalObjectiveStatusAction() secAction;

	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "SetFormalObjectiveStatus";
	}
#endif

private:
	int m_iObjective;
	char *m_szStatus;
};

// Set Formal Objective Info action

class SetFormalObjectiveInfoAction : public TriggerAction
{
public:
	SetFormalObjectiveInfoAction() secAction;
	virtual ~SetFormalObjectiveInfoAction() secAction;

	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "SetFormalObjectiveInfo";
	}
#endif

private:
	char *m_szInfo;
};

// Show Objectives action

class ShowObjectivesAction : public TriggerAction
{
public:
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
    static void OnMPShowObjectivesEvent(Side side);
};

// Show Cut Scene action

class CutSceneAction : public TriggerAction
{
public:
	CutSceneAction() secAction;
	virtual ~CutSceneAction() secAction;
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Cut scene";
	}
#endif

private:
	char *m_pszMessage;
};

// Jump To Mission action

class JumpToMissionAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Jump to mission";
	}
#endif

private:
	char m_szLevel[kcbFilename];
};

// For the moment, only actions can "block".
// Per-side context is stored in the actions that need it for blocking.

class Trigger
{
public:
	Trigger() secTrigger;
	~Trigger() secTrigger;

	bool LoadState(Stream *pstm) secTrigger;
	bool SaveState(Stream *pstm) secTrigger;
	bool Init(IniReader *pini, FindProp *pfind) secTrigger;
	void Execute(Side side, bool fForce = false) secTrigger;
	void Arm(Side side) secTrigger;
	void SetCurrentActionComplete(Side side) secTrigger;

private:
	bool LoadCondition(IniReader *pini, FindProp *pfind) secCondition;
	bool LoadAction(IniReader *pini, FindProp *pfind) secAction;

	Condition *m_pcdn;
	TriggerAction *m_pactn;
	TriggerAction *m_apactnLast[kcSides];
	bool m_afArmed[kcSides];

#ifdef DEBUG_HELPERS
	friend class TriggerViewer;
#endif
};

// Modify Persistent Variable action

class ModifyPvarAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString();
#endif

private:
	char m_szName[kcbPvarNameMax];
	int m_nAmount;
	int m_nAction;
};

// Set Persistent Variable Text action

class SetPvarTextAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Set Pvar Text";
	}
#endif

private:
	char m_szName[kcbPvarNameMax];
	char m_szValue[kcbPvarValueMax];
};

// ShowAlert action

class ShowAlertAction : public TriggerAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(Trigger *ptgr, Side side) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Show Alert";
	}
#endif

private:
	char m_szAlert[80];
};

#define kcTriggersPerSide 128
#define kcSwitchMax 16
#define kcTriggerTimersMax 16
#define kctTimerNotStarted -100000

class TriggerMgr // tgrm
{
public:
	TriggerMgr() secTrigger;
	~TriggerMgr() secTrigger;

	bool LoadState(Stream *pstm) secTrigger;
	bool SaveState(Stream *pstm) secTrigger;
	bool Init(IniReader *pini) secTrigger;
	void Exit() secTrigger;
	void Update() secTrigger;
	void SetConditionTrue(int nCondition, SideMask sidm) secTrigger;
	bool IsConditionTrue(int nCondition, SideMask sidm) secTrigger;

	CountdownTimer *GetCountdownTimer() {
		return &m_cdt;
	}

	void SetSwitch(int iSwitch, bool fOn) {
		m_abSwitch[iSwitch] = (byte)fOn;
	}

	bool GetSwitch(int iSwitch) {
		return m_abSwitch[iSwitch] != 0;
	}

	int AddPeriodicTimer(int ctPeriod) {
		Assert(m_cTimers < kcTriggerTimersMax);
		m_actPeriod[m_cTimers] = ctPeriod;
		m_actCountdown[m_cTimers] = kctTimerNotStarted;
		return m_cTimers++;
	}

	// Periodic timers only start counting the first time they are tested.
	// This way they can sit 'below' other conditions and have deterministic
	// firing times.

	void StartPeriodicTimer(int iTimer) {
		if (m_actCountdown[iTimer] == kctTimerNotStarted)
			m_actCountdown[iTimer] = m_actPeriod[iTimer];
	}

	bool IsPeriodicTimerTriggered(int iTimer) {
		return m_actCountdown[iTimer] <= 0;
	}

#ifdef DEBUG_HELPERS
	long GetTimerCountdown(int iTimer) {
		return m_actCountdown[iTimer];
	}

	long GetTimerPeriod(int iTimer) {
		return m_actPeriod[iTimer];
	}

	char *GetSwitchName(int iSwitch) {
		return m_aszSwitchNames[iSwitch];
	}
#endif

	void Enable(bool fEnable) {
		m_fEnabled = fEnable;
	}

private:
	bool AssignTriggerSides(int ntgr, char *psz) secTrigger;

	int m_ctgr;
	Trigger *m_atgr;
	byte m_mpSide2nTrigger[kcSides][kcTriggersPerSide];
	SideMask m_asidmCondition[knConditionMax];
	byte m_abSwitch[kcSwitchMax];
	int m_cTimers;
	int m_actPeriod[kcTriggerTimersMax];
	int m_actCountdown[kcTriggerTimersMax];
	long m_tLastUpdate;
	bool m_fEnabled;
	CountdownTimer m_cdt;

#ifdef DEBUG_HELPERS
	char m_aszSwitchNames[kcSwitchMax][50];
	friend TriggerViewer;
#endif
};

// UnitGroup
// - directs the process of building all the Units in the group
// - needs to know when Units under its direction complete commands given to them (or deactivate)
// - loads Unit list and UnitGroupActions (called by UnitGroupMgr at Level load time)
// - loads/saves itself

const byte kfUleBuilt = 0x01;
const byte kfUleReplicant = 0x02;

struct UnitListEntry {
	UnitType ut;
	Gid gid;
	byte bf;
};

class UnitGroupAction;	// forward declaration

class UnitGroup
{
public:
	UnitGroup() secUnitGroup;
	~UnitGroup() secUnitGroup;

	bool LoadState(Stream *pstm) secUnitGroup;
	bool SaveState(Stream *pstm) secUnitGroup;
	bool Init(IniReader *pini, int iug) secUnitGroup;
	void Activate() secUnitGroup;
	void Update() secUnitGroup;
	void OnBuilt(UnitGob *punt) secUnitGroup;
	void AddUnit(UnitGob *punt, bool fReplicant) secUnitGroup;
	void RemoveReplicants() secUnitGroup;

	word GetFlags() {
		return m_wf;
	}

	void SetFlags(word wf) {
		m_wf = wf;
	}

	Player *GetOwner() {
		return m_pplr;
	}

	UnitListEntry *GetUnitList() {
		return m_aule;
	}

	int GetUnitCount() {
		return m_cule;
	}
#ifdef DEBUG
	char *GetName() {
		return m_szName;
	}
#endif

private:
	bool LoadAction(IniReader *pini, FindProp *pfind) secUnitGroup;

	word m_wf;
	UnitGroupAction *m_pactn;
	UnitGroupAction *m_pactnLast;
	Player *m_pplr;
	int m_cule;
	UnitListEntry *m_aule;
	word m_wfMunt;		// flags for built MobileUnits
	int m_nSpawnArea;
	int m_nHealth;		// percent
#ifdef DEBUG
	char m_szName[64];
#endif

#ifdef DEBUG_HELPERS
	friend class UnitGroupViewer;
#endif
};

const word kfUgNeedsUnit = 0x0001;
const word kfUgActive = 0x0002;
const word kfUgLoopForever = 0x0004;
const word kfUgCreateAtLevelLoad = 0x0008;
const word kfUgRandomGroup = 0x0010;
const word kfUgReplaceGroup = 0x0020;
const word kfUgSpawn = 0x0040;
const word kfUgWaitingToReplace = 0x0080;
const word kfUgActivatedBefore = 0x0100;
const word kfUgNotRecentlyActivated = 0x0200;

// UnitGroupMgr
// - Updates the active UnitGroups
// - deactivates groups when the contained units are all deactivated
// - loads group data from Level file
// - loads/saves itself and UnitGroups

class UnitGroupMgr // tgrm
{
public:
	UnitGroupMgr() secUnitGroup;
	~UnitGroupMgr() secUnitGroup;

	bool LoadState(Stream *pstm) secUnitGroup;
	bool SaveState(Stream *pstm) secUnitGroup;
	bool Init(IniReader *pini) secUnitGroup;
	void Exit() secUnitGroup;
	void Update() secUnitGroup;
	void ActivateUnitGroup(int iug) secUnitGroup;
	void ActivateRandomUnitGroup() secUnitGroup;
	bool CreateAtLevelLoadGroups() secUnitGroup;
	UnitGroup *GetUnitGroup(Gid gid) secUnitGroup;

private:
	int m_cug;
	UnitGroup *m_aug;

#ifdef DEBUG_HELPERS
	friend class UnitGroupViewer;
#endif
};

// UnitGroup Actions

class UnitGroupAction
{
public:
	UnitGroupAction() secAction;
	virtual ~UnitGroupAction() {}
	virtual bool LoadState(Stream *pstm) secAction;
	virtual bool SaveState(Stream *pstm) secAction;
	virtual bool Init(char *psz) = 0;
	virtual void Reset() secAction;
	virtual bool Perform(UnitGroup *pug) = 0;

	UnitGroupAction *m_pactnNext;

#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "UnitGroupAction";
	}

	static char s_szDebugHelpers[200];
#endif
};

class WaitUnitGroupAction : public UnitGroupAction
{
public:
	WaitUnitGroupAction() secAction;
	virtual bool LoadState(Stream *pstm) secAction;
	virtual bool SaveState(Stream *pstm) secAction;
	virtual bool Init(char *psz) secAction;
	virtual void Reset() secAction;
	virtual bool Perform(UnitGroup *pug) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString();
#endif

private:
	dword m_ctWait;
	long m_tStart;
	bool m_fWaiting;
};

class SetSwitchUnitGroupAction : public UnitGroupAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(UnitGroup *pug) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Set switch";
	}
#endif

private:
	int m_iSwitch;
	bool m_fOn;
};

class MoveUnitGroupAction : public UnitGroupAction
{
public:
	virtual bool LoadState(Stream *pstm) secAction;
	virtual bool SaveState(Stream *pstm) secAction;
	virtual bool Init(char *psz) secAction;
	virtual void Reset() secAction;
	virtual bool Perform(UnitGroup *pug) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString();
#endif

private:
	bool m_fWaiting;
	int m_nArea;
};

class AttackUnitGroupAction : public UnitGroupAction
{
public:
	virtual bool LoadState(Stream *pstm) secAction;
	virtual bool SaveState(Stream *pstm) secAction;
	virtual bool Init(char *psz) secAction;
	virtual void Reset() secAction;
	virtual bool Perform(UnitGroup *pug) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString();
#endif

private:
	// These are initialized from the loaded level

	UnitMask m_um;
	word m_wfCaSideMask;
	dword m_ctWait;

	// These are persisted in the save game

	bool m_fWaiting;
	long m_tStart;
	Gid m_gidTarget;
};

class GuardUnitGroupAction : public UnitGroupAction
{
public:
	virtual bool LoadState(Stream *pstm) secAction;
	virtual bool SaveState(Stream *pstm) secAction;
	virtual bool Init(char *psz) secAction;
	virtual void Reset() secAction;
	virtual bool Perform(UnitGroup *pug) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Guard";
	}
#endif

private:
	// These are initialized from the loaded level

	dword m_ctWait;

	// These are persisted in the save game

	bool m_fWaiting;
	long m_tStart;
};

class MineUnitGroupAction : public UnitGroupAction
{
public:
	virtual bool Init(char *psz) secAction;
	virtual bool Perform(UnitGroup *pug) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Mine";
	}
#endif
};

class GuardVicinityUnitGroupAction : public UnitGroupAction
{
public:
	virtual bool LoadState(Stream *pstm) secAction;
	virtual bool SaveState(Stream *pstm) secAction;
	virtual bool Init(char *psz) secAction;
	virtual void Reset() secAction;
	virtual bool Perform(UnitGroup *pug) secAction;
#ifdef DEBUG_HELPERS
	virtual char *ToString() {
		return "Guard vicinity";
	}
#endif

private:
	// These are initialized from the loaded level

	dword m_ctWait;

	// These are persisted in the save game

	bool m_fWaiting;
	long m_tStart;
};

// BuildMgr
// - directs BuilderGobs to build desired Units
// - keeps track of which BuilderGobs are building which Units for which UnitGroups
// - is notified when a BuilderGob completes a build (and what Unit was
//   completed and for which UnitGroup)

struct BuildEntry {
	BuildEntry *pbldeNext;
	UnitType ut;
	int nArea;
	UnitGroup *pug;
	Gid gidBuilder;
};

class BuildMgr
{
public:
	BuildMgr() secBuildMgr;
	~BuildMgr() secBuildMgr;
	bool BuildUnit(UnitType ut, UnitGroup *pug, int nArea) secBuildMgr;
	void OnBuilt(UnitGob *punt, BuilderGob *pbldr) secBuildMgr;
	void Update() secBuildMgr;

private:
	BuildEntry *m_pbldeFirst;
};

// Simulation

// Use min gob count for 68K build because of stack variables

#ifdef __CPU_68K
#define kcpgobMax 512
#else
#define kcpgobMax 768
#endif
#define kcpgobVisibleMax 200

class Simulation : public TimerMgr // sim
{
public:
	Simulation() secSimulation;
	~Simulation() secSimulation;

	bool OneTimeInit() secSimulation;
	void OneTimeExit() secSimulation;
	bool PerLevelInit() secSimulation;
	void PerLevelExit() secSimulation;

	bool LoadLevel(const char *pszLevelName) secSimulation;
	bool LoadState(Stream *pstm) secSimulation;
	bool SaveState(Stream *pstm) secSimulation;
	void ClearGobSelection() secSimulation;
	void SetGobSelected(Gob *pgob) secSimulation;
	void SelectSameUnitTypes(UnitGob *punt, TRect *ptrc) secSimulation;

	Level *GetLevel() {
		return m_plvl;
	}

	int GetUpdateCount() {
		return m_cUpdates;
	}

	void SetMiniMapScale(int nScale) {
		m_nMiniMapScale = nScale;
	}

	int GetMiniMapScale() {
		return m_nMiniMapScale;
	}

	BuildMgr *GetBuildMgr() {
		return m_pbldm;
	}

	void Update(CommandQueue *pcmdq) secSimulation;
	void Draw(UpdateMap *pupd, DibBitmap *pbm) secSimulation;
	void DrawBackground(UpdateMap *pupd, DibBitmap *pbm) secSimulation;
	void DrawFog(UpdateMap *pupd, DibBitmap *pbm) secSimulation;
	bool SetViewPos(WCoord wx, WCoord wy, bool fInit = false) secSimulation;
	void GetViewPos(WCoord *pwx, WCoord *pwy) secSimulation;
	bool HitTest(Enum *penm, WCoord wx, WCoord wy, word wf, Gob **ppgob) secSimulation;
    Gob *FingerHitTest(WCoord wx, WCoord wy, word wf, bool *pfHitSurrounding) secSimulation;
	void Pause(bool fPause) secSimulation;
	bool IsPaused() secSimulation;
	void FindVisibleGobs(Gob ***pppgobVisible = NULL, int *pcgobVisible = NULL) secSimulation;
	void SetSelection(Rect *prc) secSimulation;

	// TimerMgr override

	virtual long GetTickCount() secSimulation;
	virtual void AddTimer(Timer *ptmr, long ct) secSimulation; //stub

#ifdef TRACKSTATE
    void TrackState(StateFrame *frame);
#endif

private:
    void HandlePlayerDisconnect(Message *pmsg);

	Level *m_plvl;
	WCoord m_wxView, m_wyView;
	bool m_fGameOver;
	int m_cUpdates;		// Can count 80 ms updates for 5+ years without wrapping
	long m_tCurrent;
	bool m_fPaused;
	int m_nMiniMapScale;
	BuildMgr *m_pbldm;

	Gob *m_apgobVisible[kcpgobVisibleMax];
	int m_cgobVisible;
	WCoord m_wxViewSave;
	WCoord m_wyViewSave;
	int m_cUpdatesSave;
	int m_cupdTriggerMgrUpdateLast;
};
extern Simulation gsim;

// Minimap coord from World coord

inline int MmcFromWc(WCoord wc) {
	return ((wc + 0x007f) & 0xff80) >> (9 - gsim.GetMiniMapScale());	// == 7 for hires, 8 for lores
}

// Level

// NOTE: changing the sizes any of the 'int's used in the types and
// member variables below will break .ini and .lvl file reading which
// uses the '%d' feature of IniScanf which always stores parsed numbers
// as 'int's.

struct SideInfo	// sidi
{
	int nInitialCredits;
	WPoint wptInitialView;
	int nIntelligence;
	int cStructuresInitial;
	int cMobileUnitsInitial;
};

class Level // lvl
{
public:
	Level() secLevel;
	~Level() secLevel;
	bool Init(const char *pszLevelName, bool fConstantsOnly = false) secLevel;

	bool LoadState(Stream *pstm) secLevel;
	bool SaveState(Stream *pstm) secLevel;
	bool LoadLevelInfo(const char *pszLevelName, IniReader *piniLoaded = NULL) secLevel;

	TileMap *GetTileMap() {
		return m_ptmap;
	}

	FogMap *GetFogMap() {
		return m_pfogm;
	}

	TerrainMap *GetTerrainMap() {
		return m_ptrmap;
	}

	int GetMinPlayers() {
		return m_nPlayersMin;
	}

	int GetMaxPlayers() {
		return m_nPlayersMax;
	}

	const char *GetTitle() {
		return m_szTitle;
	}

	SideInfo *GetSideInfo(Side side) {
		return &m_asidi[side];
	}

	TriggerMgr *GetTriggerMgr() {
		return &m_tgrm;
	}

	UnitGroupMgr *GetUnitGroupMgr() {
		return &m_ugm;
	}

	char *GetFilename() {
		return m_szFileLevel;
	}

	// This version is the "level file format" version

	int GetVersion() {
		return m_nVersion;
	}

	// This is the per-save unique revision number

	dword GetRevision() {
		return m_dwRevision;
	}

private:
	bool LoadSideInfo(IniReader *pini, char *pszSideName, SideInfo *psidi) secLevel;
	bool LoadLevelConstants(const char *pszLevelName, IniReader *pini) secLevel;
	bool LoadLevelVariables(IniReader *pini) secLevel;

private:
	char m_szTitle[kcbLevelTitle];
	char m_szFileLevel[kcbFilename];
	int m_nPlayersMin, m_nPlayersMax;
	TileMap *m_ptmap;
	FogMap *m_pfogm;
	TerrainMap *m_ptrmap;
	bool m_fInitialized;
	SideInfo m_asidi[5];
	TriggerMgr m_tgrm;
	UnitGroupMgr m_ugm;

	// This is the "level format" version. This is versioned upward over time and is here so
	// that WI knows what format the level was saved in.

	int m_nVersion;

	// This is the revision # of the level. So saved games with older revision numbers
	// aren't loaded. Note this isn't an incrementing version number. It is a unique
	// mark that every mission has.

	dword m_dwRevision;
};

// Structure/Unit type values and masks

const UnitMask kumShortRangeInfantry = 1L << kutShortRangeInfantry;
const UnitMask kumLongRangeInfantry = 1L << kutLongRangeInfantry;
const UnitMask kumTakeoverSpecialist = 1L << kutTakeoverSpecialist;
const UnitMask kumAndy = 1L << kutAndy;
const UnitMask kumFox = 1L << kutFox;
const UnitMask kumInfantry = kumShortRangeInfantry | kumLongRangeInfantry | kumTakeoverSpecialist | kumAndy | kumFox;

const UnitMask kumGalaxMiner = 1L << kutGalaxMiner;
const UnitMask kumLightTank = 1L << kutLightTank;
const UnitMask kumMediumTank = 1L << kutMediumTank;
const UnitMask kumMachineGunVehicle = 1L << kutMachineGunVehicle;
const UnitMask kumRocketVehicle = 1L << kutRocketVehicle;
const UnitMask kumMobileHeadquarters = 1L << kutMobileHeadquarters;
const UnitMask kumArtillery = 1L << kutArtillery;
const UnitMask kumVehicles = kumGalaxMiner | kumLightTank | kumMediumTank | kumMachineGunVehicle |
		kumRocketVehicle | kumMobileHeadquarters | kumArtillery;
const UnitMask kumMobileUnits = kumInfantry | kumVehicles;

const UnitMask kumHumanResourceCenter = 1L << kutHumanResourceCenter;
const UnitMask kumReactor = 1L << kutReactor;
const UnitMask kumProcessor = 1L << kutProcessor;
const UnitMask kumHeadquarters = 1L << kutHeadquarters;
const UnitMask kumResearchCenter = 1L << kutResearchCenter;
const UnitMask kumVehicleTransportStation = 1L << kutVehicleTransportStation;
const UnitMask kumRadar = 1L << kutRadar;
const UnitMask kumWarehouse = 1L << kutWarehouse;
const UnitMask kumReplicator = 1L << kutReplicator;

const UnitMask kumMachineGunTower = 1L << kutMachineGunTower;
const UnitMask kumRocketTower = 1L << kutRocketTower;
const UnitMask kumTowers = kumMachineGunTower | kumRocketTower;

const UnitMask kumStructures = kumHumanResourceCenter | kumReactor | kumProcessor | kumHeadquarters |
		kumResearchCenter | kumVehicleTransportStation | kumRadar | kumWarehouse | kumMachineGunTower |
		kumRocketTower | kumReplicator;

// NOTE: we don't count the Replicator as a Builder so the BuildMgr won't start using it
const UnitMask kumBuilder = kumHeadquarters | kumVehicleTransportStation | kumHumanResourceCenter;
const UnitMask kumAll = kumInfantry | kumVehicles | kumStructures;
const UnitMask kumImpossible = 0xffffffff;

// Upgrade type values and masks (values to be moved to res.h most likely)

struct Upgrade { // upg
	UpgradeMask upgm;
	char szName[50];
	char szIconName[20];
	UpgradeMask upgmPrerequisites;
	UnitMask umPrerequisites;
	int nCost;
	int ctTimeToBuild;
	char *szDescription;
	char *szPrerequisites;
};

#define kupgtAdvancedHRC 0
#define kupgtAdvancedVTS 1
#define kupgtIncreasedBullpupSpeed 2
#define kupgtMax 2

const UpgradeMask kupgmAdvancedHRC = 1 << kupgtAdvancedHRC;
const UpgradeMask kupgmAdvancedVTS = 1 << kupgtAdvancedVTS;
const UpgradeMask kupgmIncreasedBullpupSpeed = 1 << kupgtIncreasedBullpupSpeed;
const UpgradeMask kupgmAll = kupgmAdvancedHRC | kupgmAdvancedVTS | kupgmIncreasedBullpupSpeed;

// Player-related types

class PlayerMgr;

// UNDONE: pull these from game.ini?

const long knWarehouseCapacity = 5000;
const long knProcessorCapacity = 3000;

// Who is using credits during a given update? Useful for sound effects

#define knConsumerGeneric 0
#define knConsumerRepair 1
#define knConsumerMax 1

const int kcFormalObjectivesMax = 4;

const int ksoObjectives = 1;
const int ksoWinSummary = 2;
const int ksoLoseSummary = 3;

class Player // plr
{
	friend class PlayerMgr;

public:
	Player() secPlayer;
	~Player() secPlayer;

	void Init(Pid pid) secPlayer;
	bool LoadState(Stream *pstm) secPlayer;
	bool SaveState(Stream *pstm) secPlayer;
	void SetName(const char *pszName) secPlayer;
	UnitMask GetUnitMask() secPlayer;
	void SetCredits(int nCredits, bool fAffectTotals, int nConsumer = knConsumerGeneric) secPlayer;
	int GetCreditsDirection() secPlayer;
	int GetCreditsConsumer() secPlayer;
	void AddPowerSupplyAndDemand(int nPowerSupply, int nPowerDemand) secPlayer;
	void Repair(bool fOn) secPlayer;
	void Update(int cUpdates) secPlayer;
	void SetFormalObjectiveText(int iObjective, char *pszText) secPlayer;
	void SetFormalObjectiveStatus(int iObjective, char *pszStatus) secPlayer;
	void SetFormalObjectiveInfo(char *pszInfo) secPlayer;
	int ShowObjectives(int so, bool fForceInfoDisplay = false, bool fAborting = false) secObjectives;
	void ModifyNeedCreditsCount(int cDelta)secPlayer;
	bool IsBehind(int cUpdates) secPlayer;
    void SetLeftGame() {
        m_fLeftGame = true;
    }
    bool GetLeftGame() {
        return m_fLeftGame;
    }
	char *GetFormalObjectiveText(int iObjective) {
		return m_aszFormalObjectiveText[iObjective];
	}

	char *GetFormalObjectiveStatus(int iObjective) {
		return m_aszFormalObjectiveStatus[iObjective];
	}

	char *GetFormalObjectiveInfo() {
		return m_szFormalObjectiveInfo;
	}

	const char *GetName() {
		return m_szName;
	}

	word GetFlags() {
		return m_wf;
	}

	void SetFlags(word wf) {
		m_wf = wf;
	}

	Pid GetId() {
		return m_pid;
	}

	Side GetSide() {
		return m_side;
	}

	void SetSide(Side side) {
		m_side = side;
 		m_sidmAllies = GetSideMask(m_side);
		m_sidmDiscovered = GetSideMask(m_side);
	}

	SideMask GetAllies() {
		return m_sidmAllies;
	}

	void SetAllies(SideMask sidm) {
		m_sidmAllies = sidm | GetSideMask(m_side);
	}

	SideMask GetDiscoveredSides() {
		return m_sidmDiscovered;
	}

	void SetDiscoveredSides(SideMask sidm) {
		m_sidmDiscovered = sidm;
	}

	void SetDiscoverPoint(TPoint *ptpt) {
		m_tptDiscover = *ptpt;
	}

	TPoint GetDiscoverPoint() {
		return m_tptDiscover;
	}

	int GetCredits() {
		return m_nCredits;
	}

	word GetNeedCreditsCount() {
		return m_cStructsNeedCredits;
	}

	int GetCapacity() {
		return (m_acut[kutWarehouse] * knWarehouseCapacity) + (m_acut[kutProcessor] * knProcessorCapacity);
	}

	int GetPowerSupply() {
		return m_nPowerSupply;
	}

	int GetPowerDemand() {
		return m_nPowerDemand;
	}

	bool IsPowerLow() {
		return m_nPowerDemand > m_nPowerSupply;
	}

	void IncUnitCount(UnitType ut) {
		m_acut[ut]++;
	}

	void DecUnitCount(UnitType ut) {
		Assert(m_acut[ut] != 0);
		m_acut[ut]--;
	}

	int GetUnitCount(UnitType ut) {
		return m_acut[ut];
	}

    void IncUnitBuiltCount(UnitType ut) {
        m_acutBuilt[ut]++;
    }

    void DecUnitBuiltCount(UnitType ut) {
        m_acutBuilt[ut]--;
        if (m_acutBuilt[ut] < 0) {
            m_acutBuilt[ut] = 0;
        }
    }

	int GetUnitActiveCountFromMask(UnitMask um) secPlayer;
	int GetUnitInstanceCountFromMask(UnitMask um) secPlayer;

	word GetUpgrades() {
		return m_wfUpgrades;
	}

	void SetUpgrades(word wfUpgrades) secPlayer;

	UnitMask GetAllowedUnits() {
		return m_umAllowed;
	}

	void SetAllowedUnits(UnitMask um) {
		m_umAllowed = um;
	}

	void SetObjective(char *psz) secSimulation;

	char *GetObjective() {
		return m_szObjective;
	}

	UpgradeMask GetUpgradeMask() {
		return m_upgm;
	}

	void SetUpgradeMask(UpgradeMask upgm) {
		m_upgm = upgm;
	}

	UpgradeMask GetAllowedUpgrades() {
		return m_upgmAllowed;
	}

	void SetAllowedUpgrades(UpgradeMask um) {
		m_upgmAllowed = um;
	}

	void IncEnemyMobileUnitsKilled() {
		m_cmuntKilled++;
	}

	int GetEnemyMobileUnitsKilled() {
		return m_cmuntKilled;
	}

	void IncEnemyStructuresKilled() {
		m_cstruKilled++;
	}

	int GetEnemyStructuresKilled() {
		return m_cstruKilled;
	}

	void IncMobileUnitsLost() {
		m_cmuntLost++;
	}

	int GetMobileUnitsLost() {
		return m_cmuntLost;
	}

	void IncStructuresLost() {
		m_cstruLost++;
	}

	int GetStructuresLost() {
		return m_cstruLost;
	}

	int GetTotalCreditsAcquired() {
		return m_nTotalCreditsAcquired;
	}

	void ModifyTotalCreditsConsumed(int nAmount) {
		m_nTotalCreditsConsumed += nAmount;
	}

	int GetTotalCreditsConsumed() {
		return m_nTotalCreditsConsumed;
	}

	word GetHandicap() {
		return m_wfHandicap;
	}

	void SetHandicap(word wf) {
		m_wfHandicap = wf;
	}

	// Only valid for server-side Player proxies

	long GetUpdateCount() {
		return m_cUpdates;
	}

	void SetUpdateCount(int cUpdates) {
		m_cUpdates = cUpdates;
	}

	bool IsLagging(long cUpdates) secPlayer;
	int GetLagState() secPlayer;
	void SetLagState(int nLagState) secPlayer;
	int GetLagTimeout() secPlayer;

#ifdef TRACKSTATE
    void TrackState(StateFrame *frame);
#endif

private:
	word m_wfUpgrades;
	Side m_side;
	SideMask m_sidmAllies;
	SideMask m_sidmDiscovered;
	TPoint m_tptDiscover;
	char m_szName[kcbPlayerName];
	word m_wf;
	Pid m_pid;
	int m_nCredits;
	int m_nCreditsAcquired, m_nCreditsConsumed;
	int m_nDirCredits;
	int m_nConsumerCredits;
	int m_nPowerSupply;
	int m_nPowerDemand;
	int m_acut[kutMax];
	int m_acutBuilt[kutMax];
	UnitMask m_umAllowed;
	char m_szObjective[kcchObjectiveMax];
	UpgradeMask m_upgm;
	UpgradeMask m_upgmAllowed;
	char *m_aszFormalObjectiveText[kcFormalObjectivesMax];
	char *m_aszFormalObjectiveStatus[kcFormalObjectivesMax];
	char *m_szFormalObjectiveInfo;
	int m_cmuntKilled;
	int m_cstruKilled;
	int m_cmuntLost;
	int m_cstruLost;
	int m_nTotalCreditsAcquired;
	int m_nTotalCreditsConsumed;
	int m_cUpdatesRepairLast;
	word m_cStructsNeedCredits;
	word m_wfHandicap;
	int m_cUpdates;	// only valid for server-side Player proxies
	int m_nLagState;
	long m_tLagStart;
	long m_tLastLag;
    bool m_fLeftGame;

#ifdef DETECT_SYNC_ERRORS
public:
	UpdateResult m_ur;
#endif
};

class PlayersUpdateNetMessage;

class PlayerMgr // plrm
{
public:
	PlayerMgr() secPlayer;
	~PlayerMgr() secPlayer;
	void Reset() secPlayer;
	void Init(PlayersUpdateNetMessage *ppunm) secPlayer;
	bool Init(char *pszLevel) secPlayer;
	bool LoadState(Stream *pstm) secPlayer;
	bool SaveState(Stream *pstm) secPlayer;
	Player *AllocPlayer(word wf = 0) secPlayer;
	void Update(int cUpdates) secPlayer;
	void SetAllies(Player **applr, int cplrs, SideMask sidmAllies) secPlayer;

	void FreePlayer(Player *pplr) {
		pplr->m_wf = 0;
	}

	Player *GetPlayer(Side side) secPlayer;

	Player *GetPlayerFromPid(Pid pid) {
        // Pid is unsigned 
        if (pid != kpidNeutral && pid < kcPlayersMax) {
            return &m_aplr[((int)pid)];
        }
        return NULL;
	}

	Player* GetNextPlayer(Player *pplr) secPlayer;
	Player* GetNextHumanPlayer(Player *pplr) secPlayer;
    Player* GetNextObservingPlayer(Player *pplr) secPlayer;
    const char *GetCreatorName();

	int GetPlayerCount() secPlayer;
	bool DetectTransitionToSingleHumanTeam(Pid pidLeft) secPlayer;
	int GetUnitInstanceCountFromMask(UnitMask um, word wfPlr) secPlayer;
    void SendWinStats();

#ifdef TRACKSTATE
    void TrackState(StateFrame *frame);
#endif

protected:
    int GetHumanTeamCount(bool fExtra, Pid pidExtra);

	Player *m_aplr;
};

// Gob

#define kfGobNoEnemiesNearby	0x00000001	// No enemies nearby so don't waste time looking
#define kfGobRedraw 			0x00000002  // Something has changed that requires this gob to redraw
//#define kfGobInvalidated		0x00000004	// Gob has been invalidated
#define kfGobBeingUpgraded		0x00000008	// Gob is being upgraded
#define kfGobBeingBuilt			0x00000010	// Gob is in the process of being built, not yet ready for active duty
//#define kfGobOccupyingTerrain	0x00000020
#define kfGobFlashing			0x00000040	// Gob is flashing
#define kfGobDrawFlashed		0x00000080	// Gob will be drawn flashing
//#define kfGobRepairing			0x00000100	// unused
#define kfGobActive				0x00000200	// Gob is alive and doing its thing
#define kfGobStructure			0x00000400	// Gob is a fixed structure
#define kfGobMobileUnit			0x00000800	// Gob is a mobile unit
#define kfGobUnit				0x00001000	// Gob is a Unit (our version of RTTI!)
#define kfGobDebug				0x00002000	// Gobs DebugBreak at strategic points when this is enabled
#define kfGobSelected			0x00004000	// Gob is selected
#define kfGobStateMachine		0x00008000	// Gob has an active StateMachine that may want Update messages
#define kfGobVisibleLastFrame	0x00010000  // Gob was visible last frame
#define kfGobIncludeFindVisible 0x00020000  // Include gob in next FindVisibleGobs search
#define kfGobTransitioningToVisible 0x00040000 // Gob is going from non-visible to visible
#define kfGobAnimationChanged	0x00080000	// UpdateCount must not affect next AdvanceAnimation

#define kfGobLayerSurfaceDecal	0x00100000
#define kfGobLayerDepthSorted	0x00200000
#define kfGobLayerSmokeFire		0x00400000
#define kfGobLayerSymbols		0x00800000	// Repairing and need-power symbols draw on this layer
#define kfGobLayerSelection		0x01000000
#define kfGobLayerMask (kfGobLayerSurfaceDecal | kfGobLayerDepthSorted | kfGobLayerSmokeFire | kfGobLayerSymbols | kfGobLayerSelection)

// Layers

#define knLayerSurfaceDecal 0
#define knLayerDepthSorted 1
#define knLayerSmokeFire 2
#define knLayerSymbols 3
#define knLayerSelection 4
#define knLayerEnd 4

#define knLayerMiniMap -1

// When a y coordinate is passed to MakeSortKey as the first value and a Gob
// id (gid) is passed as the second value the resultant key when used for
// sorting will give perfect vertical ordering and stable horizontal ordering.
//
// NOTE: will become spotty when gid values wrap or exceed 1024 (more than
// 256 ids assigned) but this is deemed inconsequential.

#define MakeSortKey(a, b) (((int)(a) << 8) | (((b) >> 2) & 0xff))

class Gob : public StateMachine // gob
{
	friend class StateMachineMgr;
	friend class GobMgr;

public:
	Gob() secGob;
	virtual ~Gob() secGob;

	dword GetFlags() secGob;
	void Flash() secGob;
	void MarkRedraw() secGob;
	bool Invalidate() secGob;
	bool AdvanceAnimation(Animation *pani) secGob;
	void StartAnimation(Animation *pani, int nStrip, int nFrame, word wfAni) secGob;
	void SetAnimationStrip(Animation *pani, int nStrip) secGob;
	void SetAnimationFrame(Animation *pani, int nFrame) secGob;
	void SetFlags(dword ff) secGob;
	Gid GetId() secGob;
	Side GetSide() secGob;
	Player *GetOwner() secGob;
	void SetOwner(Player *pplr) secGob;

	bool IsGobWithinRange(Gob *pgobTarget, TCoord tcRange) secGob;
	bool IsTargetWithinRange(WPoint *pwptTarget, Gob *pgobTarget, TCoord tcRange) secGob;

	virtual void SetPosition(WCoord wx, WCoord wy) secGob;
	virtual bool Init(IniReader *pini, FindProp *pfind, const char *pszName) = 0;
	virtual bool LoadState(Stream *pstm) secGob;
	virtual bool SaveState(Stream *pstm) secGob;
	virtual bool IsSavable();
	virtual GobType GetType() = 0;
	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) = 0;
	virtual dword GetSortKey() secGob;
	virtual void GetClippingBounds(Rect *prc) secGob;
	virtual void GetUIBounds(WRect *pwrc) secGob;
	virtual void GetPosition(WPoint *pwpt) secGob;
	virtual void GetTilePosition(TPoint *ptpt) secGob;
	virtual void GetTileRect(TRect *ptrc) secGob;
	virtual void GetTilePaddedWRect(WRect *pwrc) secGob;
	virtual void GetCenter(WPoint *pwpt) secGob;
	virtual void PopupMenu() secGob;
	virtual void InitMenu(Form *pfrm) secGob;
	virtual void OnMenuItemSelected(int id) secGob;
#if defined(WIN) && defined(DEBUG)
	virtual void ToString(char *psz);
#endif
#ifdef TRACKSTATE
    virtual void TrackState(StateFrame *frame);
#endif

	// StateMachine methods

#if defined(DEBUG_HELPERS)
	virtual char *GetName() secGob;
#endif

	// Used for tracking visibility

	TRectSmall m_trcBoundingLast;

protected:
	Gid m_gid;
	dword m_ff;
	WCoord m_wx, m_wy;
	Player *m_pplr;

private:
	Gob *m_pgobNext;
	Gid m_gidNext;
};

inline dword Gob::GetFlags() {
	return m_ff;
}

inline void Gob::Flash() {
	m_ff |= kfGobFlashing | kfGobRedraw;
	m_unvl.MinSkip();
}

inline void Gob::SetFlags(dword ff) {
	m_ff = ff;
}

inline Gid Gob::GetId() {
	return m_gid;
}

inline Side Gob::GetSide() {
	return m_pplr->GetSide();
}

inline Player *Gob::GetOwner() {
	return m_pplr;
}

inline void Gob::SetOwner(Player *pplr) {
	m_pplr = pplr;
}

Gob *CreateGob(GobType gt) secGob;

// GobMgr stuff

const Gid kgidNull = (Gid)-1;
const word kwfGidEndMarker = 0x8000;

// AreaMask; assumes 32 areas max

#define kcAreasMax 32
#define knAreaLastDiscovery -1
#if kcAreasMax > 32
#error Code assumes 32 areas max! Could change to 64 easily enough
#endif
typedef dword AreaMask;

struct AreaEntry // are
{
	word iareNext;
	Gid gid;
};

struct Area // ar
{
	TRect trc;
	word iareFree;
	word iareHead;
	word careAlloc;
	word careUsed;
	SideMask sidm;
	UnitMask um;
	AreaEntry *aare;
};

#define knLimitStruct 0
#define knLimitMobileUnit 1
#define knLimitScenery 2
#define knLimitScorch 3
#define knLimitSupport 4

class GobMgr // gobm
{
public:
	GobMgr() secGobMgr;
	~GobMgr() secGobMgr;

	bool Init(TCoord ctx, TCoord cty, int cpgobMax) secGobMgr;
	void Reset() secGobMgr;
	bool SaveState(Stream *pstm) secGobMgr;
	bool LoadState(Stream *pstm) secGobMgr;
	void AddGob(Gob *pgob, Gid gid = kgidNull) secGobMgr;
	void RemoveGob(Gob *pgob) secGobMgr;
	bool MoveGob(Gob *pgob, WCoord wxOld, WCoord wyOld, WCoord wxNew, WCoord wyNew) secGobMgr;
	Gob *GetGob(Gid gid, bool fActiveOnly = true) secGobMgr;
	UnitGob *GetUnitGob(TCoord tx, TCoord ty) secGobMgr;
	void ShadowGob(Gob *pgob, TCoord tx, TCoord ty, int ctx, int cty) secGobMgr;
	void UnshadowGob(Gob *pgob, TCoord tx, TCoord ty, int ctx, int cty) secGobMgr;
	Gob *GetShadowGob(TCoord tx, TCoord ty) secGobMgr;
	void GetAreaRect(int nArea, TRect *ptrc, Side side = ksideNeutral) secGobMgr;
	bool LoadAreas(IniReader *pini) secGobMgr;
	Gob *EnumGobsInArea(Enum *penm, int nArea, SideMask sidm, UnitMask um) secGobMgr;
	void MoveGobBetweenAreas(Gid gid, AreaMask amOld, AreaMask amNew) secGobMgr;
	AreaMask CalcAreaMask(TCoord tx, TCoord ty) secGobMgr;
	AreaMask CalcAreaMask(TCoord tx, TCoord ty, int ctx, int cty) secGobMgr;
	bool CheckUnitsInArea(int nArea, SideMask sidm, UnitMask um) secGobMgr;
	bool IsGobWithinArea(Gob *pgobTarget, int nArea) secGobMgr;

#ifdef DEBUG_HELPERS
	char m_aszAreaNames[kcAreasMax][50];
	char *GetAreaName(int nArea) {
		return m_aszAreaNames[nArea];
	}
#endif

	Gob *GetFirstGob() {
		return m_pgobHead;
	}

	// UNDONE: will making this static generate better code?
	class Gob *GetNextGob(Gob *pgob) {
		return pgob->m_pgobNext;
	}

	// First gid at TCoord

	Gid GetFirstGid(TCoord tx, TCoord ty) {
		Gid gid = m_pgidMap[ty * m_ctx + tx];
		if (gid & kwfGidEndMarker)
			return kgidNull;
		return gid;
	}

	// Next gid in list

	Gid GetNextGid(Gid gid) {
		Gob *pgob = GetGob(gid, false);
		if (pgob == NULL)
			return kgidNull;
		return pgob->m_gidNext;
	}

	void GetMapSize(TCoord *pctx, TCoord *pcty) {
		*pctx = m_ctx;
		*pcty = m_cty;
	}

	int GetGobCount() {
		return m_cpgobActive;
	}

	int FindGobs(const Rect *prcBounds, Gob **apgob, int cpgobMax, byte *pbFogMap) secGobMgr;
	bool IsStructurePlacementValid(StructConsts *pstruc, TCoord tx, TCoord ty, Player *pplr) secGobMgr;
#if defined(DEBUG) && defined(WIN)
	Gob *FindEnemyWithinRange(UnitGob *punt, TCoord tcRange, bool fStructures = false) secGobMgr;
#endif
	int FindGobs(const TRect *ptrcBounds, Gob **apgob, int cpgobMax) secGobMgr;
	void TrackGobCounts(Gob *pgob, bool fIncrement) secGobMgr;
	bool IsBelowLimit(int nLimit, Player *pplr = NULL) secGobMgr;

private:
	void AddGobToAreas(Gid gid, AreaMask am) secGobMgr;
	void RemoveGobFromAreas(Gid gid, AreaMask am) secGobMgr;
	int GetAreasFromMask(AreaMask am, int *pnArea) secGobMgr;
	void FreeAreas() secGobMgr;

	Gob *m_pgobHead;
	Gid *m_pgidMap;
	int m_cpgobMax;
	int m_cpgobActive;		// UNDONE: _DEBUG only?
	Gob **m_ppgobFreeHead;
	Gob **m_ppgobFreeTail;
	TCoord m_ctx, m_cty;
	int m_car;
	Area m_aar[kcAreasMax];
	int m_cSceneryGobs;
	int m_cScorchGobs;
	int m_cSupportGobs;

#ifdef MP_DEBUG_SHAREDMEM
public:
#endif
	Gob **m_apgobMaster;
};
extern GobMgr ggobm;

// GobStateMachineMgr

class GobStateMachineMgr : public StateMachineMgr
{
public:
	virtual StateMachineId GetId(StateMachine *psm) {
		return (StateMachineId)((Gob *)psm)->GetId();
	};
	virtual StateMachine *GetStateMachine(StateMachineId smid) {
		return (StateMachine *)ggobm.GetGob((Gid)smid, false);
	};
};
extern GobStateMachineMgr gsmm;

// Gob derivatives

class OvermindGob : public Gob // ovr
{
public:
	OvermindGob() secOvermindGob;
	virtual ~OvermindGob() secOvermindGob;
	static bool InitClass(IniReader *pini) secOvermindGob;
	bool Init(const char *pszName = NULL) secOvermindGob;
	void Toggle() secOvermindGob;

	// Gob methods

	virtual bool Init(IniReader *pini, FindProp *pfind, const char *pszName) secOvermindGob;
	virtual bool IsSavable() secOvermindGob;
	virtual GobType GetType() secOvermindGob;
	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) secOvermindGob;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secOvermindGob;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secOvermindGob;
#endif

private:
	bool m_fEnabled;
	int m_cAttackCountdown;
	int m_cBuildUnitCountdown;
	bool m_fInitializationComplete;
};

class SurfaceDecalGob : public Gob // dcl
{
public:
	SurfaceDecalGob() secSurfaceDecalGob;
	virtual ~SurfaceDecalGob() secSurfaceDecalGob;
	static bool InitClass(IniReader *pini) secSurfaceDecalGob;
	bool Init(WCoord wx, WCoord wy, dword ff, const char *pszBitmap, TBitmap *ptbm = NULL, const char *pszName = NULL) secSurfaceDecalGob;

	// Gob methods

	virtual bool Init(IniReader *pini, FindProp *pfind, const char *pszName) secSurfaceDecalGob;
	virtual bool IsSavable() secSurfaceDecalGob;
	virtual GobType GetType() secSurfaceDecalGob;
	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) secSurfaceDecalGob;
	virtual void GetClippingBounds(Rect *prc) secSurfaceDecalGob;

protected:
	TBitmap *m_ptbm;
};

// ScorchGob different than SurfaceDecalGob so that m_nScorch can be persisted

class ScorchGob : public SurfaceDecalGob
{
public:
	ScorchGob() secScorchGob;
	bool Init(WCoord wx, WCoord wy, int nScorch) secScorchGob;

	// Scorch methods

	int GetSequence() secScorchGob;

	// Gob methods

	virtual GobType GetType() secScorchGob;
	virtual bool LoadState(Stream *pstm) secScorchGob;
	virtual bool SaveState(Stream *pstm) secScorchGob;
	virtual bool IsSavable() secScorchGob;

private:
	int m_nScorch;
	int m_nSequence;
};

class SceneryGob : public Gob // scn
{
public:
	SceneryGob() secSceneryGob;
	virtual ~SceneryGob() secSceneryGob;
	static bool InitClass(IniReader *pini) secSceneryGob;
	bool Init(WCoord wx, WCoord wy, dword ff, const char *pszBitmap, const char *pszName = NULL) secSceneryGob;

	// Gob methods

	virtual bool Init(IniReader *pini, FindProp *pfind, const char *pszName) secSceneryGob;
	virtual bool LoadState(Stream *pstm) secSceneryGob;
	virtual bool SaveState(Stream *pstm) secSceneryGob;
	virtual GobType GetType() secSceneryGob;
	virtual dword GetSortKey() secSceneryGob;
	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) secSceneryGob;
	virtual void GetClippingBounds(Rect *prc) secSceneryGob;

private:
	TBitmap *m_ptbm;
};

class AnimGob : public Gob // anm
{
public:
	AnimGob() secAnimGob;
	virtual ~AnimGob() secAnimGob;
	bool Init(WCoord wx, WCoord wy, word wfAnm, const char *pszAniName, AnimationData *panid = NULL,
			int nStrip = 0, StateMachineId smidNotify = ksmidNull, const char *pszName = NULL) secAnimGob;
	virtual bool OnStripDone() secAnimGob;

	// Gob methods

	virtual bool Init(IniReader *pini, FindProp *pfind, const char *pszName) secAnimGob;
	virtual bool IsSavable() secAnimGob;
	virtual GobType GetType() secAnimGob;
	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) secAnimGob;
	virtual void GetClippingBounds(Rect *prc) secAnimGob;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secAnimGob;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secAnimGob;
#endif

protected:
	StateMachineId m_smidNotify;
	word m_wfAnm;
	Animation m_ani;
};
AnimGob *CreateAnimGob(WCoord wx, WCoord wy, word wfAnm, const char *pszAniName, AnimationData *panid,
		int nStrip = 0, StateMachineId smidNotify = ksmidNull, const char *pszName = NULL) secAnimGob;

const word kfAnmDeleteWhenDone = 0x0001;
const word kfAnmLoop = 0x0002;
const word kfAnmSmokeFireLayer = 0x0004;
const word kfAnmSurfaceDecalLayer = 0x0008;

const int kcbStructUnitName = 50;

struct GobConsts { // gobc
	GobType gt;
};

#define GetUnitCostMin() (gfMultiplayer ? gnUnitCostMPMin : gnUnitCostMin)
#define GetUnitCostMax() (gfMultiplayer ? gnUnitCostMPMax : gnUnitCostMax)

#define kfUntcNotifyEnemyNearby	0x0001	// wants to be notified of nearby mobile enemies
#define kfUntcNotifyPowerLowHigh 0x0002 // bld is power sensitive, needs notifications
#define kfUntcLargeDefog 0x0004 // this unit defogs a larger area
#define kfUntcMobileUnitBuilder 0x0008 // builds mobile units
#define kfUntcStructureBuilder 0x0010 // builds structures
#define kfUntcHasFullnessIndicator 0x0020 // draws fullness indicator

struct UnitConsts : public GobConsts { // untc
	UnitType ut;
	UnitMask um;
	UnitMask umPrerequisites;
	UpgradeMask upgmPrerequisites;
	Sfx sfxImpact;
	int nCost;
	int nCostMP;
	fix fxArmorStrength;
	fix fxArmorStrengthMP;
	int nInfantryDamage;
	int nInfantryDamageMP;
	int nVehicleDamage;
	int nVehicleDamageMP;
	int nStructureDamage;
	int nStructureDamageMP;
	int ctFiringRate;
	int tcFiringRange;	// kept as int instead of TCoord to accommodate Ini::GetPropertyValue
	int cupdTimeToBuild;
	int cupdTimeToBuildMP;
	char szAniName[kcbFilename];
	AnimationData *panid;
	WRect wrcUIBounds;
	WRect wrcUIBoundsFinger;
	int wdySortOffset;
	char szName[kcbStructUnitName];
	char szLongName[kcbStructUnitName];
	UnitMenu *pfrmMenu;
	word wf;
	char *szDescription;

	int GetCost() {
		return gfMultiplayer ? nCostMP : nCost;
	}
	fix GetArmorStrength() {
		return gfMultiplayer ? fxArmorStrengthMP : fxArmorStrength;
	}
	int GetTimeToBuild() {
		return gfMultiplayer ? cupdTimeToBuildMP : cupdTimeToBuild;
	}
	int GetInfantryDamage() {
		return gfMultiplayer ? nInfantryDamageMP : nInfantryDamage;
	}
	int GetVehicleDamage() {
		return gfMultiplayer ? nVehicleDamageMP : nVehicleDamage;
	}
	int GetStructureDamage() {
		return gfMultiplayer ? nStructureDamageMP : nStructureDamage;
	}
};

struct StructConsts : public UnitConsts { // struc
	int ctx, cty;	// kept as int instead of TCoord to accommodate Ini::GetPropertyValue
	int ctxReserve, ctyReserve;	// kept as int instead of TCoord to accommodate Ini::GetPropertyValue
	int nPowerDemand;
	int nPowerSupply;
	int nUpgradeCost;
	Sfx sfxAbortRepair;
	Sfx sfxRepair;
	Sfx sfxDamaged;
	Sfx sfxDestroyed;
	Sfx sfxSelect;
	Sfx sfxUpgrade;
	Sfx sfxAbortUpgrade;
};

struct BuilderConsts : public StructConsts { // bldc
	int nBuildRate;
	Sfx sfxUnitBuild;
	Sfx sfxUnitBuildAbort;
	Sfx sfxUnitReady;
	UnitMask umCanBuild;
};

struct TowerConsts : public StructConsts { // twrc
	Sfx sfxAttack;
	Sfx sfxFire;
	int *anFiringStripIndices;
};

struct MobileUnitConsts : public UnitConsts { // muntc
	Sfx sfxFire;
	SfxCategory sfxcDestroyed;
	SfxCategory sfxcSelect;
	SfxCategory sfxcMove;
	SfxCategory sfxcAttack;
	WCoord wcMoveDistPerUpdate;
	WCoord wcMoveDistPerUpdateMP;
	int *anFiringStripIndices;
	int *anMovingStripIndices;
	int *anIdleStripIndices;

	WCoord GetMoveDistPerUpdate() {
		return gfMultiplayer ? wcMoveDistPerUpdateMP : wcMoveDistPerUpdate;
	}
};

class MobileUnitBuildForm;
struct MobileUnitBuilderConsts : public BuilderConsts { // mubc
	word fUpgrade;
	word fUpgradeInProgress;
	MobileUnitBuildForm *pfrmBuild;
};

struct SpConsts : public MobileUnitConsts { // spc
	Sfx sfxStructureCaptured;
};

struct MinerConsts : public MobileUnitConsts { // mnrc
	Sfx sfxMine;
	Sfx sfxUnderAttack;
};

const int kcmsDamageIndicatorTimeout = 4000; // 4.0 secs TUNE:

#define kcEnemyNearbyRemember 5 // # of enemies to remember

#define kcyHealthBar 4 // Height of health bar above selection

const WCoord kwcFullnessPip = WcFromTile16ths(2);

#define kcyFullnessBar (PcFromWc(kwcFullnessPip) + 2) // Height of fullness bar below miner, processor, warehouse

class UnitGob : public Gob // unt
{
public:
	UnitGob(UnitConsts *puntc) secUnitGob;
    ~UnitGob() secUnitGob;

	static bool InitClass(UnitConsts *psubc, IniReader *pini) secUnitGob;
    static void InitFingerUIBounds(UnitConsts *psubc);
	static void ExitClass(UnitConsts *psubc) secUnitGob;
	static bool LoadMenu(UnitConsts *psubc, IniReader *pini, char *pszTemplate, int idfDefault) secUnitGob;
	virtual bool Init(WCoord wx, WCoord wy, Player *pplr, fix fxHealth, dword ff, const char *pszName) secUnitGob;
	virtual void Delete() secUnitGob;
	virtual void DefUpdate() secUnitGob;
	virtual void Activate() secUnitGob;
	virtual void Deactivate() secUnitGob;
	virtual bool IsIdle() secUnitGob;
	virtual bool IsValidTarget(Gob *pgobTarget) secUnitGob;
	virtual void SetTarget(Gid gid, WCoord wxTarget = 0, WCoord wyTarget = 0, WCoord wxCenter = 0, WCoord wyCenter = 0, TCoord tcRadius = 0, WCoord wcMoveDistPerUpdate = 0) secUnitGob;
	virtual void GetAttackPoint(WPoint *pwpt) secUnitGob;
    virtual dword GetAnimationHash() secUnitGob;
    virtual void GetAnimationBounds(Rect *prc, bool fBase) secUnitGob;
    virtual void DrawAnimation(DibBitmap *pbm, int x, int y) secUnitGob;
    virtual AnimSprite *CreateHilightSprite(); secUnitGob;
#ifdef MP_DEBUG_SHAREDMEM
	virtual void MPValidate() secUnitGob;
#endif
	void NotifyNearbyAlliesOfHit(Gid gidAssailant) secUnitGob;
	void ShowDamageIndicator() secUnitGob;
	void ClearDamageIndicator() secUnitGob;
	int GetDamageTo(UnitGob *puntTarget) secUnitGob;
	void RecalcEnemyNearby(bool fClearOnly) secUnitGob;

	UnitConsts *GetConsts() {
		return m_puntc;
	}

	UnitType GetUnitType() {
		return m_puntc->ut;
	}

	word GetUnitFlags() {
		return m_wfUnit;
	}

	void SetUnitFlags(word wf) {
		m_wfUnit = wf;
	}

	bool IsAlly(Side side) secUnitGob;

	fix GetHealth() {
		return m_fxHealth;
	}

    AnimSprite *GetAnimSprite() {
        return m_panispr;
    }

	void Select(bool fSelect) secUnitGob;
    void Hilight(bool fHilight) secUnitGob;

	virtual void SetHealth(fix fxHealth) secUnitGob;

	// Gob overrides

	virtual bool Init(IniReader *pini, FindProp *pfind, const char *pszName) secUnitGob;
	virtual bool LoadState(Stream *pstm) secUnitGob;
	virtual bool SaveState(Stream *pstm) secUnitGob;
	virtual GobType GetType() secUnitGob;
	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) secUnitGob;
	virtual dword GetSortKey();
	virtual void GetClippingBounds(Rect *prc) secUnitGob;
	virtual void GetUIBounds(WRect *pwrc) secUnitGob;
	virtual void GetCenter(WPoint *pwpt) secUnitGob;
	virtual void PopupMenu() secUnitGob;

protected:
	void RememberEnemyNearby(Gid gidEnemy) secUnitGob;
	TCoord CalcRange(TCoord tx, TCoord ty, Gob *pgob) secUnitGob;
	virtual UnitGob *FindEnemyNearby(TCoord tc) secUnitGob;
	void NotifyEnemyNearby() secUnitGob;

	union {
		UnitConsts *m_puntc;
		StructConsts *m_pstruc;
		TowerConsts *m_ptwrc;
		SpConsts *m_pspc;
		MinerConsts *m_pmnrc;
		MobileUnitConsts *m_pmuntc;
		BuilderConsts *m_pbldrc;
		MobileUnitBuilderConsts *m_pmubc;
	};
	Gid m_agidEnemyNearby[kcEnemyNearbyRemember];
	fix m_fxHealth;
	Animation m_ani;
    AnimSprite *m_panispr;
	word m_wfUnit;
	int m_cupdLastHitNotify;
	short m_cDamageCountdown;
};

const word kfUnitInvulnerable = 0x0001;
const word kfUnitDrawHealthIndicator = 0x0002;
const word kfUnitDrawRepairingSymbol = 0x0004;
const word kfUnitRepairing = 0x0008;
const word kfUnitDrawNeedsPowerSymbol = 0x0010;
const word kfUnitDrawNeedCreditsSymbol = 0x0020;
const word kfUnitNeedCredits = 0x0040;
const word kfUnitHilighted = 0x0080;

class StructGob : public UnitGob // stru
{
public:
	StructGob(StructConsts *pstruc) secStructures;
	virtual ~StructGob() secStructures;
	static bool InitClass(StructConsts *pstruc, IniReader *pini) secStructures;
	static void ExitClass(StructConsts *pstruc) secStructures;

	void Repair(bool fOn) secStructures;
	bool NeedsRepair() secStructures;
	void EnableOrDisableSymbolLayer() secStructures;
	virtual bool IsTakeoverable(Player *pplr) secStructures;
	virtual void Takeover(Player *pplr) secStructures;

	static bool LoadConfirmation() secStructures;
	bool PopupConfirmation(char *pszTitle) secStructures;
	void TakedownConfirmation() secStructures;
	void SelfDestruct(bool fRecycleValue = true) secStructures;
	bool IsAccessible(TCoord tx, TCoord ty) secStructures;

	// UnitGob overrides

	virtual bool Init(WCoord wx, WCoord wy, Player *pplr, fix fxHealth, dword ff, const char *pszName) secStructures;
	virtual void Delete() secStructures;
	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) secStructures;
	virtual void DefUpdate() secStructures;
	virtual void Activate() secStructures;
	virtual void Deactivate() secStructures;
	virtual bool IsValidTarget(Gob *pgobTarget) secStructures;
	virtual void SetHealth(fix fxHealth) secStructures;
	virtual void GetAttackPoint(WPoint *pwpt) secStructures;

	// Gob overrides

	virtual bool LoadState(Stream *pstm) secStructures;
	virtual bool SaveState(Stream *pstm) secStructures;
	virtual void GetTileRect(TRect *ptrc) secStructures;
	virtual void GetTilePaddedWRect(WRect *pwrc) secStructures;
	virtual void InitMenu(Form *pfrm) secStructures;
	virtual void OnMenuItemSelected(int id) secStructures;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secStructures;

	// Used to track visibility in FindGobs

	word m_nSeqLastVisible;

protected:
	void OnPowerLowHigh() secStructures;
	void DrawSymbol(TBitmap *ptbm, DibBitmap *pbm, int xViewOrigin, int yViewOrigin) secStructures;
	virtual void WaitingForCredits(bool fNeed, bool fOverride = false, Player *pplr = NULL) secStructures;

private:
	ScorchGob *GetScorchGob() secStructures;

	long m_tLastSmoke;

	static TBitmap *s_ptbmRepairing;
	static TBitmap *s_ptbmNeedsPower;
	static TBitmap *s_ptbmNeedCredits;
};

// TUNE:
const fix kfxHealthUnitsRepairedPerUpdate = itofx(1);
const int knCreditsPerRepairUpdate = 1;
const long kcupdSymbolFlashRate = 8; // ideally a power of 2

#define GetReplicationCost(nCost) ((nCost) / 3)

class UnitMenu : public Form, public Timer
{
public:
	void SetOwner(UnitGob *punt, bool fPerUnitInit = true) secUnitGob;
	UnitGob *GetOwner() secUnitGob;

	// Form overrides

	virtual bool DoModal(int *pnResult = NULL, Sfx sfxShow = ksfxGuiFormShow, Sfx sfxHide = ksfxGuiFormHide) secUnitGob;
	virtual void OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd) secUnitGob;
	virtual void OnPaint(DibBitmap *pbm) secUnitGob;
	virtual bool OnHitTest(Event *pevt) secUnitGob;

	// Timer overrides

	virtual void OnTimer(long tCurrent) secUnitGob;

protected:
	UnitGob *m_punt;
	Rect m_rcButtons;
};

enum MobileUnitAction // mua
{
	kmuaNone,
	kmuaGuard,
	kmuaGuardVicinity,
	kmuaGuardArea,
	kmuaMove,
	kmuaAttack,
	kmuaHuntEnemies,
};

struct MoveDirections // movd
{
	MobileUnitGob *pmunt;
	byte idir;
	byte cdir;
	Direction adir[8];
};

const word kfMuntChaseEnemies = 0x0001;			// Will chase enemies when attacking them
const word kfMuntReturnFire = 0x0002;			// Will attack anything that hits it
const word kfMuntAttackEnemiesWhenMoving = 0x0004; // Will attack enemies in range when moving
const word kfMuntAttackEnemiesWhenGuarding = 0x0008; // Will attack enemies in range when guarding
const word kfMuntRunAwayWhenHit = 0x0010;		// Will run away when hit
const word kfMuntMoveWait = 0x0020;				// Is waiting for a tile to free up
const word kfMuntPathPending = 0x0040;			// Will be calcing a path soon
const word kfMuntCommandPending = 0x0080;		// Has a command pending
const word kfMuntFiring	= 0x0100;				// Is shooting at something
const word kfMuntMoveWaitingNearby = 0x0200;	// Gob nearby is "move waiting" on this gob
const word kfMuntDestinationReserved = 0x0400;	// Dest tile reserved
const word kfMuntStuck = 0x0800;				// Stuck bit for debugging
const word kfMuntAtReplicatorInput = 0x1000;	// At replicator input spot
const word kfMuntStayPut = 0x2000;				// Don't move to attack enemies

// All the flags that control a MobileUnit's aggressiveness

const word kfMuntAggressivenessBits = kfMuntChaseEnemies | kfMuntReturnFire |
		kfMuntAttackEnemiesWhenMoving | kfMuntAttackEnemiesWhenGuarding |
		kfMuntRunAwayWhenHit | kfMuntStayPut;

// Most MobileUnits move (follow paths) and like to shoot

extern bool gfGodMode;
#define kcPathsCache 5

class MobileUnitGob : public UnitGob // munt
{
public:
	MobileUnitGob(MobileUnitConsts *pmuntc) secUnitGob;
	virtual ~MobileUnitGob() secUnitGob;
	// NOTE: derivatives should initialize the an*StripIndices arrays of the MobileUnitConsts
	static bool InitClass(MobileUnitConsts *pmuntc, IniReader *pini) secUnitGob;
	static void ExitClass(MobileUnitConsts *pmuntc) secUnitGob;
	static MobileUnitGob *AnyTransitionsIntoTile(TCoord tx, TCoord ty, Gob *pgobIgnore) secUnitGob;
	static void FreeCachedPaths() secUnitGob;
	bool IsReadyForCommand() secUnitGob;
	bool IsMobile() secUnitGob;
	bool IsStandingOnActivator() secUnitGob;

    virtual int GetIdleCountdown() secUnitGob;

	bool HasAttackTarget() {
		return m_gidTarget != kgidNull;
	}

	word GetMobileUnitFlags() {
		return m_wfMunt;
	}

	void SetMobileUnitFlags(word wf) {
		m_wfMunt = wf;
	}

	void ClearAction() {
		m_mua = kmuaNone;
	}

	bool IsHumanOrGodControlled() {
		return !(m_pplr->GetFlags() & kfPlrComputer) || gfGodMode;
	}

	bool IsMoveWaiting() {
		return (m_wfMunt & kfMuntMoveWait) != 0;
	}
#ifdef MP_DEBUG_SHAREDMEM
	virtual void MPValidate() secUnitGob;
#endif

	// UnitGob overrides

	virtual bool Init(IniReader *pini, FindProp *pfind, const char *pszName) secUnitGob;
	virtual bool Init(WCoord wx, WCoord wy, Player *pplr, fix fxHealth, dword ff, const char *pszName) secUnitGob;
#ifdef DRAW_PATHS
	void DrawPath(DibBitmap *pbm, WCoord wxViewOrigin, WCoord wyViewOrigin) secUnitGob;
#endif
#ifdef DRAW_LINES
	void DrawTargetLine(DibBitmap *pbm, int xViewOrigin, int yViewOrigin) secUnitGob;
#endif
	virtual bool IsIdle() secUnitGob;
	virtual void SetTarget(Gid gid, WCoord wxTarget = 0, WCoord wyTarget = 0, WCoord wxCenter = 0, WCoord wyCenter = 0, TCoord tcRadius = 0, WCoord wcMoveDistPerUpdate = 0) secUnitGob;
	virtual void Activate() secUnitGob;
	virtual void Deactivate() secUnitGob;
	virtual void GetAttackPoint(WPoint *pwpt) secUnitGob;

	// Gob overrides

	virtual bool LoadState(Stream *pstm) secUnitGob;
	virtual bool SaveState(Stream *pstm) secUnitGob;
	virtual void SetPosition(WCoord wx, WCoord wy) secUnitGob;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secUnitGob;

protected:
	void MoveEnter(bool fReset = false) secUnitGob;
	void MoveExit() secUnitGob;
	int MoveUpdate() secUnitGob;

	virtual bool Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy) secUnitGob;
	virtual void Idle() secUnitGob;
	virtual void PerformAction(char *szAction) secUnitGob;
	bool InTransition() secUnitGob;

private:
	bool IsAttackPointWithinFiringRangeOfTarget(UnitGob *puntTarget) secUnitGob;
	bool PendOrProcessCommand(Message *pmsg, State stNew) secUnitGob;
	bool PendOrProcessAction(Message *pmsg, State stNew, MobileUnitAction mua) secUnitGob;
	bool IsTargetInRange() secUnitGob;
	void SetStatePendingFireComplete(State st) secUnitGob;
	void ContinueActionPendingFireComplete() secUnitGob;
	void ReserveTile(TCoord tx, TCoord ty, bool fReserve) secUnitGob;
	bool IsTileReserved(TCoord tx, TCoord ty) secUnitGob;
	byte GetTileFlags(TCoord tx, TCoord ty) secUnitGob;
	MobileUnitGob *GetReservingUnit(TCoord tx, TCoord ty) secUnitGob;
	int GetNextLocations(TPoint *atpt) secUnitGob;
	int GetNextLocations2(Path *ppath, int *pitptStart, int cStepsFurtherStop, TPoint *atpt) secUnitGob;
	int FindClosestPoint(TPoint *ptptClosest, int ctptFurtherStop) secUnitGob;
	bool PrepPath(WCoord wxDst, WCoord wyDst) secUnitGob;
	Path *FindPath(TCoord txFrom, TCoord tyFrom, TCoord txTo, TCoord tyTo) secUnitGob;
	bool PrepareNextTransition(TPoint *ptptNext, int *pnMoveResult) secUnitGob;
	void EnterTransition(TPoint *ptptNext) secUnitGob;
	void ContinueTransition() secUnitGob;
	bool HandleCollision(MobileUnitGob **apmunt, byte *abf, TPoint *atpt, int ctpt) secUnitGob;
	bool PlanMoveAsidePath(MobileUnitGob *pmunt, TCoord tx, TCoord ty) secUnitGob;
	bool GetMoveAsideDirections(Direction dirMover, MoveDirections *pmovd) secUnitGob;
	bool AcceptMoveAsideRequest(Direction dir) secUnitGob;
	MobileUnitGob *GetMobileUnitAt(TCoord tx, TCoord ty) secUnitGob;
	bool CheckDestinationReached() secUnitGob;
	bool IsMoveWaitCycle(MobileUnitGob *pmunt) secUnitGob;
	bool MoveWaitForUnit(MobileUnitGob *pmunt, TCoord tx, TCoord ty) secUnitGob;
	bool ContinueMoveWaiting(MobileUnitGob **apmunt, byte *abf, TPoint *atpt, int ctpt, TPoint *ptptNext) secUnitGob;
	void NotifyMoveWaitingNearby(WCoord wx, WCoord wy) secUnitGob;
	bool FindLocalAvoidPath(TCoord tx, TCoord ty) secUnitGob;
	int FindCloserTrackPoint(TrackPoint *ptrkpStart, Direction dirBlocked, bool fClockwise, int ctpt, TPoint *atpt, TrackPoint *ptrkp) secUnitGob;
	bool CheckReplicatorPoint() secUnitGob;
	UnitGob *FindValidTargetInArea(int nArea) secUnitGob;

protected:
	Direction16 m_dir, m_dirNext;
	Gid m_gidTarget;
	long m_tLastFire;
	Message m_msgPending;
	Message m_msgAction;
	int m_cCountdown;		// multi-purpose countdown timer
	int m_cMoveStepsRemaining;
	TPoint m_tptChaseInitial;
	MobileUnitAction m_mua, m_muaPending;
	word m_wfMunt;
	State m_stPending;
	int m_cupdLastHitOrNearbyAllyHit;

#ifdef DRAW_PATHS
public:
	WCoord m_wxDst, m_wyDst;
protected:
#endif

#ifdef DEBUG_HELPERS
public:
#endif
	TCoord m_txDst, m_tyDst;
#ifdef DEBUG_HELPERS
protected:
#endif

#ifdef DRAW_LINES
public:
#endif
	WPoint m_wptTarget;

#ifdef DRAW_PATHS
public:
#endif
	word m_nSeqMoveAside;
	Path *m_ppathUnit;
	Path *m_ppathAvoid;
	int m_itptPath;
	WPoint m_wptTargetCenter;
	TCoord m_tcTargetRadius;
	WCoord m_wcMoveDistPerUpdate;

	static Path *s_apathCached[kcPathsCache];
	static int s_cpathCached;

#ifdef DEBUG_HELPERS
	friend void paint();
	friend class UnitGroupViewer;
#endif
};

// Values returned by UnitGob::MoveUpdate()

const int knMoveMoving = 0;
const int knMoveTargetReached = 1;
const int knMoveStuck = 2;
const int knMoveWaiting = 3;

const int kxInvalid = -1000;
const WCoord kwxInvalid = -1000;
const TCoord ktxInvalid = -100;
const word kcBuildQueueMax = 10;

class BuildQueue
{
public:
	BuildQueue() secBuildQueue;
	BuildQueue &operator=( BuildQueue &bqRHS) secBuildQueue;
	void SetSize(word cutMax) secBuildQueue;
	int GetRemainingCapacity() secBuildQueue;
	void Enqueue(UnitType ut) secBuildQueue;
	void Dequeue() secBuildQueue;
	bool RemoveUnit(UnitType ut) secBuildQueue;
	UnitType Peek() secBuildQueue;
	bool IsEmpty() secBuildQueue;
	bool IsFull() secBuildQueue;
	void Clear() secBuildQueue;
	int GetUnitCount(UnitType ut) secBuildQueue;

	// called by builder gob
	virtual bool LoadState(Stream *pstm) secBuildQueue;
	virtual bool SaveState(Stream *pstm) secBuildQueue;

private:
	char m_achBuildQueue[kcBuildQueueMax];
	word m_cchQueueMax;
};

class BuilderGob : public StructGob // bldr
{
public:
	BuilderGob(BuilderConsts *pbldrc) secBuilderGob;
	virtual ~BuilderGob() secBuilderGob;
	static bool InitClass(BuilderConsts *pbldrc, IniReader *pini) secBuilderGob;
	static void ExitClass(BuilderConsts *pbldrc) secBuilderGob;
	void DrawUpgradeEffect(DibBitmap *pbm, int xViewOrigin, int yViewOrigin) secBuilderGob;
	void AbortBuild(bool fRefundCreditsSpent = false, UnitType ut = kutNone) secBuilderGob;
	static int GetGlobalQueuedCount(Player *pplr, word wfTest) secBuilderGob;
	static int GetQueuedCount(Player *pplr, word wfTest) secBuilderGob;

	void SetRallyPoint(TCoord tx, TCoord ty) {
		m_tptRally.tx = tx;
		m_tptRally.ty = ty;
	}

	// in builder gob so it will be tracked per-instance

	void SetLastSelection(short iSel) { m_iLastSelection = iSel;}
	short GetLastSelection() {return m_iLastSelection;}

	// Build management methods

	virtual void FindInitPosition(WPoint *pwpt) secBuilderGob;
	virtual void Build(UnitType ut, Gid gid = kgidNull) secBuilderGob;
	bool IsBuildInProgress() {
		return !m_bq.IsEmpty();
	}
	UnitType UnitBuildInProgress() {return m_bq.Peek();}
	void DrawBuildProgress(DibBitmap *pbm, Rect *prc) secBuilderGob;
	void SyncBuildQueue(BuildQueue *pbq) secBuilderGob;

	UnitGob *GetBuiltGob() secBuilderGob;
	void ClearBuiltGob() secBuilderGob;

	// UnitGob overrides

	virtual bool Init(WCoord wx, WCoord wy, Player *pplr, fix fxHealth, dword ff, const char *pszName) secBuilderGob;
	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) secBuilderGob;
	virtual void DefUpdate() secBuilderGob;
	virtual void Deactivate() secBuilderGob;

	// Gob overrides

	virtual bool LoadState(Stream *pstm) secBuilderGob;
	virtual bool SaveState(Stream *pstm) secBuilderGob;
    virtual void GetClippingBounds(Rect *prc);

protected:
    bool ShowingBuildProgress();

	short m_iLastSelection;
	Gid m_gidBuildVisible;
	fix m_fxHealthBuilding;
	int m_nCreditsSpentOnBuilding;
 	int m_nCostRemainder;
	TPoint m_tptRally;

#ifdef MP_STRESS
public:
#endif
	BuildQueue m_bq;
};

class MobileUnitBuildForm : public Form
{
public:
	BuilderGob *GetOwner() {
		return m_pbldr;
	}

	void SetOwner(BuilderGob *pbldr) secStructures;
	void UpdateUnitInfo(ListItem *pli) secStructures;
	void OnUnitCompleted(BuilderGob *pbldr, UnitType ut) secStructures;
	void DefUpdate(BuilderGob *pbldr, bool fBuildInProgress) secStructures;
	int CountQueue(UnitType ut) secStructures;
	void UpdateOrderButton(bool fCalcLimit) secStructures;

	// Form overrides

	void EndForm(int nResult = 0) secStructures;
	virtual void OnControlSelected(word idc) secStructures;
	virtual void OnControlNotify(word idc, int nNotify);
	virtual void OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd) secStructures;

private:
	int m_cupdLast;
	bool m_fLimitReached;
	bool m_fOrderValid;
	BuilderGob *m_pbldr;
	BuildQueue m_bqPrivate;

	static int s_iliCurrent;
};

class TankGob : public MobileUnitGob // tnk
{
public:
	TankGob(MobileUnitConsts *pmuntc) secTankGob;
	static bool InitClass(MobileUnitConsts *pmuntc, IniReader *pini) secTankGob;
	static void ExitClass(MobileUnitConsts *pmuntc) secTankGob;
	virtual void LaunchProjectile(WCoord wx, WCoord wy, WCoord wxTarget, WCoord wyTarget, int nDamage, Gid gidOwner, Gid gidTarget) secTankGob;

	// UnitGob overrides

	virtual bool Init(WCoord wx, WCoord wy, Player *pplr, fix fxHealth, dword ff, const char *pszName) secTankGob;
	virtual void DefUpdate() secTankGob;

	// UnitGob overrides

	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) secTankGob;
	virtual bool Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy) secTankGob;
	virtual void Idle() secTankGob;
    virtual dword GetAnimationHash() secTankGob;
    virtual void GetAnimationBounds(Rect *prc, bool fBase) secTankGob;
    virtual void DrawAnimation(DibBitmap *pbm, int x, int y) secTankGob;

	// Gob overrides

	virtual void GetClippingBounds(Rect *prc) secTankGob;
	virtual bool LoadState(Stream *pstm) secTankGob;
	virtual bool SaveState(Stream *pstm) secTankGob;

protected:
	Animation m_aniTurret;

    // Note: If this is a char, iphone/llvm will encounter an alignment bug
    // between this member and MTankGob::m_pgobSecondShot. Direction16
    // has been changed to an int for this reason.

	Direction16 m_dir16Turret;
};

class SRInfantryGob : public MobileUnitGob // sri
{
public:
	SRInfantryGob() secInfantryGob;
	static bool InitClass(IniReader *pini) secInfantryGob;
	static void ExitClass() secInfantryGob;

	// UnitGob overrides

	virtual bool Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy) secInfantryGob;
	virtual void Idle() secInfantryGob;

    // MobileUnitGob overrides

    virtual int GetIdleCountdown() secInfantryGob;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secInfantryGob;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secInfantryGob;
#endif
};

class LRInfantryGob : public MobileUnitGob // lri
{
public:
	LRInfantryGob() secInfantryGob;
	static bool InitClass(IniReader *pini) secInfantryGob;
	static void ExitClass() secInfantryGob;

	// UnitGob overrides

	virtual bool Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy) secInfantryGob;
	virtual void Idle() secInfantryGob;

    // MobileUnitGob overrides

    virtual int GetIdleCountdown() secInfantryGob;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secInfantryGob;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secInfantryGob;
#endif
};

class SpInfantryGob : public MobileUnitGob // lri
{
public:
	SpInfantryGob() secInfantryGob;
	static bool InitClass(IniReader *pini) secInfantryGob;
	static void ExitClass() secInfantryGob;

	// UnitGob overrides

	virtual bool IsValidTarget(Gob *pgobTarget) secInfantryGob;
	virtual bool Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy) secInfantryGob;
	virtual void Idle() secInfantryGob;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secInfantryGob;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secInfantryGob;
#endif
};

class LTankGob : public TankGob // ltnk
{
public:
	LTankGob() secTankGob;
	static bool InitClass(IniReader *pini) secTankGob;
	static void ExitClass() secTankGob;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secGob;
#endif
};

class TankShotGob; // it's down there

class MTankGob : public TankGob // mtnk
{
public:
	MTankGob() secTankGob;
	~MTankGob() secTankGob;
	static bool InitClass(IniReader *pini) secTankGob;
	static void ExitClass() secTankGob;
	void LaunchProjectile(WCoord wx, WCoord wy, WCoord wxTarget, WCoord wyTarget, int nDamage, Gid gidOwner, Gid gidTarget) secTankGob;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secGob;
#endif

	// UnitGob overrides

	virtual void DefUpdate() secTankGob;
	virtual void Deactivate() secTankGob;

private:
	TankShotGob *m_pgobSecondShot;
	short m_cShotcountdown;
};

class RTankGob : public TankGob // rtnk
{
public:
	RTankGob() secTankGob;
	static bool InitClass(IniReader *pini) secTankGob;
	static void ExitClass() secTankGob;
	void LaunchProjectile(WCoord wx, WCoord wy, WCoord wxTarget, WCoord wyTarget, int nDamage, Gid gidOwner, Gid gidTarget) secTankGob;

#if defined(DEBUG_HELPERS)
	virtual char *GetName() secGob;
#endif
};

class GTankGob : public TankGob // gtnk
{
public:
	GTankGob() secTankGob;
	static bool InitClass(IniReader *pini) secTankGob;
	static void ExitClass() secTankGob;

	// MobileUnitGob overrides

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secTankGob;

	// TankGob overrides

	virtual bool Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy) secTankGob;

#if defined(DEBUG_HELPERS)
	virtual char *GetName() secGob;
#endif
};

class MinerGob : public MobileUnitGob // mnr
{
public:
	MinerGob() secMiner;
	static bool InitClass(IniReader *pini) secMiner;
	static void ExitClass() secMiner;

	int GetGalaxiteAmount() secMiner;
	void SetGalaxiteAmount(int nAmount) secMiner;
	void SetFavoriteProcessor(Gid gid) secMiner;
	void Hide(bool fHide) secMiner;
	void SendDeliverCommand(Gid gidProcessor) secMiner;

	bool IsAttemptingToDeliver(Gid gid) {
		if (m_fAttemptingToDeliver && gid == m_gidFavoriteProcessor)
			return true;
		return false;
	}

	// Gob overrides

	virtual void InitMenu(Form *pfrm) secMiner;
	virtual void OnMenuItemSelected(int id) secMiner;
	virtual bool LoadState(Stream *pstm) secMiner;
	virtual bool SaveState(Stream *pstm) secMiner;
	virtual void GetClippingBounds(Rect *prc) secMiner;

	// UnitGob overrides

	virtual bool IsValidTarget(Gob *pgobTarget) secMiner;
	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) secMiner;
	virtual void Deactivate() secMiner;
	virtual void SetTarget(Gid gid, WCoord wxTarget = 0, WCoord wyTarget = 0, WCoord wxCenter = 0, WCoord wyCenter = 0, TCoord tcRadius = 0, WCoord wcMoveDistPerUpdate = 0) secMiner;

#ifdef MP_DEBUG_SHAREDMEM
	virtual void MPValidate() secMiner;
#endif

	// MobileUnitGob overrides

	void PerformAction(char *szAction) secMiner;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secMiner;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secGob;
#endif

private:
	void Mine(WCoord wx, WCoord wy) secMiner;
	Gid FindClosestProcessor() secMiner;

private:
	Animation m_aniVacuum;
	int m_nGalaxiteAmount;
	char m_cDelay;
	Gid m_gidFavoriteProcessor;
	TPoint m_tptGalaxite;
	bool m_fMinerUnderAttack;
	static AnimationData *s_panidVacuum;
	bool m_fAttemptingToDeliver;
#ifdef MP_STRESS
public:
#endif
	bool m_fHidden;
};

inline void MinerGob::SetGalaxiteAmount(int nAmount)
{
	m_nGalaxiteAmount = nAmount;
}

inline int MinerGob::GetGalaxiteAmount()
{
	return m_nGalaxiteAmount;
}

inline void MinerGob::SetFavoriteProcessor(Gid gid)
{
	m_gidFavoriteProcessor = gid;
}

// TUNE:

const int knMinerGalaxiteMax = 100;	// two full patches of Galaxite
const int knGalaxiteValue = 10;
const int kcMinerSuckDelay = 7; // number of updates between Galaxite decrements

class MobileHqGob : public MobileUnitGob // mhq
{
public:
	MobileHqGob() secGob;
	static bool InitClass(IniReader *pini) secGob;
	static void ExitClass() secGob;
	bool CanTransform(TPoint *ptp = NULL) secGob;

	// Gob overrides

	virtual void InitMenu(Form *pfrm) secGob;
	virtual void OnMenuItemSelected(int id) secGob;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secGob;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secGob;
#endif
};

class MobileUnitBuilderGob : public BuilderGob // mub
{
public:
	MobileUnitBuilderGob(MobileUnitBuilderConsts *pmubc) secStructures;

	// Gob overrides

	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) secStructures;
	virtual void InitMenu(Form *pfrm) secStructures;
	virtual void OnMenuItemSelected(int id) secStructures;
	virtual void DefUpdate() secStructures;

	// UnitGob overrides

	virtual void Deactivate() secStructures;

	// StructGob overrides

	virtual void Takeover(Player *pplr) secStructures;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secStructures;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secStructures;
#endif
};

class HrcGob : public MobileUnitBuilderGob // hrc
{
public:
	HrcGob() secStructures;
	static bool InitClass(IniReader *pini) secStructures;
	static void ExitClass() secStructures;

	// StateMachine methods

#if defined(DEBUG_HELPERS)
	virtual char *GetName() secStructures;
#endif
private:
	static MobileUnitBuildForm *s_pfrmBuild;
};

class ReactorGob : public StructGob // rctr
{
public:
	ReactorGob() secStructures;
	static bool InitClass(IniReader *pini) secStructures;
	static void ExitClass() secStructures;

	// StateMachine methods

#if defined(DEBUG_HELPERS)
	virtual char *GetName() secStructures;
#endif
};

class ProcessorGob : public StructGob // prcr
{
public:
	ProcessorGob() secStructures;
	static bool InitClass(IniReader *pini) secStructures;
	static void ExitClass() secStructures;

    // UnitGob overrides

    virtual dword GetAnimationHash();
    virtual void GetAnimationBounds(Rect *prc, bool fBase);
    virtual void DrawAnimation(DibBitmap *pbm, int x, int y);

	// StructGob overrides

	virtual bool IsTakeoverable(Player *pplr) secStructures;
	virtual void Takeover(Player *pplr) secStructures;

	// Gob overrides

	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) secStructures;
	virtual bool LoadState(Stream *pstm) secStructures;
	virtual bool SaveState(Stream *pstm) secStructures;
	virtual void GetClippingBounds(Rect *prc) secStructures;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secStructures;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secStructures;
#endif

private:
	void NotifyMinersAttemptingDelivery(bool fDying) secStructures;

	Animation m_aniOverlay;
	Gid m_gidMiner;
	WPoint m_wptFakeMiner;
	bool m_fProcessingAnimationInProgress;
	bool m_fDoorMoving;
};

class StructureBuildForm;

class HqGob : public BuilderGob // hq
{
public:
	HqGob() secStructures;
	virtual ~HqGob() secStructures;

	static bool InitClass(IniReader *pini) secStructures;
	static void ExitClass() secStructures;

	// Gob overrides

	virtual void InitMenu(Form *pfrm) secGob;
	virtual void OnMenuItemSelected(int id) secGob;

	// UnitGob overrides

	virtual void Deactivate() secStructures;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secStructures;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secStructures;
#endif
private:

	void RemoveScorch(StructGob *pstru) secStructures;

	static StructureBuildForm *s_pfrmBuild;
};

class PlaceStructureForm : public Form, Timer
{
public:
	PlaceStructureForm() secStructures;

	void SetOwner(BuilderGob *pgobOwner, UnitType ut) secStructures;
	BuilderGob *GetOwner() {
		return m_pgobOwner;
	}
	void UpdatePlacementIndicator(WCoord wxViewStart, WCoord wyViewStart, WCoord wxView, WCoord wyView) secStructures;

    void SetPassOnInput(bool fPassOn) { m_fPassOnInput = fPassOn; }
    bool IsBusy() { return m_pgobOwner != NULL; }
	void OnPaintSimUI(DibBitmap *pbm) secStructures;
	void OnPaintControlsSimUI(DibBitmap *pbm, UpdateMap *pupd) secStructures;
    void UpdatePosition(Event *pevt);

	// Timer implementation

	virtual void OnTimer(long tCurrent) secStructures;

	// Form overrides

	virtual void OnControlSelected(word idc) secStructures;
	virtual bool OnPenEvent(Event *pevt) secStructures;
	virtual void OnScroll(int dx, int dy) secStructures;
	virtual bool OnHitTest(Event *pevt) secStructures;

private:
    bool FingerHitTest(Event *pevt);
    void GetSubRects(WCoord wx, WCoord wy, Rect *prcInside, Rect *prcOutside);

	BuilderGob *m_pgobOwner;
	StructConsts *m_pstruc;
	TCoord m_tx, m_ty;
	TCoord m_wxPen, m_wyPen;
	int m_wxDragStart, m_wyDragStart;
	TCoord m_txStart, m_tyStart;
	bool m_fDragging;
    bool m_fPassOnInput;
};

class WaitForm : public Form
{
public:
	WaitForm(char *pszTitle, bool fFullScreen = false) secForm;

	// Form overrides

	virtual bool Init(FormMgr *pfrmm, IniReader *pini, word idf) secForm;
	virtual void OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd) secForm;

private:
	dword m_ctWaitPopup;
	char *m_pszTitle;
	bool m_fFullScreen;
	Rect m_rcOrig;
};

class TransportWaitingUI
{
public:
	TransportWaitingUI(int nWaitStr, bool fShow = true) secComm;
	TransportWaitingUI(char *psz, bool fShow = true) secComm;
	~TransportWaitingUI() secComm;
	void Show() secComm;
	void Hide() secComm;

private:
	Form *m_pfrm;
	char *m_psz;
};

class RadarGob : public StructGob // rdr
{
public:
	RadarGob() secRadar;

	static bool InitClass(IniReader *pini) secRadar;
	static void ExitClass() secRadar;

	// Struct gob methods

	virtual void Activate() secRadar;
	virtual void Deactivate() secRadar;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secRadar;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secRadar;
#endif
};

class ResearchGob : public StructGob // rsrch
{
public:
	ResearchGob() secStructures;
	static bool InitClass(IniReader *pini) secStructures;
	static void ExitClass() secStructures;
	virtual void UpgradeUpdate() secStructures;
	virtual void AbortUpgrade() secStructures;

	// Gob overrides - these need to become virtual if this gets a derived class!

	virtual void InitMenu(Form *pfrm) secStructures;
	virtual void OnMenuItemSelected(int id) secStructures;
	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) secStructures;
	virtual void Deactivate() secStructures;
	virtual bool LoadState(Stream *pstm) secStructures;
	virtual bool SaveState(Stream *pstm) secStructures;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secStructures;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secStructures;
#endif

private:
	int m_nCreditsSpentOnUpgrade;
	UnitType m_utTarget;
};

class VtsGob : public MobileUnitBuilderGob // vts
{
public:
	VtsGob() secStructures;
	static bool InitClass(IniReader *pini) secStructures;
	static void ExitClass() secStructures;

private:
	static MobileUnitBuildForm *s_pfrmBuild;
};

// TUNE: tankshot movement rate

const WCoord kwcTankShotRate = kwcTile / 2;

class TankShotGob : public Gob
{
public:
	TankShotGob() secGob;
	static bool InitClass(IniReader *pini) secGob;
	static void ExitClass() secGob;
	bool Init(AnimationData *panid, WCoord wx, WCoord wy, WCoord wxTarget, WCoord wyTarget, int nDamage, Gid gidOwner, Gid gidTarget, WCoord wcMoveRate) secGob;
	void Launch() secGob;

	// Gob methods

	virtual bool Init(IniReader *pini, FindProp *pfind, const char *pszName) secGob;
	virtual GobType GetType() secGob;
	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) secGob;
	virtual void GetClippingBounds(Rect *prc) secGob;
	virtual bool IsSavable() secGob;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secGob;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secGob;
#endif

private:
	Gid m_gidOwner;
	Gid m_gidTarget;
	WLineIterator m_li;
	Animation m_ani;
	int m_nDamage;

public:
	static AnimationData *s_panidShot;
};
TankShotGob *CreateTankShotGob(WCoord wx, WCoord wy, WCoord wxTarget, WCoord yTarget, int nDamage, Gid gidOwner, Gid gidTarget) secGob;

class BulletGob : public TankShotGob
{
public:
	static bool InitClass(IniReader *pini) secGob;
	static void ExitClass() secGob;

	// Gob methods

	virtual GobType GetType() secGob;

	// StateMachine methods

#if defined(DEBUG_HELPERS)
	virtual char *GetName() secGob;
#endif

public:
	static AnimationData *s_panidShot;
};
BulletGob *CreateBulletGob(WCoord wx, WCoord wy, WCoord wxTarget, WCoord yTarget, int nDamage, Gid gidOwner, Gid gidTarget, bool fEagle = false) secGob;

class AndyShotGob : public TankShotGob
{
public:
	static bool InitClass(IniReader *pini) secGob;
	static void ExitClass() secGob;

	// Gob methods

	virtual GobType GetType() secGob;

	// StateMachine methods

#if defined(DEBUG_HELPERS)
	virtual char *GetName() secGob;
#endif

public:
	static AnimationData *s_panidShot;
};
AndyShotGob *CreateAndyShotGob(WCoord wx, WCoord wy, WCoord wxTarget, WCoord yTarget, int nDamage, Gid gidOwner, Gid gidTarget, bool fEagle = false) secGob;

class RocketGob : public Gob
{
public:
	RocketGob() secGob;
	static bool InitClass(IniReader *pini) secGob;
	static void ExitClass() secGob;
	bool Init(WCoord wx, WCoord wy, WCoord wxTarget, WCoord wyTarget, int nDamage, Gid gidOwner, Gid gidTarget) secGob;

	// Gob methods

	virtual bool Init(IniReader *pini, FindProp *pfind, const char *pszName) secGob;
	virtual bool IsSavable() secGob;
	virtual GobType GetType() secGob;
	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) secGob;
	virtual void GetClippingBounds(Rect *prc) secGob;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secGob;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secGob;
#endif

private:
	Gid m_gidOwner;
	Gid m_gidTarget;
	WLineIterator m_li;
	Animation m_ani;
	int m_nDamage;

	static AnimationData *s_panidRocket;
	static int s_nTrailStrip;
};
RocketGob *CreateRocketGob(WCoord wx, WCoord wy, WCoord wxTarget, WCoord wyTarget, int nDamage, Gid gidOwner, Gid gidTarget) secGob;

// SmokeGob

class SmokeGob : public AnimGob
{
public:
	SmokeGob(int cLoops) secGob;
	bool Init(WCoord wx, WCoord wy, StateMachineId smidNotify) secGob;

	// Gob overrides

	virtual bool LoadState(Stream *pstm) secGob;
	virtual bool SaveState(Stream *pstm) secGob;
	virtual bool IsSavable() secGob;
	virtual GobType GetType() secGob;

	// AnimGob overrides

	virtual bool OnStripDone() secGob;

private:
	int m_cLoops;
};

// PuffGob

class PuffGob : public AnimGob
{
public:
	PuffGob() secGob;
	bool Init(WCoord wx, WCoord wy, StateMachineId smidNotify) secGob;

	// Gob overrides

	virtual bool LoadState(Stream *pstm) secGob;
	virtual bool SaveState(Stream *pstm) secGob;
	virtual bool IsSavable() secGob;
	virtual GobType GetType() secGob;
};

#if 0 // not used anymore
class RicochetGob : public AnimGob
{
public:
	bool Init(WCoord wx, WCoord wy, word wfAnm, const char *pszAniName, AnimationData *panid = NULL,
			StateMachineId smidNotify = ksmidNull, const char *pszName = NULL) secGob;

	// AnimGob overrides

	virtual bool OnStripDone() secGob;

private:
	bool m_fNew;
};
#endif

// WarehouseGob

class WarehouseGob : public StructGob
{
public:
	WarehouseGob() secWarehouse;
	static bool InitClass(IniReader *pini) secWarehouse;
	static void ExitClass() secWarehouse;

	// StructGob overrides

	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) secWarehouse;
	virtual void Takeover(Player *pplr) secWarehouse;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secWarehouse;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secWarehouse;
#endif
};

// TowerGob

class TowerGob : public StructGob
{
public:
	TowerGob(TowerConsts *ptwrc) secTowers;
	static bool InitClass(TowerConsts *ptwrc, IniReader *pini) secTowers;
	static void ExitClass(TowerConsts *ptwrc) secTowers;
	virtual bool Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy) = 0;
#ifdef DRAW_LINES
	void DrawTargetLine(DibBitmap *pbm, int xViewOrigin, int yViewOrigin) secTowers;
#endif

	// Gob overrides

	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) secTowers;
	virtual bool LoadState(Stream *pstm) secTowers;
	virtual bool SaveState(Stream *pstm) secTowers;
	virtual void GetClippingBounds(Rect *prc) secTowers;

	// UnitGob overrides

	virtual void Activate() secTowers;
	virtual void DefUpdate() secTowers;
	virtual void SetTarget(Gid gid, WCoord wxTarget = 0, WCoord wyTarget = 0, WCoord wxCenter = 0, WCoord wyCenter = 0, TCoord tcRadius = 0, WCoord wcMoveDistPerUpdate = 0) secTowers;
	virtual bool IsValidTarget(Gob *pgobTarget) secTowers;
    virtual dword GetAnimationHash() secTowers;
    virtual void GetAnimationBounds(Rect *prc, bool fBase) secTowers;
    virtual void DrawAnimation(DibBitmap *pbm, int x, int y) secTowers;

	// StructGob override

	virtual bool IsTakeoverable(Player *pplr) secTowers;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secTowers;

protected:

	// UnitGob protected override

	virtual UnitGob *FindEnemyNearby(TCoord tcRange) secTowers;

	// members

	Animation m_aniTurret;
	Direction16 m_dir16Turret;
	Gid m_gidTargetPrimary;
	Gid m_gidTargetSecondary;
	long m_tLastFire;

#ifdef DRAW_LINES
public:
#endif
	WPoint m_wptTarget;
};

// GunTowerGob

class GunTowerGob : public TowerGob
{
public:
	GunTowerGob() secTowers;
	static bool InitClass(IniReader *pini) secTowers;
	static void ExitClass() secTowers;

	// TowerGob overrides

	virtual bool Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy) secTowers;

#if defined(DEBUG_HELPERS)
	virtual char *GetName() secTowers;
#endif
};

// RocketTowerGob

class RocketTowerGob : public TowerGob
{
public:
	RocketTowerGob() secTowers;
	static bool InitClass(IniReader *pini) secTowers;
	static void ExitClass() secTowers;

	// TowerGob overrides

	virtual bool Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy) secTowers;

#if defined(DEBUG_HELPERS)
	virtual char *GetName() secTowers;
#endif
};

class ArtilleryGob : public MobileUnitGob // art
{
public:
	ArtilleryGob() secArtilleryGob;
	static bool InitClass(IniReader *pini) secArtilleryGob;
	static void ExitClass() secArtilleryGob;

	// UnitGob overrides

	virtual bool Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy) secArtilleryGob;

	// StateMachine methods

//	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secArtilleryGob;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secArtilleryGob;
#endif
};

class ArtilleryShotGob : public TankShotGob
{
public:
	static bool InitClass(IniReader *pini) secArtilleryGob;
	static void ExitClass() secArtilleryGob;

	// Gob methods

	virtual GobType GetType() secArtilleryGob;

	// StateMachine methods

#if defined(DEBUG_HELPERS)
	virtual char *GetName() secArtilleryGob;
#endif

public:
	static AnimationData *s_panidShot;
};
ArtilleryShotGob *CreateArtilleryShotGob(WCoord wx, WCoord wy, WCoord wxTarget, WCoord yTarget, int nDamage, Gid gidOwner, Gid gidTarget) secArtilleryGob;

class AndyGob : public MobileUnitGob
{
public:
	AndyGob() secInfantryGob;
	AndyGob(MobileUnitConsts *pmuntc) secInfantryGob;
	static bool InitClass(IniReader *pini) secInfantryGob;
	static void ExitClass() secInfantryGob;

	// UnitGob overrides

	virtual bool Init(WCoord wx, WCoord wy, Player *pplr, fix fxHealth, dword ff, const char *pszName) secInfantryGob;
	virtual bool Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy) secInfantryGob;
	virtual void Idle() secInfantryGob;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secInfantryGob;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secInfantryGob;
#endif
};

class FoxGob : public AndyGob // fox
{
public:
	FoxGob() secInfantryGob;
	static bool InitClass(IniReader *pini) secInfantryGob;
	static void ExitClass() secInfantryGob;

#if defined(DEBUG_HELPERS)
	virtual char *GetName() secInfantryGob;
#endif
};

class ReplicatorGob : public StructGob // rep
{
public:
	ReplicatorGob() secReplicatorGob;
	static bool InitClass(IniReader *pini) secReplicatorGob;
	static void ExitClass() secReplicatorGob;
	void GetInputTilePosition(TPoint *ptpt) secReplicatorGob;
	void Enable(bool fEnable) secReplicatorGob;

	// UnitGob overrides

	virtual bool Init(WCoord wx, WCoord wy, Player *pplr, fix fxHealth, dword ff, const char *pszName) secReplicatorGob;
	virtual void Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer) secReplicatorGob;
	virtual void GetClippingBounds(Rect *prc) secReplicatorGob;
	virtual bool LoadState(Stream *pstm) secReplicatorGob;
	virtual bool SaveState(Stream *pstm) secReplicatorGob;
    virtual AnimSprite *CreateHilightSprite() { return NULL; }

	// StructGob overrides

	virtual void Activate() secReplicatorGob;
	virtual void Deactivate() secReplicatorGob;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secReplicatorGob;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secReplicatorGob;
#endif

	// For query of replicator input points

	static void ClearReplicatorCount() {
		s_cReplicators = 0;
	}

	static int GetReplicatorCount() {
		return s_cReplicators;
	}
	static void GetReplicatorInputPoint(int n, TPoint *ptpt) {
		*ptpt = s_atptInput[n];
	}

protected:
	virtual void WaitingForCredits(bool fNeed, bool fOverride = false, Player *pplr = NULL) secReplicatorGob;

private:
	bool ClearOutputBay(TCoord txBay, TCoord tyBay, TCoord txOut, TCoord tyOut) secReplicatorGob;

	bool m_fEnabled;
	bool m_fReplicating;
	int m_ifrmLights;
	Player *m_pplrNeedCredits;

	// For management of replicator input points

	static void AddReplicatorInputPoint(TCoord tx, TCoord ty) secReplicatorGob;
	static void RemoveReplicatorInputPoint(TCoord tx, TCoord ty) secReplicatorGob;
	static int s_cReplicators;
	static TPoint s_atptInput[10];
};

const int kdtxReplicatorInput = 2;
const int kdtyReplicatorInput = 0;
const int kdtxReplicatorOutput1 = 1;
const int kdtyReplicatorOutput1 = 2; // 1;
const int kdtxReplicatorOutput2 = 3;
const int kdtyReplicatorOutput2 = 2; // 1;

// ActivatorGob

class ActivatorGob : public AnimGob
{
public:
	// Gob overrides

	ActivatorGob() {m_fActivated = false;};
	virtual bool Init(IniReader *pini, FindProp *pfind, const char *pszName) secReplicatorGob;
	virtual bool LoadState(Stream *pstm) secReplicatorGob;
	virtual bool SaveState(Stream *pstm) secReplicatorGob;
	virtual bool IsSavable() secReplicatorGob;
	virtual GobType GetType() secReplicatorGob;

	// StateMachine methods

	virtual int ProcessStateMachineMessage(State st, Message *pmsg) secReplicatorGob;
#if defined(DEBUG_HELPERS)
	virtual char *GetName() secReplicatorGob;
#endif

private:
	bool m_fActivated;
};

// MemMgr - for allocs that are mostly read-only.

typedef void *HMem;
typedef word HandleEntry;
typedef word HandleEntryRef;

struct HMemStruct
{
	word her;
	byte iheap;
};

struct MemHeader
{
	word w;
	word cbBlock;
};

#define kcHandlesPerTable 64
struct HandleTable
{
	int iheFree;
	HandleEntry ahe[kcHandlesPerTable];
};

struct Heap
{
	byte *pbHeap;
	word cbHeap;
	MemHeader *pmhdrFreeFirst;
	word cbLargestFreeBlock;
	word wf;
	word nRec;
	void *hMem;
};

#define kfHintWillLock 0x1

class MemMgr
{
public:
	MemMgr() secMemMgr;
	~MemMgr() secMemMgr;

	void Init(dword cbDynReserve, dword cbMaxNeeded, dword *pcbTotal) secMemMgr;
	void Exit() secMemMgr;

	HMem AllocHandle(word cb, word wfHints = 0) secMemMgr;
	void FreeHandle(HMem hmem) secMemMgr;
	void SetLocked(HMem hmem) secMemMgr;
	void ClearLocked(HMem hmem) secMemMgr;
	void *GetPtr(HMem hmem) secMemMgr;
	void WriteHandle(HMem hmem, word ib, void *pvSrc, word cb) secMemMgr;

	void *AllocPtr(word cb) secMemMgr;
	void FreePtr(void *pv) secMemMgr;
	void WritePtr(void *pvDst, word ib, void *pvSrc, word cb) secMemMgr;

	dword GetFreeSize(word *pcbLargestFree = NULL) secMemMgr;
	dword GetTotalSize(dword *pcbDyn, dword *pcbDb) secMemMgr;
	static void GetInitSize(dword cbDynReserve, dword *pcbDyn, dword *pcbStorage) secMemMgr;
	static void OnRebootFreeStorageDatabase(); // must remain in .text section

#ifdef INCL_VALIDATEHEAP
	void Validate() secMemMgr;
	void ValidateHandleTableFreeEntries() secMemMgr;
#endif

private:
	HandleTable *NewHandleTable() secMemMgr;
	bool AllocHandleEntry(word *pher) secMemMgr;
	void FreeHandleEntry(word her) secMemMgr;
	Heap *NewHeap(void *pvHeap, word cbHeap, bool fDbRam, word nRec, void *hMem) secMemMgr;
	void WriteHeader(Heap *pheap, MemHeader *pmhdrDst, MemHeader *pmhdrSrc, word cb = sizeof(MemHeader)) secMemMgr;
	HMem AllocBlock(int iheap, MemHeader *pmhdr, MemHeader *pmhdrFreeLast, word cbBlock, word wfHints) secMemMgr;
	MemHeader *Compact(Heap *pheap, word cbBlock, MemHeader **ppmhdrFreeLast, word wfHints) secMemMgr;
	MemHeader *SplitFreeBlock(Heap *pheap, MemHeader *pmhdr, MemHeader *pmhdrFreeLast, MemHeader *pmhdrNew, word wfHints) secMemMgr;
	word GetLargestFreeBlockSize(Heap *pheap) secMemMgr;

#ifdef INCL_VALIDATEHEAP
	void ValidateHandle(HMem hmem) secMemMgr;
#endif

	Heap *m_apheap[64];
	int m_cheap;
	HandleTable **m_aphtab;
	int m_chtab;
	int m_chtabAlloced;
	dword m_cbTotalFreeBlocks;

#ifdef PIL
	LocalID m_lid;
	DmOpenRef m_pdb;
#endif
};
extern MemMgr gmmgr;

// Cache Manager.
// 16 bytes for quick lookup

struct CacheEntry // ce
{
	CacheEntry *pceNext;
	CacheEntry *pcePrev;
	word wUniqueLock;
	word cbSize;
	HMem hmem;
};

#define kcCacheEntries 512
#define kwIndexMask 0x1ff
#define kwUniqueMask 0xfe00
#define kwLockMask 0x001ff
#define kwIncUnique 0x0200
#define kwIncLock 0x0001

class CacheMgr {
public:
	CacheMgr() secCacheMgr;
	~CacheMgr() secCacheMgr;
	bool Init() secCacheMgr;
	void Exit() secCacheMgr;

	void *GetPtr(CacheHandle hc) secCacheMgr;
	void *Lock(CacheHandle hc) secCacheMgr;
	void Unlock(CacheHandle hc) secCacheMgr;
	bool IsValid(CacheHandle hc) secCacheMgr;
	CacheHandle NewObject(void *pv, word cb, word wfHints = 0) secCacheMgr;
	void Write(CacheHandle hc, word ib, void *pvSrc, word cb) secCacheMgr;
	word GetSize(CacheHandle hc) secCacheMgr;
	bool MakeSpace(dword cb) secCacheMgr;
	dword GetTotalSize() secCacheMgr;
	dword GetLimit() secCacheMgr;
	void SetLimit(dword cbLimit) secCacheMgr;

private:
	CacheEntry *m_pceList;
	CacheEntry *m_pceFirst;
	CacheEntry *m_pceFree;
	dword m_cbTotalSize;
	dword m_cbLimit;

	inline CacheHandle MakeHandle(CacheEntry *pce)
	{
		return (CacheHandle)((pce - m_pceList) | (pce->wUniqueLock & kwUniqueMask));
	}

	inline CacheEntry *ValidateHandle(CacheHandle hc)
	{
		if (hc == 0)
			return NULL;
		word ice = (hc & kwIndexMask);
		if (ice >= kcCacheEntries)
			return NULL;
		CacheEntry *pce = &m_pceList[ice];
		if ((pce->wUniqueLock & kwUniqueMask) != (hc & kwUniqueMask))
			return NULL;
		return pce;
	}

	inline void Add(CacheEntry *pce)
	{
		if (m_pceFirst != NULL) {
			pce->pceNext = m_pceFirst;
			m_pceFirst->pcePrev->pceNext = pce;
			pce->pcePrev = m_pceFirst->pcePrev;
			m_pceFirst->pcePrev = pce;
		} else {
			pce->pceNext = pce;
			pce->pcePrev = pce;
		}
		m_pceFirst = pce;
	}

	inline void Remove(CacheEntry *pce)
	{
		pce->pcePrev->pceNext = pce->pceNext;
		pce->pceNext->pcePrev = pce->pcePrev;
		if (pce == m_pceFirst) {
			m_pceFirst = pce->pceNext;
			if (m_pceFirst == pce)
				m_pceFirst = NULL;
		}
	}

	inline void AddToFreeList(CacheEntry *pce)
	{
		if (m_pceFree != NULL) {
			pce->pceNext = m_pceFree;
			m_pceFree->pcePrev->pceNext = pce;
			pce->pcePrev = m_pceFree->pcePrev;
			m_pceFree->pcePrev = pce;
		} else {
			pce->pceNext = pce;
			pce->pcePrev = pce;
		}
		m_pceFree = pce;
	}

	inline void RemoveFromFreeList(CacheEntry *pce)
	{
		pce->pcePrev->pceNext = pce->pceNext;
		pce->pceNext->pcePrev = pce->pcePrev;
		if (pce == m_pceFree) {
			m_pceFree = pce->pceNext;
			if (m_pceFree == pce)
				m_pceFree = NULL;
		}
	}

	void Discard(CacheEntry *pce) secCacheMgr;
};
extern CacheMgr gcam;

// SoundMgr

#define kcChannelsMax 16

} // namespace wi
    
#include "sounddevice.h"

namespace wi {

struct PcmHeader // pcmh
{
	byte *pb;
	word cb;
};

struct SfxEntry // sfxe
{
	byte nSound;
	byte nChannel; // unused
	byte nPriority;
};
#define kcbSfxEntry 3

class SoundMgr // sndm
{
public:
	SoundMgr() secSoundMgr;
	~SoundMgr() secSoundMgr;

	bool Init() secSoundMgr;
	void Exit() secSoundMgr;
	void Enable(bool fEnable) secSoundMgr;
	bool IsEnabled() secSoundMgr;
	void PlaySfx(Sfx sfx) secSoundMgr;
	void WaitSilence() secSoundMgr;
	long FilterSleepTicks(long ct) secSoundMgr;
	void SetVolume(int nVolume) secSoundMgr;
	int GetVolume() secSoundMgr;
	bool SaveStateAndClear() secSoundMgr;
	void RestoreState() secSoundMgr;

private:
	int m_cChannels;
	byte m_anPriorityChannel[kcChannelsMax];
	FileMap *m_afmap;
	PcmHeader *m_apcmh;
	int m_cpcmh;
	SfxEntry *m_asfxe;
	int m_csfxe;
	FileMap m_fmapSfxEntries;
	SoundDevice *m_psndd;
	bool m_fStateSaved;
	bool m_fEnabledSav;
	int m_nVolumeSav;
};
extern SoundMgr gsndm;

// StringTable Class

class StringTable
{
public:
	StringTable() secStringTable;
	~StringTable() secStringTable;

	bool Init(char *pszFilename) secStringTable;
	bool GetString(int id, char *psz, int cb) secStringTable;

private:
	File *m_pfil;
};
extern StringTable *gpstrtbl;


// AlertControl

#define kcchAlertText 64
class AlertControl : public LabelControl, public Timer
{
public:
	AlertControl() secAlertControl;
	~AlertControl() secAlertControl;

	void AddText(const char *psz) secAlertControl;

	// Control methods

	virtual bool Init(Form *pfrm, IniReader *pini, FindProp *pfind) secAlertControl;

	// Timer methods

	virtual void OnTimer(long tCurrent) secAlertControl;

private:
	word m_wf;
};

// DRM stuff

struct Key // key
{
	byte ab[9];
};
extern Key gkey;
bool DrmValidate() secDrm;

//
// Preferences support
//

// The current version

#define knVersionPreferencesV01 1
#define knVersionPreferencesLatest knVersionPreferencesV01

// Keys

#define knPrefVersion "version"
#define kszPrefUsername "username"
#define kszPrefPassword "password"
#define kszPrefToken "token"
#define kfPrefAnonymous "anonymous"
#define knPrefYearLastRun "year_last_run" // demo time check
#define knPrefMonthLastRun "month_last_run"
#define knPrefDayLastRun "day_last_run"
#define kwfPrefPerfOptions "perf_options"
#define knPrefVolume "volume" // in percent (0 - 100)
#define kfPrefSoundMuted "sound_muted"
#define kwfPrefHandicap "handicap" // Difficulty-affecting flags
#define knPrefGameSpeed "game_speed"
#define kfPrefLassoSelection "lassos_election"
#define knPrefHueOffset "hue_offset" // HSL settings
#define knPrefSatMultiplier "sat_multiplier"
#define knPrefLumOffset "lum_offset"
#define knPrefCxModeBest "cx_mode_best" // Remember what mode / data is best. Check against in case config
#define knPrefCyModeBest "cy_mode_best"
#define knPrefDepthModeBest "depth_mode_best"
#define knPrefDepthDataBest "depth_data_best"
#define knPrefSizeDataBest "size_data_best"
#define knPrefDegreeOrientationBest "degree_orientation_best"

#define knPrefCxModeLast "cx_mode_last" // Remember what mode / data was chosen
#define knPrefCyModeLast "cy_mode_best"
#define knPrefDepthModeLast "depth_mode_last"
#define knPrefDepthDataLast "depth_data_last"
#define knPrefSizeDataLast "size_data_last"
#define knPrefDegreeOrientationLast "degree_orientation_last"

#define kszPrefKey "key" // DRM key
#define knPrefDemoRank "demo_rank"
// #define knPrefScale "scale"
#define knPrefScrollSpeed "scroll_speed"
// #define kfPrefIgnoreBluetoothWarning "ignore_bluetooth_warning"
#define kszPrefAskUrl "ask_url"
#define kszPrefDeviceId "did"
#define knPrefUpdateDisplay "update_display"

class Preferences
{
public:
    Preferences() secPreferences;
    ~Preferences() secPreferences;

    bool InitFromFile() secPreferences;
    bool InitFromDeafults() secPreferences;

    bool Save() secPreferences;
    const char *GetString(const char *key) secPreferences;
    int GetInteger(const char *key) secPreferences;
    float GetFloat(const char *key) secPreferences;
    bool GetBool(const char *key) secPreferences;

    void Set(const char *key, const char *psz) secPreferences;
    void Set(const char *key, int n) secPreferences;
    void Set(const char *key, float n) secPreferences;
    void Set(const char *key, bool f) secPreferences;

private:
    json::JsonMap *m_pmap;
    char *m_pszJson;
};
Preferences *PrefsFromFile() secPreferences;
Preferences *PrefsFromDefaults() secPreferences;

extern Preferences *gpprefs;

// Our in-game MessageBox

bool HtMessageBox(word wf, const char *pszTitle, const char *pszFormat, ...) secMisc;
bool HtMessageBox(word idf, word wf, const char *pszTitle, const char *pszBody) secMisc;
DialogForm *CreateHtMessageBox(word idf, word wf, const char *pszTitle, const char *pszBody) secMisc;

const word kfMbWhiteBorder = 0x0001;
const word kfMbClearDib = 0x0002;
const word kfMbKeepTimersEnabled = 0x0004;

// For Wc14Transport.dll to utilize external functionality

struct TransportHost
{
	void *(*New)(int size);
	void (*Delete)(void *pv);
	void (*strncpyz)(char *pszDst, const char *pszSrc, int cb);
	void (*Status)(const char *psz);
	bool (*HtMessageBox)(unsigned short wf, const char *pszTitle, const char *pszFormat, ...);
	TimerMgr *ptimm;
};

// Globals

extern byte grvlp[];
extern byte grvlpLarge[];
extern IniReader *gpiniForms;
extern IniReader *gpiniGame;
extern Font *gapfnt[];
extern Player *gpplrLocal;
extern byte *gmpiclriclrShadow;
extern UnitConsts *gapuntc[kutMax];
extern PlaceStructureForm *gpfrmPlace;
#ifdef DRAW_PATHS
extern bool gfDrawPaths;
#endif
#ifdef DRAW_LINES
extern bool gfDrawLines;
#endif
extern bool gfPlayingBack;
extern CommandQueue gcmdq;
extern bool gfOvermindEnabled;
extern PlayerMgr gplrm;
extern Player gplrDummy;
extern TBitmap *gaptbmScorches[4];
extern BuilderConsts gbldcHq;
extern byte *gpbScratch;
extern word gcbScratch;
extern bool gfLassoSelection;
extern int g_mpDirToDx[8];
extern int g_mpDirToDy[8];
extern int gcxTile;
extern int gcyTile;
extern int gcxyBorder;
extern char gszPlayerName[kcbPlayerName];
extern char gszUsername[kcbPlayerName];
extern char gszPassword[kcbPlayerName];
extern char gszToken[kcbTokenMax];
extern bool gfAnonymous;
extern bool gfDrawUpdateRects;
extern bool gfAutosave;
extern DibBitmap *gpbmClip;
extern FormMgr *gpfrmmSim;
extern FormMgr *gpfrmmInput;
extern MultiFormMgr *gpmfrmm;
extern SpriteManager *gpsprm;
#ifdef STATS_DISPLAY
extern long gcPathScoresCalced;
#endif
extern int gnHueOffset;
extern int gnSatMultiplier;
extern int gnLumOffset;
extern int gnDemoRank;
extern float gnScrollSpeed;
extern char gszAskURL[512];
extern char gszDeviceId[34];
extern int gtGameSpeed;
extern AnimationData *g_panidMoveTarget;
extern UpdateMap *gpupdSim;
extern Color *gaclrFixed;
extern Color gaclr24bpp[];
extern Color gaclr8bpp[];
extern Color gaclr4bpp[];
extern byte gmpDistFromDxy[10][10];
extern TRect *gptrcMapOpaque;
extern TRect gtrcMapOpaque;
extern Upgrade gaupg[kupgtMax];
extern MiniMapControl *gpmm;
#ifdef STRESS
extern bool gfStress;
extern long gtStressTimeout;
#endif
extern byte gmpdirdirOpposite[8];
extern int ganSquared[64];
extern bool gfClearFog;
extern int gnUnitCostMin;
extern int gnUnitCostMax;
extern int gnUnitCostMPMin;
extern int gnUnitCostMPMax;
extern fix gfxMobileUnitArmorStrengthMin;
extern fix gfxMobileUnitArmorStrengthMax;
extern fix gfxStructureArmorStrengthMin;
extern fix gfxStructureArmorStrengthMax;
extern fix gfxStructureDamageMin;
extern fix gfxStructureDamageMax;
extern fix gfxVehicleDamageMin;
extern fix gfxVehicleDamageMax;
extern fix gfxInfantryDamageMin;
extern fix gfxInfantryDamageMax;
extern TCoord gtcFiringRangeMin;
extern TCoord gtcFiringRangeMax;
extern WCoord gwcMoveDistPerUpdateMin;
extern WCoord gwcMoveDistPerUpdateMax;
extern int gnPowerSupplyMin;
extern int gnPowerSupplyMax;
extern int gnPowerDemandMin;
extern int gnPowerDemandMax;
extern int gaiclrSide[kcSides];
extern char *gszVersion;
extern char gszBuildDate[];
extern char gszBuildTime[];
extern bool gfDragSelecting;
extern WPoint s_wptSelect1, s_wptSelect2;
extern WRect gwrcSelection;
extern WPoint s_awptSelection[];
extern int s_cwptSelection;
extern word gwfPerfOptions;
extern word gwfHandicap;
extern int gcStructGobsHumanLimitSP;
extern int gcStructGobsComputerLimitSP;
extern int gcMuntGobsHumanLimitSP;
extern int gcMuntGobsComputerLimitSP;
extern int gcStructGobsComputerDeltaSP;
extern int gcStructGobsHumanDeltaSP;
extern int gcStructGobsLimitMP;
extern int gcMuntGobsLimitMP;
extern int gcSceneryGobsLimit;
extern int gcScorchGobsLimit;
extern int gcSupportGobsLimit;
extern bool gfGrayscale;
extern bool gfDemo;
extern Transport *gptra;
#ifdef MP_STRESS
extern bool gfMPStress;
extern int gnMPPos;
#endif
extern char *gpszDataDir;
extern bool gfIgnoreBluetoothWarning;
extern TexAtlasMgr *gptam;
extern int gcmsDisplayUpdate;

inline Color GetColor(int iclr) {
	return gaclrFixed[iclr];
}

inline Color GetSideColor(Side side) {
	return GetColor(gaiclrSide[side]);
}

// Helpers
inline UnitType UnitTypeFromPVoid(void *pv) {
    return *((UnitType *)&pv);
}

inline int IntFromPVoid(void *pv) {
    return *((int *)&pv);
}

// Out-of-band data for gameOverEvent (knGoLoadSavedGame case)
extern Stream *gpstmSavedGame;
extern int gimodeReinitialize;
extern bool gfLoadReinitializeSave;

char *_fgets(char *psz, int cch, File *pfil) secMisc;
void HtStrupr(char *psz) secMisc;

#ifdef DEBUG
int _GetRandom(char *pszFile, int nLine) secMisc;
#define GetRandom() _GetRandom(__FILE__, __LINE__)
#else
int _GetRandom() secMisc;
#define GetRandom() _GetRandom()
#endif
void SetRandomSeed(unsigned long nSeed) secMisc;
unsigned long GetRandomSeed() secMisc;
int GetAsyncRandom() secMisc;
void DrawBuildProgressIndicator(DibBitmap *pbm, Rect *prc, int nNumerator, int nDenominator) secGob;
void DrawSelectionIndicator(DibBitmap *pbm, Rect *prc, int nNumerator, int nDenominator) secGob;
void DrawHealthIndicator(DibBitmap *pbm, Rect *prc, int nNumerator, int nDenominator) secGob;
void DrawFullnessIndicator(DibBitmap *pbm, Rect *prc, int nPips, int nPipsMax) secMiner;
void DrawBorder(DibBitmap *pbm, Rect *prc, int nThickness, Color clr, UpdateMap *pupd = NULL) secDibBitmap;
void DrawBitmapBorder(DibBitmap *pbm, UpdateMap *pupd, Rect *prc, AnimationData *panid, int ifrm = 0, Side side = ksideNeutral) secDibBitmap;
int DrawFancyText(DibBitmap *pbm, Font *pfntDefault, char *psz, int x, int y, int cch = 0) secLabelControl;
int GetFancyTextExtent(Font *pfntDefault, char *psz, int cch = 0) secLabelControl;
Direction CalcDir(int dx, int dy) secGob;
Direction16 CalcDir16(int dx, int dy) secGob;
bool FormDragger(Form *pfrm, Event *pevt) secForm;
bool HostMultiplayerGame() secMultiplayer;
bool JoinOrHostMultiplayerGame(const PackId *ppackid) secMultiplayer;
Direction TurnToward(Direction dirTo, Direction dirFrom) secMisc;
Direction16 TurnToward16(Direction16 dirTo, Direction16 dirFrom) secMisc;
int CalcCreditsShare(Player *pplr) secStructures;
void DrawTileMap(byte **ppbMap, int ctx, int cty, byte *pbDst, int cbDstStride, int cxLeftTile, int cyTopTile, int cxRightTile, int cyBottomTile, int ctxInside, int ctyInside, int cxTile, int cyTile) secTileMap;
FormMgr *CreateFormMgr(DibBitmap *pbm) secFormMgr;
void ShadowHelper(DibBitmap *pbm, UpdateMap *pupd, Rect *prc) secForm;
void FillHelper(DibBitmap *pbm, UpdateMap *pupd, Rect *prc, Color clr) secForm;
void BltHelper(DibBitmap *pbm, TBitmap *ptbm, UpdateMap *pupd, int xDst, int yDst) secForm;
void RgbToHsl(byte bR, byte bG, byte bB, word *pnH, word *pnS, word *pnL) secMisc;
void HslToRgb(word nH, word nS, word nL, byte *pbR, byte *pbG, byte *pbB) secMisc;
UnitConsts *GetUnitConsts(GobType gt) secGob;
Sfx SfxFromCategory(SfxCategory sfxc) secMisc;
bool ParseNumber(char **ppsz, int *pn) secTrigger;
bool ParseLong(char **ppsz, long *pn) secTrigger;
bool ParseArea(char **ppsz, int *pn) secTrigger;
bool ParseUnitMask(char **ppsz, UnitMask *pum) secTrigger;
bool ParseUpgradeMask(char **ppsz, UpgradeMask *pupgm) secTrigger;
bool ParseString(char **ppsz, char *psz) secTrigger;
SideMask GetSideMaskFromCaSideMask(Side sideCur, word wfCaSideMask) secTrigger;
int GetPlayersListFromCaSideMask(Side sideCur, word wfMask, Player **applr) secTrigger;
void Ecom(int nCharFrom, int nCharTo, char *pszMessage, int nBackground, bool fMore) secEcom;
bool DoModalGameOptionsForm(bool fInGame) secGameOptionsForm;
bool ShowDownloadMissionPackForm(PackId *ppackid);
bool DownloadMissionPack(const PackId *ppackid, const char *pszTitle,
        bool fPlayButton);
bool DownloadMissionPackFromURL(const char *pszURL, PackId *ppackid,
        bool *play);
bool AskInstallMissionPack(const PackId *ppackid, const char *pszUITitle,
        const char *pszPackTitle);
bool IsTileFree(TCoord tx, TCoord ty, byte bf = kbfStructure, Gob **ppgob = NULL) secMisc;
void ShowAlert(int id) secAlertControl;
void ShowAlert(const char *psz) secAlertControl;
void SendAttackCommand(Gid gidReceiver, Gid gidTarget) secUnitGob;
word AggBitsFromAgg(int nAggressiveness) secUnitGob;
void FindNearestFreeTile(TCoord tx, TCoord ty, WPoint *pwpt, byte bf = kbfStructure | kbfMobileUnit) secMisc;
void GetPrerequisiteString(char *psz, UnitConsts *puntc) secUnitGob;
void BringInBounds(WCoord *pwx, WCoord *pwy) secMisc;
Direction DirectionFromLocations(TCoord txOld, TCoord tyOld, TCoord txNew, TCoord tyNew) secTerrainMap;
Direction16 Direction16FromLocations(TCoord txOld, TCoord tyOld, TCoord txNew, TCoord tyNew) secTerrainMap;
int RadiusFromUnitCount(int cUnits) secMisc;
void MoveUnitsToArea(MobileUnitGob **apmunt, int cpmunt, TRect *ptrc) secTrigger;
void AddPointToLassoSelection(WPoint wpt) secSimUIForm;
void ExpandVars(char *pszSrc, char *pszBuff, int cbBuff) secMisc;
void GetRankTitle(char *psz, int cb) secMisc;
void DoRegisterNowForm() secShell;
bool PickTransport(Transport **pptra) secMultiplayer;
void FormatButtons(Form *pfrm, word *aidc, int cidc, int idcRef, int idcRefNext) secGameOptionsForm;
bool DoInputPanelForm(char **ppszChars, int cRows, const char *pszLabel,
        const char *pszEdit, char *pszOut, int cbOut,
        bool (*pfnValidateInput)(const char *psz) = NULL) secInputPanelForm;
void DoInputPanelForm(Form *pfrm, word idcLabel, word idcEdit) secInputPanelForm;
void GetSpeedMultiplierString(char *psz, long tGameSpeed) secGameOptionsForm;
bool PtInPolygon(WPoint *awpt, int cwpt, WCoord wx, WCoord wy);
void AddPointToLassoSelection(WPoint wpt);
//const char *GetString(const json::JsonMap *map, const char *key);

#ifdef MP_DEBUG_SHAREDMEM
void MPInitSharedMemoryWindow(bool fServer);
void MPExitSharedMemoryWindow(bool fServer);
void MPUpdateState();
void MPValidateState();
Gob *MPGetGobPtr(Gid gid);
void MPCopyMem(void *pvTo, void *pvFrom, int cb);
void MPValidateMemory(void *pvRemote, void *pvLocal, int cb);
void MPValidateMember2(void *pvLocal, void *pvRemote, int cbThis, int cbOffsetMem, int cbMem);
#define MPValidateMember(c, m, pr) MPValidateMember2(this, pr, sizeof(c), OFFSETOF(c, m), sizeof(m))
#define MPValidateGobMember(c, m) MPValidateMember(c, m, MPGetGobPtr(GetId()))

struct SharedMemWindow
{
	bool fDetectSyncErrors;
	DWORD pidProcess;
	Gob *apgobMaster[kcpgobMax + 1];
};

extern SharedMemWindow gsmw;
extern HANDLE ghSharedMem;
extern SharedMemWindow *gpsmw;
#endif

// Misc runtime stuff

int strnicmp(const char *psz1, const char *psz2, int cch);
char *itoa (int val, char *buf, int radix);

#ifdef DRAW_PATHS
void LoadArrows() secGob;
void FreeArrows() secGob;
void DrawArrow(DibBitmap *pbm, int x, int y, Direction dir, Side side) secGob;
#endif

#ifdef BETA_TIMEOUT
bool CheckBetaTimeout() secMisc;
#endif

struct ClipInfo
{
	int cxClip;
	int cx;
	int cbRowDst;
	int cyClip;
};

#ifdef __CPU_68K
extern "C" void DrawTileMap816(byte **ppbMap, int ctx, int cty, byte *pbDst, int cbDstStride, int cxLeftTile, int cyTopTile, int cxRightTile, int cyBottomTile, int ctxInside, int ctyInside) secCode7;
extern "C" void DrawTileMap824(byte **ppbMap, int ctx, int cty, byte *pbDst, int cbDstStride, int cxLeftTile, int cyTopTile, int cxRightTile, int cyBottomTile, int ctxInside, int ctyInside) secCode9;
extern "C" void CopyToScreen4bpp(byte *pbSrc, byte *pbDst, byte *mp8to4, int c) secCode14;
extern "C" void FillShadow68K(byte *pbDst, int cbRowDst, int cx, int cy, byte *aclrMap) secCode14;
extern "C" void Fill68K(byte *pbRow, int cx, int cy, int cbStride, byte bFill) secCode11;
extern "C" void LeftToRightBlt68K(byte *pbSrc, int cxSrcStride, byte *pbDst, int cxDstStride, int cx, int cy) secCode10;
extern "C" void RightToLeftBlt68K(byte *pbSrc, int cxSrcStride, byte *pbDst, int cxDstStride, int cx, int cy) secCode10;
extern "C" dword DecodeAndMix8BitAdpcmChannelsBy4(MixerState *pmxst, bool fZeroSilence) secCode6;
extern "C" void Decode8BitAdpcmTable() secCode6;
extern "C" word Compile868K(byte *pb, ScanData *psd, bool fOdd) secCode8;
extern "C" void DrawDispatch68K(byte *pb, byte *pbSrc, byte *pbDst, int cbReturn, dword *mpscaiclrSide, byte *mpiclriclrShadow) secCode8;
extern "C" void DrawSelection68k(byte *pbDst, int cxDib, int cxSel, int cySel, int cxTab, int cyTab, byte clrSel, int cxHealth, byte clrHealth, byte *mpclrclrShadow, bool fBrackets) secCode14;
extern "C" void DebugBreak() secCode14;
extern "C" void CopyToScreen2bpp(byte *pbSrc, byte *pbDst, byte *mp8to2Low, byte *mp8to2High) secCode14;
extern "C" void CopyToScreen4bpp(byte *pbSrc, byte *pbDst, byte *mp8to4, int c) secCode14;
extern "C" void CopyToScreen8bpp(byte *pbSrc, byte *pbDst, int cx, int cy) secCode14;
extern "C" void FastMemSet(void *pv, long cb, byte b) secCode14;
extern "C" void UpdateScreen824(bool *pfMap, int ctx, int cty, byte *pbSrc, byte *pbDst, int cbStride, int cxLeftTile, int cyTopTile, int cxRightTile, int cyBottomTile, int ctxInside, int ctyInside) secCode10;
extern "C" void UpdateScreen816(bool *pfMap, int ctx, int cty, byte *pbSrc, byte *pbDst, int cbStride, int cxLeftTile, int cyTopTile, int cxRightTile, int cyBottomTile, int ctxInside, int ctyInside) secCode10;
extern "C" void UpdateScreen416(byte *mp8to4, bool *pfMap, int ctx, int cty, byte *pbSrc, byte *pbDst, int cbStride, int cxLeftTile, int cyTopTile, int cxRightTile, int cyBottomTile, int ctxInside, int ctyInside) secCode11;
#endif

extern "C" word DecompressChunk(byte **ppbCompressed, byte *pbDecompressed, byte *pbCacheEnd, word cbMax) secPackFile;
extern "C" void DecompressToCache(byte *pbCompressed, CacheHandle hc, CompressionHeader *pcoh) secPackFile;

// Thunks

void FillShadowThunk(byte *pbDst, int cbRowDst, int cx, int cy, byte *aclrMap) secThunks;
void FillThunk(byte *pbDst, int cx, int cy, int cbStride, byte bFill) secThunks;
void LeftToRightBltThunk(byte *pbSrc, int cxSrcStride, byte *pbDst, int cxDstStride, int cx, int cy) secThunks;
void RightToLeftBltThunk(byte *pbSrc, int cxSrcStride, byte *pbDst, int cxDstStride, int cx, int cy) secThunks;
void DrawTileMapThunk(byte **ppbMap, int ctx, int cty, byte *pbDst, int cbDstStride, int cxLeftTile, int cyTopTile, int cxRightTile, int cyBottomTile, int ctxInside, int ctyInside, int cxTile, int cyTile) secThunks;
word Compile8Thunk(byte *pb, ScanData *psd, bool fOdd) secThunks;
void DrawDispatchThunk(byte *pb, byte *pbSrc, byte *pbDst, int cbReturn, dword *mpscaiclrSide, byte *mpiclriclrShadow) secThunks;

int YClipToScan(int yClip, int cx, byte*& pop, byte*& pbSrc, word*& psc);
int DrawScan(byte *pbDst, int cx, byte*& pbSrc, byte*& pop, word*& psc,
        dword *mpscaiclr, byte *mpiclriclrShadow);

// ARM stuff

void InitArmCode() secArmThunks;
void ExitArmCode() secArmThunks;
void DrawTileMapArm(byte **ppbMap, int ctx, int cty, byte *pbDst, int cbDstStride, int cxLeftTile, int cyTopTile, int cxRightTile, int cyBottomTile, int ctxInside, int ctyInside, int cxTile, int cyTile) secArmThunks;
void LeftToRightBltArm(byte *pbSrc, int cxSrcStride, byte *pbDst, int cxDstStride, int cx, int cy) secArmThunks;
void RightToLeftBltArm(byte *pbSrc, int cxSrcStride, byte *pbDst, int cxDstStride, int cx, int cy) secArmThunks;
void memsetArm(byte *pbDst, byte b, dword cb) secArmThunks;
void FillArm(byte *pbDst, int cx, int cy, int cbStride, byte bFill) secArmThunks;
void FillShadowArm(byte *pbDst, int cbRowDst, int cx, int cy, byte *aclrMap) secArmThunks;
word Compile8Arm(byte *pb, ScanData *psd, bool fOdd) secArmThunks;
void DrawDispatchArm(byte *pb, byte *pbSrc, byte *pbDst, int cbReturn, dword *mpscaiclrSide, byte *mpiclriclrShadow) secArmThunks;
extern bool gfArmPresent;

// Host stuff

bool HostInit();
void HostExit();
void HostOpenUrl(const char *pszUrl);
bool HostGetEvent(Event *pevt, long ctWait = -1) secHost;
void HostServiceGetEvent() secHost;
void HostOutputDebugString(char *pszFormat, ...) secHost;
long HostGetTickCount() secHost;
long HostGetMillisecondCount() secHost;
long HostRunSpeedTests(DibBitmap *pbmSrc) secHost;
dword HostGetCurrentKeyState(dword keyBit) secHost;
void HostMessageBox(TCHAR *pszFormat, ...) secHost;
Display *HostCreateDisplay() secDisplay;
bool HostIsPenDown() secHost;
const char *HostGetMainDataDir() secHost;
const char *HostGetPrefsFilename() secHost;
void HostSuspendModalLoop(DibBitmap *pbm) secHost;
void HostNotEnoughMemory(bool fStorage, dword cbFree, dword cbNeed) secHost;
bool HostGetOwnerName(char *pszBuff, int cb, bool fShowError) secHost;
bool HostEnumAddonFiles(Enum *penm, char *pszAddonDir, int cbDir,
        char *pszAddon, int cb) secHost;
bool HostIsPOSE() secHost;
void HostGetUserName(char *pszBuff, int cbMax) secHost;
SoundDevice *HostOpenSoundDevice() secSoundDevice;
bool HostSoundServiceProc() secSoundDevice;
void HostSleep(dword ct) secHost;
void HostSetGameThread(base::Thread *thread);
base::Thread& HostGetGameThread();
base::Thread *HostGetGameThreadPointer();
void HostAppStop();

const int knKeyboardAskDefault = 0;
const int knKeyboardAskURL = 1;

void HostInitiateAsk(const char *title, int max, const char *def,
        int keyboard = knKeyboardAskDefault, bool secure = false);
void HostGetAskString(char *psz, int cb);

class IChatController;
IChatController *HostGetChatController();

void HostInitiateWebView(const char *title, const char *url);
const char *HostGenerateDeviceId();
const char *HostGetPlatformString();

// Date

struct Date
{
	int nYear;
	int nMonth;
	int nDay;
};
int CompareDates(Date *pdate1, Date *pdate2) secMisc;

void HostGetCurrentDate(Date *pdate) secHost;

// Silk rects

#define kircSilkGraffiti 0
#define kircSilkApps 1
#define kircSilkMenu 2
#define kircSilkCalc 3
#define kircSilkFind 4

void HostGetSilkRect(int irc, Rect *prc) secHost;

// Host file IO

const word kfOfRead = 0x0001;		// same as "rb"
const word kfOfWrite = 0x0002;		// same as "wb"

#define kfSeekSet 0
#define kfSeekCur 1
#define kfSeekEnd 2

FileHandle HostOpenFile(const char *pszFilename, word wf) secHost;
void HostCloseFile(FileHandle hf) secHost;
dword HostWriteFile(void *pv, dword c, dword cb, FileHandle hf) secHost;
dword HostReadFile(void *pv, dword c, dword cb, FileHandle hf) secHost;
dword HostSeekFile(FileHandle hf, int off, int nOrigin) secHost;
dword HostTellFile(FileHandle hf) secHost;

// Save game

class Stream
{
public:
	virtual ~Stream() {}
	virtual void Close() = 0;
	virtual dword Read(void *pv, dword cb) = 0;
	virtual dword Write(void *pv, dword cb) = 0;
	virtual bool IsSuccess() = 0;

	// Helpers

	void ReadString(char *psz, int cb) secStream;
	void ReadBytesRLE(byte *pb, int cb) secStream;
	inline byte ReadByte()
	{
		byte b = 0;
		Read(&b, sizeof(b));
		return b;
	}
	inline word ReadWord()
	{
		word w = 0;
		Read(&w, sizeof(w));
		return w;
	}
	inline dword ReadDword()
	{
		dword dw = 0;
		Read(&dw, sizeof(dw));
		return dw;
	}

	void WriteString(char *psz) secStream;
	void WriteBytesRLE(byte *pb, int cb) secStream;
	inline void WriteByte(byte b)
	{
		Write(&b, sizeof(b));
	}
	inline void WriteWord(word w)
	{
		Write(&w, sizeof(w));
	}
	inline void WriteDword(dword dw)
	{
		Write(&dw, sizeof(dw));
	}

private:
	byte *FindRLERepeat(byte *pbStart, byte *pbMax, int cbMin) secStream;
	void WriteRLEChunk(byte *pb, int cb, bool fRepeat) secStream;
};

const int knGameReinitializeSave = -3;
const int knGameAutosave = 10;

#define kszTempName "htsavetemp"
int HostGetSaveGameCount() secHost;
bool HostGetSaveGameName(int nGame, char *psz, int cb, Date *pdate, int *pnHours24, int *pnMinutes, int *pnSeconds) secHost;
Stream *HostNewSaveGameStream(int nGame, char *pszName) secHost;
Stream *HostOpenSaveGameStream(int nGame, bool fDelete = false) secHost;
Stream *PickLoadGameStream() secLoadSave;
bool PickSaveGameStream(Stream **ppstm) secLoadSave;
void DeleteStaleSaveGames() secLoadSave;
bool CheckSaveGameVersion(char *pszVersion, byte bPlatform) secLoadSave;
bool HostDeleteSaveGame(char *psz, int nGame) secLoadSave;

// Debug stuff

#if defined(MP_DEBUG) && defined(INCL_TRACE)
#define MpTrace ::wi::HostOutputDebugString
//#define MpTrace(fmt, args...) LOG() << base::Log::Format(fmt, ## args)
#else
#define MpTrace 1 ? (void)0 : ::wi::HostOutputDebugString
#endif

#ifdef INCL_TRACE
#define Trace ::wi::HostOutputDebugString
//#define Trace(fmt, args...) LOG() << base::Log::Format(fmt, ## args)
#else
#define Trace 1 ? (void)0 : ::wi::HostOutputDebugString
#endif

#ifdef STATUS_LINE
void Status(const char *psz) secMisc;
#else
#define Status(psz)
#endif

#ifdef DEBUG_HELPERS
void InitDebugHelpers();
void ExitDebugHelpers();
#endif

#if 0 
// Don't need this now that we discovered CE's COREDLL takes care of this.
// However, if we ever build a Windows desktop version of Wc14Transport.dll
// we will want to do this.

// Transport DLLs must call back to the WI .exe for memory management
// This allows us to continue our practice of having the Transport and
// its Connections perform 'new' operations and leave it to the rest
// of the game to 'delete' them.

#if defined(CE) && defined(DLL)
extern TransportHost *gptrah;

inline void* operator new(size_t size)
{
	if (size == 0) 
		size = 1;
	return gptrah->New(size);
}

inline void operator delete(void* ptr)
{
	if (ptr != NULL) 
		gptrah->Delete(ptr);
}
#endif
#endif

} // namespace wi

#endif // __HT_H__
