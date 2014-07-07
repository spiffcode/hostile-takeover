#ifndef __NETMESSAGE_H__
#define __NETMESSAGE_H__

#include "inc/basictypes.h"
#include "inc/rip.h"
#include "mpshared/mpht.h"
#include "mpshared/misc.h"
#include "mpshared/side.h"
#include "base/format.h"
#include "base/log.h"
#include "base/misc.h"
#include <stdio.h>
#include <string.h>

//---------------------------------------------------------------------------
// Network and multiplayer stuff

// NOTE: be sure to update PszFromNetMessage in Comm.cpp if you change/add/remove NetMessageIds

namespace wi {

class NetMessage;
#ifdef DEBUG_LOGGING
const char *PszFromNetMessage(NetMessage *pnm);
#endif

#ifdef RELEASE_LOGGING
void LogGameParams(const GameParams& params);
const char *GameParamsToString(const GameParams& params);
#endif
   
const int kcbGameName = 80;

typedef word NetMessageId; // nmid

const NetMessageId knmidScCantAcceptMoreConnections = 1;
const NetMessageId knmidScServerInfo = 2;
const NetMessageId knmidCsClientCommands = 3;
const NetMessageId knmidScUpdate = 4;
const NetMessageId knmidScGameParams = 5;
const NetMessageId knmidScBeginGame = 6;
const NetMessageId knmidCsClientReady = 7;
const NetMessageId knmidCsPlayerJoin = 8;
const NetMessageId knmidScPlayersUpdate = 9;
const NetMessageId knmidCsUpdateResult = 10;
//const NetMessageId knmidGameHostFound = 11;
const NetMessageId knmidCsConnect = 12;
const NetMessageId knmidScLagNotify = 13;
const NetMessageId knmidScPlayerDisconnect = 14;
const NetMessageId knmidCsPlayerResign = 15;
const NetMessageId knmidCsRequestBeginGame = 16;
const NetMessageId knmidScBeginGameFail = 17;
const NetMessageId knmidCsKillLaggingPlayer = 18;
const NetMessageId knmidScSyncError = 19;
const NetMessageId knmidCsLagAcknowledge = 20;
const NetMessageId knmidCsWinStats = 21;
const NetMessageId knmidScCheckWin = 22;
const NetMessageId knmidCsChallengeWin = 23;
const NetMessageId knmidMax = 24;

class NetMessage // nm
{
public:
	NetMessage() {}
	NetMessage(NetMessageId nmid);
	
	word cb;	// size of whole message, including these fields
	NetMessageId nmid;

	// This (and the pad below) are DEBUG only information.
	// We want to play DEBUG vs. Release so it's not conditional
	// OPT: ditch it to shorten each message by 4 bytes
	byte nSeq;
	byte abPad[3];
	
