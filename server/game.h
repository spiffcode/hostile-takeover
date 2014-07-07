#ifndef __GAME_H__
#define __GAME_H__

#include "mpshared/mpht.h"
#include "mpshared/xpump.h"
#include "server/endpoint.h"
#include "server/playermgr.h"
#include "server/player.h"
#include "server/levelinfo.h"
#include "base/messagequeue.h"
#include "base/sigslot.h"
#include <map>
#include <vector>
#include <string>

namespace wi {

class Endpoint;

struct GameStats {
    dword server_id;
    dword server_start;
    dword gameid;
    GameParams params;
    LevelInfo info;
    dword start_utc;
    dword end_utc;
    int player_count;
    PlayerStats player_stats[kcPlayersMax];
};

const dword kfGameDontPostWinStats = 1;
const dword kfGameAnonBlock = 2;

class Game : public base::MessageHandler, public base::has_slots<> {
public:
    Game(Endpoint *endpoint, const GameParams& params, const LevelInfo& info,
            Server& server, dword roomid, dword ff);
    ~Game();

    dword id() { return id_; }
    const GameParams& params() { return params_; }
    bool playing() { return playing_; }
    const LevelInfo& info() { return info_; }
    dword roomid() { return roomid_; }
    const char *creator() { return creator_; }
    bool anonblock() { return (ff_ & kfGameAnonBlock) != 0; }
    bool ToggleAnonBlock(Endpoint *endpoint);
    bool IsAnonBlockAllowed(Endpoint *endpoint);
    bool IsSwapAllowed(Endpoint *endpoint);

    dword CanAddPlayer(Endpoint *endpoint);
    void AddPlayer(Endpoint *endpoint);
    void RemovePlayer(Endpoint *endpoint, int nReason, bool disconnect = true);
    void SendChat(Endpoint *endpoint, const char *chat,
            const char *unfiltered = NULL);
    void SendAdminChat(const char *name, const char *chat,
            bool mods_only = false);
    void OnNetMessage(Endpoint *endpoint, NetMessage *pnm);
    int GetNameCount();
    const PlayerName *GetNames();
    bool HasHeartbeat();
    void OnHeartbeat();
    std::vector<std::string> GetIdsString(Endpoint *endpoint);
    void KickPlayers();

    base::signal1<Game *> SignalOnDelete;
    base::signal1<Game *> SignalOnInProgress;
    base::signal1<Game *> SignalOnPlayersChange;

private:
    void OnPlayersChange();
    void OnClientCommands(Endpoint *endpoint, ClientCommandsNetMessage *pnm);
    void OnClientReady(Endpoint *endpoint, NetMessage *pnm);
    void OnPlayerJoin(Endpoint *endpoint, PlayerJoinNetMessage *pnm);
    void OnUpdateResult(Endpoint *endpoint, UpdateResultNetMessage *pnm);
    void OnPlayerResign(Endpoint *endpoint, PlayerResignNetMessage *pnm);
    void OnRequestBeginGame(Endpoint *endpoint, NetMessage *pnm);
    void OnKillLaggingPlayer(Endpoint *endpoint,
            KillLaggingPlayerNetMessage *pnm);
    void OnEndpointDelete(Endpoint *endpoint);
    void OnAdvertiserDelete(Endpoint *endpoint);
    void OnLagAcknowledge(Endpoint *endpoint);
    void OnWinStats(Endpoint *endpoint, WinStatsNetMessage *pnm);
    void OnChallengeWin(Endpoint *endpoint);
    void OnTimer();
    bool StartWait();
    void StartTiming();
    bool LagWait(long cUpdatesOldest);
    void SendClientCommands();
    dword NewGameId();
    void QueuePlayerDisconnect(Pid pid, int nReason);
    void PostWinStats();
    bool ValidateWinStats(Endpoint *endpoint, WinStatsNetMessage *pnm);
    bool ProcessCommand(Endpoint *endpoint, const char *chat,
            std::string *response, bool *broadcast);
    const char *ColorFromSide(int side);
    Player *HumanPlayerFromColorChar(const char *psz);
    void SwapPlayersCommand(Endpoint *endpoint, const char *chat,
            std::string *response);

    dword id_;
    GameParams params_;
    LevelInfo info_;
    dword roomid_;
    typedef std::vector<Endpoint *> ConnectedList;
    ConnectedList connected_;
    PlayerMgr playerMgr_;
    long cSecsStart_;
    long cSecsEnd_;
    long64 tCreated_;
    long64 tLastLagNotify_;
    long64 tStartTimeout_;
    Server& server_;
    bool playing_;
    long cUpdatesBlock_;
    long cUpdatesWaitSend_;
    long cUpdatesSync_;
    long msClock_;
    long cmsRate_;
    long64 msCommandsSent_;
    long cUpdatesAllowedLag_;
    Pid pidLagging_;
    bool syncerror_;
    Endpoint *advertiser_;
    dword advertiserId_;
    char creator_[kcbPlayerName];
    dword ff_;

    typedef std::vector<wi::Message> Commands;
    Commands cmds_;
    static dword s_gameidCounter_;
};

} // namespace wi

#endif // __GAME_H__
