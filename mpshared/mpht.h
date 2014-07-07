#ifndef __MPHT_H__
#define __MPHT_H__

#include "inc/basictypes.h"
#include "mpshared/side.h"
#include "base/misc.h"

namespace wi {

const long kcUpdatesBlockInitial = 1;

const dword kdwClientID = 0x47414d45;

#define kdwTypeAddon 'ADD2'
#define kszTypeAddon "ADD2"

#define knVersionLevelSupported 1

// NOTE: Gob ids (gid) and StateMachine ids (smid) are interchangeable

typedef word Gid;	// gid
typedef word StateMachineId; // smid
typedef word Side; // side
typedef short UnitType;	// ut
typedef unsigned long UnitMask;	// um
typedef signed short TCoord;
typedef signed short WCoord;	
typedef word Pid; // pid

const int kcPlayersMax = 4 + 1;	// + 1 computer controlled neutral player

const Pid kpidNeutral = 0xffff;

#define knLagNone 0
#define knLagGrace 1
#define knLagGuilty 2
#define knLagKill 3

const int kcupdAllowedLag = 7; // TUNE:

const int kcbLevelTitle = 40;
#define kcbFilename 29
const int kcbPlayerName = 32;
const int kcbRoomname = 32;
const int kcbPassword = 32;
const int kcbUsernameMax = 64;
const int kcbTokenMax = 256;
const int kcbDidMax = 34;
const int kcbChatMax = 1024;
const int kcbShowMessageMax = 512;

// Side masks

typedef word SideMask; // sidm

const SideMask ksidmNeutral = 1 << ksideNeutral;
const SideMask ksidmSide1 = 1 << kside1;
const SideMask ksidmSide2 = 1 << kside2;
const SideMask ksidmSide3 = 1 << kside3;
const SideMask ksidmSide4 = 1 << kside4;
const SideMask ksidmAll = ksidmNeutral | ksidmSide1 | ksidmSide2 | ksidmSide3 | ksidmSide4;

#define GetSideMask(side) (1 << side)

const word kfPlrInUse = 0x0001;
const word kfPlrReady = 0x0002;
const word kfPlrComputer = 0x0004;
const word kfPlrUnfulfilled = 0x0008;
const word kfPlrStructureAttacked = 0x0010;
const word kfPlrComputerOvermind = 0x0020;
const word kfPlrAutoRepair = 0x0040;
const word kfPlrDisconnectBroadcasted = 0x0080;
const word kfPlrObserver = 0x0100;
const word kfPlrSummaryShown = 0x0200;
const word kfPlrCreator = 0x0400;
const word kfPlrDisconnectScheduled = 0x0800;
const word kfPlrWinner = 0x1000;
const word kfPlrLoser = 0x2000;
const word kfPlrRemovedAtGameStart = 0x4000;
const word kfPlrHumanJoined = 0x8000;

// Size

struct Size // siz
{
	int cx;
	int cy;
};

struct WPoint // pt
{
	WCoord wx, wy;
};

// Network address

struct NetAddress // nad
{
	byte ab[16];
};

// Simulation version, use in multiplayer game params

#define knVersionSimulation 2

#define PACKID_MAIN 0xffffffff

struct PackId {
    dword id;
    byte hash[16];
};

struct GameParams // rams
{
    PackId packid; // 20
    dword dwVersionSimulation; // 4
    long tGameSpeed; // 4
    char szLvlFilename[kcbFilename]; // 29
    byte filler[3]; // 3
}; // 60

struct PlayerName {
    char szPlayerName[kcbPlayerName];
};

// Message is sent multiplayer and dispatched as is (without going through
// reformatting). This means: fixed sized data types, "platform independent"
// packing (meaning data types must be aligned on natural boundaries, like
// dword on a 4 byte boundary etc). Message structure arrays are memcpy'ed to
// produce NetMessages. This gives the additional requirement of packed Message
// structures beginning on 4 byte boundaries. To achieve this ensure that all
// union'ed structs are 4 bytes length multiples

struct Message // msg
{
	word mid;
	// UNDONE: smidSender isn't used except by DEBUG_HELPERS
	StateMachineId smidSender;
	StateMachineId smidReceiver;
	word wDummy; // for long alignment
	long tDelivery;

	// MessageId-specific arguments

	union {
		struct {
			Gid gidAssailant;
			Side sideAssailant;
			// UNDONE: maybe a damage type (e.g., small-arms, area weapon)
			short nDamage;
			word wDummy;
		} Hit;

		struct {
			word sfx;
			word wDummy;
		} PlaySfx;

		struct {
			Gid gidEnemy;
			word wDummy;
		} EnemyNearby;