	// OPT: Could move this out of the message to shorten each by 4 bytes
	NetMessage *pnmNext;
	// After this is dword aligned
};

inline NetMessage::NetMessage(NetMessageId nmid)
{
	this->nmid = nmid;
	this->cb = sizeof(NetMessage);
	this->pnmNext = NULL;
	CompileAssert((sizeof(this) % 4) == 0);
}

class ServerInfoNetMessage : public NetMessage {
public:
	ServerInfoNetMessage();
	ServerInfoNetMessage(char *pszGameName);
	char m_szGameName[kcbGameName];

#ifdef LOGGING
	void ToString(char *psz, int cb) {
		snprintf(psz, cb, "ScServerInfo(gameName: \"%s\")", m_szGameName);
	}
#endif
};

inline ServerInfoNetMessage::ServerInfoNetMessage()
{
	nmid = knmidScServerInfo;
	cb = sizeof(ServerInfoNetMessage);
	CompileAssert((sizeof(this) % 4) == 0);
}

inline ServerInfoNetMessage::ServerInfoNetMessage(char *pszGameName)
{
	nmid = knmidScServerInfo;
	cb = sizeof(ServerInfoNetMessage);
	strncpyz(m_szGameName, pszGameName, sizeof(m_szGameName));
	CompileAssert((sizeof(this) % 4) == 0);
}

class ConnectNetMessage : public NetMessage {
public:
	ConnectNetMessage();

#ifdef LOGGING
	void ToString(char *psz, int cb) {
        strncpyz(psz, "CsConnect", cb);
	}
#endif
};

inline ConnectNetMessage::ConnectNetMessage()
{
	nmid = knmidCsConnect;
	cb = sizeof(ConnectNetMessage);
	CompileAssert((sizeof(this) % 4) == 0);
}

class BeginGameNetMessage : public NetMessage {
public:
	BeginGameNetMessage();
	unsigned long ulRandomSeed;

#ifdef LOGGING
	void ToString(char *psz, int cb) {
		snprintf(psz, cb, "ScBeginGame(seed: %ld)", ulRandomSeed);
	}
#endif
};

inline BeginGameNetMessage::BeginGameNetMessage()
{
	nmid = knmidScBeginGame;
	cb = sizeof(BeginGameNetMessage);
	CompileAssert((sizeof(this) % 4) == 0);
}

class ClientCommandsNetMessage : public NetMessage {
public:
	ClientCommandsNetMessage();
	word cmsgCommands;
	word wDummy; // for long aligning the Messages on 68K
	Message amsgCommands[1];

#ifdef LOGGING
	void ToString(char *psz, int cb) {
		snprintf(psz, cb, "CsClientCommands(cmsg: %d)", cmsgCommands);
		for (int i = 0; i < cmsgCommands; i++) {
            int cch = strlen(psz);
            if ((cb - (cch + 1)) <= 0)
                break;
            snprintf(&psz[cch], cb - cch, ", %d", amsgCommands[i].mid);
        }
	}
#endif
};

inline ClientCommandsNetMessage::ClientCommandsNetMessage()
{
	Assert((sizeof(this) % 4) == 0);
}

class UpdateNetMessage : public NetMessage {
public:
	UpdateNetMessage();
	long cUpdatesBlock;
    long cUpdatesSync;
	word cmsgCommands;
	word wDummy; // for long aligning the Messages on 68K
	Message amsgCommands[1];

#ifdef LOGGING
	void ToString(char *psz, int cb) {
		snprintf(psz, cb, "ScUpdate(cmsg: %d, cUpdatesBlock: %ld, cUpdatesSync: %ld)",
                cmsgCommands, cUpdatesBlock, cUpdatesSync);
	}
#endif
};

inline UpdateNetMessage::UpdateNetMessage()
{
	Assert((sizeof(this) % 4) == 0);
}

class GameParamsNetMessage : public NetMessage {
public:
	GameParamsNetMessage();
	GameParams rams;

#ifdef LOGGING
	void ToString(char *psz, int cb) {
        strncpyz(psz, GameParamsToString(rams), cb);
    }
#endif
};

inline GameParamsNetMessage::GameParamsNetMessage()
{
	nmid = knmidScGameParams;
	cb = sizeof(GameParamsNetMessage);
	CompileAssert((sizeof(this) % 4) == 0);
}

class PlayerJoinNetMessage : public NetMessage {
public:
	PlayerJoinNetMessage();
	char szPlayerName[kcbPlayerName];

#ifdef LOGGING
	void ToString(char *psz, int cb) {
		snprintf(psz, cb, "CsPlayerJoin(%s)", szPlayerName);
	}
#endif
};

inline PlayerJoinNetMessage::PlayerJoinNetMessage()
{
	nmid = knmidCsPlayerJoin;
	cb = sizeof(PlayerJoinNetMessage);
	CompileAssert((sizeof(this) % 4) == 0);
}

const byte kfPlrrReady = 0x01;
const byte kfPlrrLocal = 0x02;
const byte kfPlrrComputer = 0x04;
const byte kfPlrrComputerOvermind = 0x08;
const byte kfPlrrUnfulfilled = 0x10;
const byte kfPlrrCreator = 0x20;

struct PlayerRecord // plrr
{
	Pid pid; // 4
	Side side; // 2
	word wf; // 2
	char szName[kcbPlayerName]; // 32
};

class PlayersUpdateNetMessage : public NetMessage {
public:
	PlayersUpdateNetMessage();
	word cplrr;
	word wDummy; // for long alignment
	PlayerRecord aplrr[1];

#ifdef LOGGING
	void ToString(char *psz, int cb) {
        int cHuman = 0;
        for (int i = 0; i < cplrr; i++) {
            if ((aplrr[i].wf & (kfPlrrComputer | kfPlrrUnfulfilled)) == 0) {
                cHuman++;
            }
        }
		snprintf(psz, cb, "ScPlayersUpdate(cplrr human: %d)", cHuman);
	}
#endif
};

inline PlayersUpdateNetMessage::PlayersUpdateNetMessage()
{
	CompileAssert((sizeof(this) % 4) == 0);
}

class UpdateResultNetMessage : public NetMessage { // urnm
public:
	UpdateResultNetMessage();
	UpdateResult ur;

#ifdef LOGGING
	void ToString(char *psz, int cb) {
		snprintf(psz, cb,
            "CsUpdateResult(cUpdatesBlock: %ld, hash %08lx, cmsLatency: %ld)",
                ur.cUpdatesBlock, ur.hash, ur.cmsLatency);
	}
#endif
};

inline UpdateResultNetMessage::UpdateResultNetMessage()
{
	nmid = knmidCsUpdateResult;
	cb = sizeof(UpdateResultNetMessage);
	CompileAssert((sizeof(this) % 4) == 0);
}

struct GameHost { // gh
	GameHost *pghNext;
	NetAddress nad;
	unsigned long msLastContact;
	unsigned long id;
	char szGameName[kcbGameName];
};

class LagNotifyNetMessage : public NetMessage { // lnnm
public:
	LagNotifyNetMessage();
	Pid pidLagging;
	word cSeconds;

#ifdef LOGGING
	void ToString(char *psz, int cb) {
		snprintf(psz, cb, "ScLagNotify(pidLagging: %d)", pidLagging);
	}
#endif
};

inline LagNotifyNetMessage::LagNotifyNetMessage()
{
	nmid = knmidScLagNotify;
	cb = sizeof(LagNotifyNetMessage);
	CompileAssert((sizeof(this) % 4) == 0);
}

class LagAcknowledgeNetMessage : public NetMessage { // lanm
public:
	LagAcknowledgeNetMessage();
	Pid pidLagging;
	word cSeconds;

#ifdef LOGGING
	void ToString(char *psz, int cb) {
		snprintf(psz, cb, "ScLagAcknowledge(pidLagging: %d)", pidLagging);
	}
#endif
};

inline LagAcknowledgeNetMessage::LagAcknowledgeNetMessage()
{
	nmid = knmidCsLagAcknowledge;
	cb = sizeof(LagAcknowledgeNetMessage);
	CompileAssert((sizeof(this) % 4) == 0);
}

#define kfwsReceivedStats 0x0001
#define kfwsWinner 0x0002
#define kfwsLoser 0x0004
#define kfwsWinChallenger 0x0008
#define kfwsComputer 0x0010
#define kfwsHuman 0x0020
#define kfwsAnonymous 0x0040
#define kfwsLocked 0x0080
#define kfwsRemovedAtGameStart 0x0100
#define kfwsMask 0x1ff

struct WinStats {
    word sidm;
    word sidmAllies;
    word cEnemyMobileUnitsKilled;
    word cEnemyStructuresKilled;
    word cMobileUnitsLost;
    word cStructuresLost;
    word ff;
    word acut[kutMax];
    word acutBuilt[kutMax];
    word dummy; // to ensure alignment on all platforms
    dword cCreditsAcquired;
    dword cCreditsConsumed;
};

class WinStatsNetMessage : public NetMessage { // wnm
public:
    WinStatsNetMessage();

