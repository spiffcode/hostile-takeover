#ifndef __PLAYERMGR_H__
#define __PLAYERMGR_H__

#include "inc/basictypes.h"
#include "server/player.h"
#include "server/endpoint.h"
#include "server/levelinfo.h"
#include "server/server.h"
#include "mpshared/netmessage.h"
#include "mpshared/mpht.h"
#include "mpshared/side.h"

namespace wi {

class PlayerMgr
{
public:
    PlayerMgr(Server& server);

    void Init(const LevelInfo& info);
    int GetPlayerCount(); 
    Player *AllocPlayer(word wf);
    Player *GetPlayer(Side side);
    Player *GetPlayer(Endpoint *endpoint);
    Player *GetPlayerFromPid(Pid pid);
    Player *GetNextPlayer(Player *pplr);
    Player *GetNextHumanPlayer(Player *pplr);
    Player *GetNextPlayerWithEndpoint(Player *pplr);
	void BroadcastPlayersUpdate();
	void Broadcast(NetMessage *pnm, Pid pidIgnore = kpidNeutral);
	void BroadcastPlayerDisconnect(Pid pid, int nReason);
	bool HaveAllPlayersReachedUpdate(long cUpdates);
    Player *FindLaggingPlayer(long cUpdates);
    bool AllPlayersLagging();
    int GetEndpointCount();
    const PlayerName *GetEndpointNames();
    bool CheckSyncError(Player *pplr, const UpdateResult& ur,
            long *pcUpdatesSync);
    void BroadcastSyncError(const UpdateResult& ur);
    void SetLastLagAcknowledge(long64 t) { tLagAcknowledge_ = t; }
    long64 GetLastLagAcknowledge() { return tLagAcknowledge_; }
    void SwapPlayers(Player *playerA, Player *playerB);

private:
    long64 tLagAcknowledge_; 
    Player players_[kcPlayersMax];
    PlayerName names_[kcPlayersMax];
    Server& server_;
};

} // namespace wi

#endif // __PLAYERMGR_H__