		struct {
			Gid gidTarget;
			WPoint wptTarget;
			WPoint wptTargetCenter;
			TCoord tcTargetRadius;
			WCoord wcMoveDistPerUpdate;
			word wDummy;
		} MoveCommand;

		// NOTE: both the gidTarget and wptTarget must be filled out so the
		// attacker can continue on to the target's location even if it no
		// longer exists.

		struct {
			Gid gidTarget;
			WPoint wptTarget;
			WPoint wptTargetCenter;
			TCoord tcTargetRadius;
			WCoord wcMoveDistPerUpdate;
			word wDummy;
		} AttackCommand;

		struct {
			word wfUpgrade;
			word wDummy;
		} UpgradeCommand;

		struct {
			word ut;
			WPoint wpt;
			word wDummy;
		} BuildOtherCommand;

		struct {
			UnitType ut;
			word wDummy;
		} AbortBuildOtherCommand;

		struct {
			Gid gidTarget;
			word wDummy;
		} DeliverCommand;

		struct {
			Gid gidTarget;
			WPoint wptTarget;
			word wDummy;
		} MineCommand;

		struct {
			word nArea;
			word wDummy;
		} GuardAreaCommand;

		struct {
			UnitMask um;
		} HuntEnemiesCommand;

        struct {
            Pid pid;
            word nReason;
        } PlayerDisconnect;
	};
};

// NOTE: be sure to update the corresponding string table in StateMachine.cpp

enum MessageId { // mid
	kmidReservedNull,
	kmidReservedEnter,
	kmidReservedExit,
	kmidReservedUpdate,
	kmidHit,
	kmidNearbyAllyHit,
	kmidDelete,
	kmidEnemyNearby,
	kmidPowerLowHigh,
	kmidMoveWaitingNearby,
	kmidBeingUpgraded,
	kmidUpgradeComplete,
	kmidMoveCommand,
	kmidAttackCommand,
	kmidAnimationComplete,
	kmidBuildOtherCommand,
	kmidAbortBuildOtherCommand,
	kmidBuildComplete,
	kmidFireComplete,
	kmidSpawnSmoke,
	kmidSelfDestructCommand,
	kmidRepairCommand,
	kmidUpgradeCommand,
	kmidAbortUpgradeCommand,
	kmidTransformCommand,
	kmidDeliverCommand,
	kmidGalaxiteDelivery,
	kmidMineCommand,
	kmidMoveAction,
	kmidAttackAction,
	kmidGuardAction,
	kmidGuardVicinityAction,
	kmidGuardAreaAction,
	kmidHuntEnemiesAction,
	kmidFire,
	kmidHeal,
	kmidPlaySfx,
    kmidPlayerDisconnect,
};

STARTLABEL(MessageNames)
	LABEL(kmidReservedNull)
	LABEL(kmidReservedEnter)
	LABEL(kmidReservedExit)
	LABEL(kmidReservedUpdate)
	LABEL(kmidHit)
	LABEL(kmidNearbyAllyHit)
	LABEL(kmidDelete)
	LABEL(kmidEnemyNearby)
	LABEL(kmidPowerLowHigh)
	LABEL(kmidMoveWaitingNearby)
	LABEL(kmidBeingUpgraded)
	LABEL(kmidUpgradeComplete)
	LABEL(kmidMoveCommand)
	LABEL(kmidAttackCommand)
	LABEL(kmidAnimationComplete)
	LABEL(kmidBuildOtherCommand)
	LABEL(kmidAbortBuildOtherCommand)
	LABEL(kmidBuildComplete)
	LABEL(kmidFireComplete)
	LABEL(kmidSpawnSmoke)
	LABEL(kmidSelfDestructCommand)
	LABEL(kmidRepairCommand)
	LABEL(kmidUpgradeCommand)
	LABEL(kmidAbortUpgradeCommand)
	LABEL(kmidTransformCommand)
	LABEL(kmidDeliverCommand)
	LABEL(kmidGalaxiteDelivery)
	LABEL(kmidMineCommand)
	LABEL(kmidMoveAction)
	LABEL(kmidAttackAction)
	LABEL(kmidGuardAction)
	LABEL(kmidGuardVicinityAction)
	LABEL(kmidGuardAreaAction)
	LABEL(kmidHuntEnemiesAction)
	LABEL(kmidFire)
	LABEL(kmidHeal)
	LABEL(kmidPlaySfx)
ENDLABEL(MessageNames)

struct UpdateResult // ur
{
	long cUpdatesBlock;
    dword hash;
    long cmsLatency;
};

} // namespace wi

#endif // __MPHT_H__