    Pid pid;
    word filler;
    byte hash[16];
    WinStats ws; 

#ifdef LOGGING
	void ToString(char *psz, int cb) {
		snprintf(psz, cb, "WinStatsNetMessage("
                "sidm: %08x, sidmAllies: %08x, "
                "cCreditsAcquired: %ld, cCreditsConsumed: %ld, "
                "cEnemyMobileUnitsKilled: %d, cEnemyStructuresKilled: %d, "
                "cMobileUnitsLost: %d, cStructuresLost: %d, ff: %08x)",
                ws.sidm, ws.sidmAllies,
                ws.cCreditsAcquired, ws.cCreditsConsumed,
                ws.cEnemyMobileUnitsKilled, ws.cEnemyStructuresKilled,
                ws.cMobileUnitsLost, ws.cStructuresLost, ws.ff);
    }
#endif
};

inline WinStatsNetMessage::WinStatsNetMessage()
{
    memset(this, 0, sizeof(*this));
	nmid = knmidCsWinStats;
	cb = sizeof(WinStatsNetMessage);
	CompileAssert((sizeof(this) % 4) == 0);
}

class CheckWinNetMessage : public NetMessage { // cwnm
public:
	CheckWinNetMessage();
    Pid pid;

#ifdef LOGGING
	void ToString(char *psz, int cb) {
		snprintf(psz, cb, "ScCheckWin(pid: %d)", pid);
	}
#endif
};

inline CheckWinNetMessage::CheckWinNetMessage()
{
	nmid = knmidScCheckWin;
	cb = sizeof(CheckWinNetMessage);
	CompileAssert((sizeof(this) % 4) == 0);
}

class ChallengeWinNetMessage : public NetMessage { // cwnm
public:
	ChallengeWinNetMessage();

#ifdef LOGGING
	void ToString(char *psz, int cb) {
        strncpyz(psz, "CsChallengeWin", cb);
	}
#endif
};

inline ChallengeWinNetMessage::ChallengeWinNetMessage()
{
	nmid = knmidCsChallengeWin;
	cb = sizeof(ChallengeWinNetMessage);
	CompileAssert((sizeof(this) % 4) == 0);
}

#define knDisconnectReasonResign 0
#define knDisconnectReasonKilled 1
#define knDisconnectReasonAbnormal 2
#define knDisconnectReasonNotResponding 3
#define knDisconnectReasonLeftGame 4

STARTLABEL(DisconnectReasons)
    LABEL(knDisconnectReasonResign)
    LABEL(knDisconnectReasonKilled)
    LABEL(knDisconnectReasonAbnormal)
    LABEL(knDisconnectReasonNotResponding)
    LABEL(knDisconnectReasonLeftGame)
ENDLABEL(DisconnectReasons)

class KillLaggingPlayerNetMessage : public NetMessage {
public:
    KillLaggingPlayerNetMessage();
    Pid pid;
    word fYes;

#ifdef LOGGING
	void ToString(char *psz, int cb) {
		snprintf(psz, cb, "CsKillLaggingPlayer(pid: %d, fYes: %d)",
                pid, fYes);
	}
#endif
};

inline KillLaggingPlayerNetMessage::KillLaggingPlayerNetMessage()
{
	nmid = knmidCsKillLaggingPlayer;
	cb = sizeof(KillLaggingPlayerNetMessage);
	CompileAssert((sizeof(this) % 4) == 0);
}

class SyncErrorNetMessage : public NetMessage {
public:
    SyncErrorNetMessage();
    UpdateResult urLastStraw;
    UpdateResult aur[kcPlayersMax];

#ifdef LOGGING
	void ToString(char *psz, int cb) {
        CompileAssert(kcPlayersMax == 5);
        const char *pszT = base::Format::ToString("ScSyncErrorNetMessage "
                "pid 0: cUpdatesBlock=%d, hash=%08x "
                "pid 1: cUpdatesBlock=%d, hash=%08x "
                "pid 2: cUpdatesBlock=%d, hash=%08x "
                "pid 3: cUpdatesBlock=%d, hash=%08x "
                "pid 4: cUpdatesBlock=%d, hash=%08x "
                "urLastStraw: cUpdatesBlock=%d, hash=%08x",
                aur[0].cUpdatesBlock, aur[0].hash,
                aur[1].cUpdatesBlock, aur[1].hash,
                aur[2].cUpdatesBlock, aur[2].hash,
                aur[3].cUpdatesBlock, aur[3].hash,
                aur[4].cUpdatesBlock, aur[4].hash,
                urLastStraw.cUpdatesBlock, urLastStraw.hash);
        strncpyz(psz, pszT, cb);
	}
#endif
};

inline SyncErrorNetMessage::SyncErrorNetMessage()
{
    nmid = knmidScSyncError;
    cb = sizeof(*this);
	CompileAssert((sizeof(this) % 4) == 0);
}

class PlayerDisconnectNetMessage : public NetMessage { // pdnm
public:
	PlayerDisconnectNetMessage();
	Pid pid;
	word nReason;

#ifdef LOGGING
	void ToString(char *psz, int cb) {
		snprintf(psz, cb, "ScPlayerDisconnect(pid: %d, nReason: %s)",
                pid, DisconnectReasons.Find(nReason));
	}
#endif
};

inline PlayerDisconnectNetMessage::PlayerDisconnectNetMessage()
{
	nmid = knmidScPlayerDisconnect;
	cb = sizeof(PlayerDisconnectNetMessage);
	CompileAssert((sizeof(this) % 4) == 0);
}

class PlayerResignNetMessage : public NetMessage { // prnm
public:
	PlayerResignNetMessage();
	Pid pid;

#ifdef LOGGING
	void ToString(char *psz, int cb) {
		snprintf(psz, cb, "CsPlayerResign(pid: %d)", pid);
	}
#endif
};

inline PlayerResignNetMessage::PlayerResignNetMessage()
{
	nmid = knmidCsPlayerResign;
	cb = sizeof(PlayerResignNetMessage);
	CompileAssert((sizeof(this) % 4) == 0);
}

#ifdef __CPU_68K // 68K byte order == our chosen network byte order
#define NetMessageByteOrderSwap(a, b)
#define MessageByteOrderSwap(a, b, c)
#define SwapGameParams(a)
#else
void NetMessageByteOrderSwap(NetMessage *pnm, bool fNative);
void MessageByteOrderSwap(word cmsgs, Message *pmsg, bool fNative);
void SwapGameParams(GameParams *prams);
#endif

bool ValidateGameParams(const GameParams& params);

extern long gatGameSpeeds[15];

} // namespace wi

#endif // __NETMESSAGE_H__
