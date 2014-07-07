#include "server/game.h"
#include "server/ids.h"
#include "server/statsposter.h"
#include "server/room.h"
#include "server/lobby.h"
#include "base/tick.h"
#include "base/md5.h"
#include <stdlib.h>
#include <math.h>

namespace wi {

const long64 kctStartTimeout = 4500;	// 45 second timeout to start a game

// This is how much time to give all clients to acknowledge a
// lag notification message.
const long64 kctLagAckKill = 3000;

dword Game::s_gameidCounter_;

Game::Game(Endpoint *endpoint, const GameParams& params, const LevelInfo& info,
        Server& server, dword roomid, dword ff) : playerMgr_(server),
        params_(params), info_(info), server_(server), roomid_(roomid),
        ff_(ff), tLastLagNotify_(0), playing_(false), cUpdatesBlock_(-1),
        msClock_(0), cUpdatesWaitSend_(0), cmsRate_(kcmsRateMax),
        msCommandsSent_(0), pidLagging_(kpidNeutral), syncerror_(false),
        advertiser_(endpoint), advertiserId_(endpoint->id()), id_(NewGameId()),
        cSecsStart_(0), cSecsEnd_(0) {

    // This only hangs around until this endpoint joins the game,
    // so the game will go away if the endpoint goes away.
    advertiser_->SignalOnDelete.connect(this, &Game::OnAdvertiserDelete);

    strncpyz(creator_, endpoint->name(), sizeof(creator_));
    tCreated_ = base::GetTickCount();
    playerMgr_.Init(info_);
}

Game::~Game() {
    LOG() << base::Log::Format("0x%08lx", this);
    if (advertiser_ != NULL) {
        advertiser_->SignalOnDelete.disconnect(this);
        advertiser_ = NULL;
    }
    SignalOnDelete(this);
    PostWinStats();
}

dword Game::NewGameId() {
    s_gameidCounter_++;
    if (s_gameidCounter_ == 0) {
        s_gameidCounter_ = 1;
    }
    return s_gameidCounter_;
}

void Game::PostWinStats() {
    if (ff_ & kfGameDontPostWinStats) {
        return;
    }
    if (!playing_) {
        return;
    }
    
    // Collect stats

    GameStats stats;
    memset(&stats, 0, sizeof(stats));
    stats.server_id = server_.id();
    stats.server_start = server_.start_time();
    stats.gameid = id_;
    stats.params = params_;
    stats.info = info_;
    stats.start_utc = cSecsStart_;
    stats.end_utc = cSecsEnd_ != 0 ? cSecsEnd_ : base::GetSecondsUnixEpocUTC();
    int count = 0;
    Player *pplr = NULL;
    while ((pplr = playerMgr_.GetNextPlayer(pplr)) != NULL) {
        pplr->GetPlayerStats(&stats.player_stats[count]);
        count++;
    }
    stats.player_count = count;
    server_.poster().Submit(stats);
}

dword Game::CanAddPlayer(Endpoint *endpoint) {
    if (disposed_) {
        return knGameJoinResultGameNotFound;
    }
    if (playing_) {
        return knGameJoinResultInProgress;
    }

    int cpplrUnfulfilled = 0;
    Player *pplr = NULL;
    while ((pplr = playerMgr_.GetNextHumanPlayer(pplr)) != NULL) {
        if (pplr->flags() & kfPlrUnfulfilled) {
            cpplrUnfulfilled++;
        }
    }
    if (cpplrUnfulfilled == 0) {
        return knGameJoinResultGameFull;
    }
    if (endpoint->anonymous() && anonblock()) {
        return knGameJoinResultFail;
    }

    return knGameJoinResultSuccess;
}

void Game::AddPlayer(Endpoint *endpoint) {
    // Add to list; establish callbacks
    ConnectedList::iterator it = std::find(connected_.begin(),
            connected_.end(), endpoint);
    if (it == connected_.end()) {
        connected_.push_back(endpoint);
        if (endpoint == advertiser_) {
            endpoint->SignalOnDelete.disconnect(this);
            advertiser_ = NULL;
        }
        endpoint->SignalOnDelete.connect(this, &Game::OnEndpointDelete);
    }

    // Clients expect knmidScGameParams on connect
    GameParamsNetMessage nm;
    nm.nmid = knmidScGameParams;
    nm.cb = sizeof(nm);
    nm.rams = params_;
    endpoint->xpump().Send(XMsgGameNetMessage::ToBuffer(&nm));
}

void Game::RemovePlayer(Endpoint *endpoint, int reason, bool disconnect) {
    LOG() << base::Log::Format("0x%08lx ", endpoint)
            << "endpoint disconnecting, reason:" << reason;

    if (endpoint->id() == advertiserId_) {
        // If this game hasn't started yet, dispose the game
        if (!playing_) {
            LOG() << base::Log::Format("0x%08lx ", this)
                    << "advertiser leaving game, auto disposing!";
            Dispose();
        }
    }

    // Find this endpoint in the connected_ list.
    ConnectedList::iterator it = std::find(connected_.begin(),
            connected_.end(), endpoint);
    if (it == connected_.end()) {
        return;
    }

    // If this endpoint is assigned a player, unassign it, which is different
    // if playing vs. not playing yet.
    Player *pplr = playerMgr_.GetPlayer(endpoint);
    if (pplr != NULL) {
        if (!playing_) {
            // Not yet playing the game. The player this endpoint was using
            // is now available for anyone else.
            pplr->SetFlags(pplr->flags() & ~(kfPlrReady | kfPlrHumanJoined));
            pplr->SetFlags(pplr->flags() | kfPlrUnfulfilled);
            pplr->SetEndpoint(NULL);
            playerMgr_.BroadcastPlayersUpdate();

            // Notify that players have changes
            SignalOnPlayersChange(this);
        } else {
            // The game is playing; turn this player into a computer player.
            // Tell other players this player is disconnecting.
            pplr->SetFlags(pplr->flags() | kfPlrComputer);
            pplr->SetEndpoint(NULL);
            QueuePlayerDisconnect(pplr->pid(), reason);

            // Notify that players have changes
            SignalOnPlayersChange(this);
        }
    }

    // Remove signaling and remove from list. If the list size is now zero,
    // schedule this game for auto-deletion.
    connected_.erase(it);
    if (disconnect) {
        endpoint->SignalOnDelete.disconnect(this);
    }
    if (connected_.size() == 0) {
        LOG() << base::Log::Format("0x%08lx ", this)
                << "no endpoints connected - auto disposing!";
        Dispose();
    }
}

void Game::OnAdvertiserDelete(Endpoint *endpoint) {
    LOG() << "name: " << endpoint->name();

    // This will only get called if the advertiser goes away before
    // joining the game. Don't disconnect from this signal while it is
    // being called.
    Dispose();
    advertiser_ = NULL;
}

void Game::OnEndpointDelete(Endpoint *endpoint) {
    // Player should be gone by now; if not it's abnormal disconnect
    LOG() << "removing " << endpoint->name();
    RemovePlayer(endpoint, knDisconnectReasonAbnormal, false);
}

void Game::OnNetMessage(Endpoint *endpoint, NetMessage *pnm) {
    switch (pnm->nmid) {
    case knmidCsClientCommands:
        OnClientCommands(endpoint, (ClientCommandsNetMessage *)pnm);
        break;

    case knmidCsClientReady:
        OnClientReady(endpoint, pnm);
        break;

    case knmidCsPlayerJoin:
        OnPlayerJoin(endpoint, (PlayerJoinNetMessage *)pnm);
        break;

    case knmidCsUpdateResult:
        OnUpdateResult(endpoint, (UpdateResultNetMessage *)pnm);
        break;

    case knmidCsConnect:
        RLOG() << base::Log::Format("0x%08lx ", endpoint)
                << "Received knmidCsConnect unexpectedly!";
        break;

    case knmidCsPlayerResign:
        OnPlayerResign(endpoint, (PlayerResignNetMessage *)pnm);
        break;

    case knmidCsRequestBeginGame:
        OnRequestBeginGame(endpoint, pnm);
        break;

    case knmidCsKillLaggingPlayer:
        OnKillLaggingPlayer(endpoint, (KillLaggingPlayerNetMessage *)pnm);
        break;

    case knmidCsLagAcknowledge:
        OnLagAcknowledge(endpoint);
        break;

    case knmidCsWinStats:
        OnWinStats(endpoint, (WinStatsNetMessage *)pnm);
        break;

    case knmidCsChallengeWin:
        OnChallengeWin(endpoint);
        break;

    default:
        Assert(false);
        break;
    }
}

void Game::OnPlayerJoin(Endpoint *endpoint, PlayerJoinNetMessage *ppjnm) {
    // If already playing, this message means nothing
    if (playing_) {
        RLOG() << base::Log::Format("0x%08lx ", endpoint)
                << "received PlayerJoin while playing.";
        NetMessage nmsg(knmidScCantAcceptMoreConnections);
        endpoint->xpump().Send(XMsgGameNetMessage::ToBuffer(&nmsg));
        return;
    }

    // Are there any unfulfilled slots for human players?
    Player *applrUnfulfilled[kcPlayersMax];
    int cpplrUnfulfilled = 0;
    Player *pplr = NULL;
    while ((pplr = playerMgr_.GetNextHumanPlayer(pplr)) != NULL) {
        if (pplr->flags() & kfPlrUnfulfilled) {
            applrUnfulfilled[cpplrUnfulfilled++] = pplr;
        }
    }

    // Don't accept this joiner if all the human slots are in use
    if (cpplrUnfulfilled == 0) {
        NetMessage nmsg(knmidScCantAcceptMoreConnections);
        endpoint->xpump().Send(XMsgGameNetMessage::ToBuffer(&nmsg));
        return;
    }

    // Randomly assign this joiner to one of the unfulfilled player slots
    pplr = applrUnfulfilled[rand() % cpplrUnfulfilled];
    pplr->SetFlags(pplr->flags() & ~kfPlrUnfulfilled);
    pplr->SetFlags(pplr->flags() | kfPlrHumanJoined);
    //pplr->SetName(ppjnm->szPlayerName);
    pplr->SetEndpoint(endpoint);

    // Is this the first joining player? If so remember it is the creator.
    if (playerMgr_.GetEndpointCount() == 1) {
        pplr->SetFlags(pplr->flags() | kfPlrCreator);
    }

    // Notify all players of the new player + all old players
    playerMgr_.BroadcastPlayersUpdate();

    // Player names have changed
    SignalOnPlayersChange(this);
}

void Game::OnClientReady(Endpoint *endpoint, NetMessage *pnm) {
    // If already playing, this message means nothing
    if (playing_) {
        RLOG() << base::Log::Format("0x%08lx ", endpoint)
                << "received ClientReady while playing.";
        return;
    }

    // Update this player's state in the master list and let all players know
    Player *pplr = playerMgr_.GetPlayer(endpoint);
    if (pplr == NULL) {
        RLOG() << base::Log::Format("0x%08lx ", endpoint)
                << "has no player.";
        return;
    }
    pplr->SetFlags(pplr->flags() | kfPlrReady);
    playerMgr_.BroadcastPlayersUpdate();
}

void Game::OnRequestBeginGame(Endpoint *endpoint, NetMessage *pnm) {
    LOG();

    // Sent by the creator to begin a game
    NetMessage nmFail(knmidScBeginGameFail);
    if (playing_) {
        endpoint->xpump().Send(XMsgGameNetMessage::ToBuffer(&nmFail));
        RLOG() << base::Log::Format("0x%08lx ", endpoint)
                << "received RequestBeginGame while playing.";
        return;
    }

    // Make sure there are enough players ready to begin the game
    int cplrReady = 0;
    Player *pplr = NULL;
    while ((pplr = playerMgr_.GetNextHumanPlayer(pplr)) != NULL) {
        word wf = pplr->flags();
        if ((wf & kfPlrUnfulfilled) != 0) {
            continue;
        }

        // Count # of players ready. If any player isn't ready,
        // the game isn't ready to begin.
        if (wf & kfPlrReady) {
            cplrReady++;
        } else {
            endpoint->xpump().Send(XMsgGameNetMessage::ToBuffer(&nmFail));
            return;
        }
    }
    if (cplrReady < info_.minplayers()) {
        endpoint->xpump().Send(XMsgGameNetMessage::ToBuffer(&nmFail));
        return;
    }

    // All ready. Convert unfulfilled Human players into Computer players.
    // The client handles these by removing all pre-created units.
    bool fPlayersChanged = false;
    pplr = NULL;
    while ((pplr = playerMgr_.GetNextHumanPlayer(pplr)) != NULL) {
        word wf = pplr->flags();
        if (wf & kfPlrUnfulfilled) {
            pplr->SetFlags(wf | kfPlrComputer | kfPlrRemovedAtGameStart);
            fPlayersChanged = true;
        }
    }

    // Let everyone know the new player state
    if (fPlayersChanged) {
        playerMgr_.BroadcastPlayersUpdate();
    }

    // Broadcast the BeginGame notification
    BeginGameNetMessage bgnm;
    bgnm.ulRandomSeed = (dword)base::GetMillisecondCount();
    playerMgr_.Broadcast(&bgnm);

    // Remove all endpoints that aren't playing the game.
    // These are endpoints that have connected to the game, but haven't
    // joined. Normally there should be zero of these, but the protocol
    // allows for it. This casuses synchronous changes to connected_, so
    // re-enumerate if and endpoint is removed.
    while (true) {
        bool fDropped = false;
        ConnectedList::iterator it = connected_.begin();
        for (; it != connected_.end(); it++) {
            if (playerMgr_.GetPlayer(*it) == NULL) {
                (*it)->DropGame(this, knDisconnectReasonKilled);
                fDropped = true;
                break;
            }
        }
        if (!fDropped) {
            break;
        }
    }

    // The game is now playing
    cSecsStart_ = base::GetSecondsUnixEpocUTC();
    StartTiming();

    // Let room subscribers know
    SignalOnInProgress(this);
}

void Game::KickPlayers() {
    // Forcefully disconnect players, and therefore take down this game
    ConnectedList::iterator it = connected_.begin();
    for (; it != connected_.end(); it++) {
        if (!(*it)->IsModerator()) {
            (*it)->Dispose();
        }
    }
}

void Game::StartTiming() {
    // Calculate the timer rate. Rate limit it. This does not need to
    // match the client update rate - the two run independently.
    long cmsUpdate = params_.tGameSpeed * 10;
    if (cmsUpdate < kcmsRateMax) {
        cmsRate_ = kcmsRateMax;
    } else {
        cmsRate_ = cmsUpdate;
    }

    // The client sends UR for 0 to start things off

    cUpdatesWaitSend_ = 0;
    cUpdatesBlock_ = 0;
    cUpdatesSync_ = 0;
    msClock_ = cUpdatesBlock_ * cmsUpdate;
    msCommandsSent_ = base::GetMillisecondCount();

    // Calculate the lag amount: N seconds in terms of updates
    
    cUpdatesAllowedLag_ = kcmsLagging / cmsUpdate;
    pidLagging_ = kpidNeutral;

    // Begin the game timer. Remember when the timer started, so that
    // it is possible to detect lagging clients.
    tStartTimeout_ = base::GetTickCount() + kctStartTimeout;
    playing_ = true;
}

bool Game::HasHeartbeat() {
    // Need heartbeat during the game
    if (playing_) {
        return true;
    }

    // After the game has been created but before endpoints have joined,
    // the heartbeat will auto-destroy the game after awhile.
    if (connected_.size() == 0) {
        return true;
    }
    return false;
}

void Game::OnHeartbeat() {
    // Auto-dispose after awhile if no endpoints have joined this game
    if (!playing_) {
        if (connected_.size() != 0) {
            return;
        }
        if (base::GetTickCount() > tCreated_ + 100 * 30) {
            Dispose();
        }
        return;
    }

    OnTimer();
}

void Game::OnTimer() {
    LOG() << "cUpdatesBlock_: " << cUpdatesBlock_;

    // If sync error, do nothing

    if (syncerror_) {
        return;
    }

    // Wait for initial UpdateResult from all players.

    if (StartWait()) {
        return;
    }

    // Wait for lagging players. cUpdatesAllowedLag_ gives grace period of
    // "allowed lag".
    if (LagWait(cUpdatesBlock_ - cUpdatesAllowedLag_)) {
        return;
    }

    // Send the next Update to clients
    // Send two commands to start the overlap if just starting out

    if (cUpdatesWaitSend_ == 0) {
        SendClientCommands();
        SendClientCommands();
    } else {
        SendClientCommands();
    }
}

bool Game::StartWait() {
    // The first UpdateResult is sent spontaneously. If all players have
    // sent it, no need to block.

    if (playerMgr_.HaveAllPlayersReachedUpdate(0)) {
        return false;
    }

    // Have players who haven't started the same as lagging is handled
    // normally.

    LagWait(0);
    return true;
}

bool Game::LagWait(long cUpdatesOldest) {
    // Find the laggy player, if there is one

    Player *pplrLagging = playerMgr_.FindLaggingPlayer(cUpdatesOldest);

    // Remember which player is lagging, for next time

    Player *pplrLaggingLast = playerMgr_.GetPlayerFromPid(pidLagging_);
    if (pplrLagging == NULL) {
        pidLagging_ = kpidNeutral;
    } else {
        pidLagging_ = pplrLagging->pid();
    }

    // If there is no lagging player, and there was no lagging player
    // last time, we're done.

    if (pplrLagging == NULL && pplrLaggingLast == NULL) {
        return false;
    }

    // A message may need to be sent now. If there is an abrupt change, from
    // lag to no lag, or vice versa, send a message now.

    bool fSendNow = false;
    if (pplrLagging != pplrLaggingLast) {
        // There is an abrupt change from lag to no lag or vice versa.
        // Send a message so the client can change UI

        fSendNow = true;

        // If going into a lag state, mark in the players when this
        // started, so it's easy to tell if a player is not responding
        // to the LagNotify message

        playerMgr_.SetLastLagAcknowledge(base::GetTickCount());
    } else {
        // Same lagging player as last time. Send occasionally so client has
        // latest lag info. Send only if interval has expired.
        
        long64 tCurrent = base::GetTickCount();
        if (llabs(tCurrent - tLastLagNotify_) >= 50) {
            tLastLagNotify_ = tCurrent;
            fSendNow = true;
        }
    }

    // Send lag notification if necessary

    if (fSendNow) {
        LagNotifyNetMessage lnnm;
        lnnm.pidLagging = pidLagging_;
        if (pplrLagging == NULL) {
            lnnm.cSeconds = 0;
        } else {
            lnnm.cSeconds = (word)pplrLagging->GetLagTimeout();
        }
        playerMgr_.Broadcast(&lnnm, lnnm.pidLagging);

        // At least one player needs to acknowledge the lag notification, or
        // else the server will go on a killing spree (to prevent zombie
        // endpoints and games).

        if (pplrLagging != NULL && pplrLagging->endpoint() != NULL) {
            long64 tAcknowledge = playerMgr_.GetLastLagAcknowledge();
            long64 tCurrent = base::GetTickCount();
            if (tCurrent - tAcknowledge > kctLagAckKill) {
                pplrLagging->endpoint()->DropGame(this,
                        knDisconnectReasonKilled);
            }
        }
    }

    return pplrLagging != NULL;
}

void Game::OnLagAcknowledge(Endpoint *endpoint) {
    playerMgr_.SetLastLagAcknowledge(base::GetTickCount());
}

bool Game::ValidateWinStats(Endpoint *endpoint, WinStatsNetMessage *pnm) {
    Player *pplrEndpoint = playerMgr_.GetPlayer(endpoint);
    if (pplrEndpoint == NULL) {
        return false;
    }

    // Note: validate pnm->hash here, as needed to validate results sent
    // from game. See game/Player.cpp.

    // All appears fine
    return true;
}

void Game::OnWinStats(Endpoint *endpoint, WinStatsNetMessage *pnm) {
    LOG() << endpoint->name();
    Player *pplrEndpoint = playerMgr_.GetPlayer(endpoint);
    if (pplrEndpoint == NULL) {
        LOG() << base::Log::Format("0x%08lx ", endpoint)
                << "Could not find player!";
        return;
    }

    // When a client wins or loses, it sends stats for every player it knows
    // about including computer players, which means every client is sending
    // stats for every other client. The simple scenario is a two player game,
    // where simultaneously one player wins, and one loses - the stats sent
    // are consistent. If a player resigns in a 3 player game mid-way through,
    // it will send stats but those stats will be stale by the time the game
    // really ends. The server wants to record up to date stats, as much
    // as possible, so clever updating is required.
    if (!ValidateWinStats(endpoint, pnm)) {
#ifdef DEBUG_LOGGING
        LOG() << base::Log::Format("0x%08lx ", endpoint)
                << "WinStats are invalid! " << PszFromNetMessage(pnm);
#endif
#ifdef RELEASE_LOGGING
        RLOG() << base::Log::Format("0x%08lx ", endpoint)
                << "WinStats are invalid! username: " << endpoint->name()
                << " ip address: "
                << endpoint->xpump().socket()->GetRemoteAddress().ToString();
#endif
        return;
    }

    // Find the player these stats are for
    Player *pplrStats = playerMgr_.GetPlayerFromPid(pnm->pid);
    if (pplrStats == NULL) {
        return;
    }
    if (!(pplrStats->flags() & kfPlrInUse)) {
        return;
    }

    // Mark the game as ending when the first winner is announced
    if (pnm->ws.ff & kfwsWinner) {
        if (cSecsEnd_ == 0) {
            cSecsEnd_ = base::GetSecondsUnixEpocUTC();
        }
    }

    // If these stats are "owned" by the sender, treat them as
    // authoritative, locking them in place. These come from
    // human players.
    if (pplrEndpoint == pplrStats) {
        pplrStats->SaveWinStats(pnm->ws, true);
        return;
    }

    // Otherwise it's what one player thinks the stats are of another.
    // Just save this as the most recent one. Since human players lock
    // their own stats, this updates computer players (unless a human
    // player lost network connection).
    pplrStats->SaveWinStats(pnm->ws, false);

    // If this player is announcing win, allow other players to dispute it
    if (pnm->ws.ff & kfwsWinner) {
        CheckWinNetMessage cwnm;
        cwnm.pid = pnm->pid;
        playerMgr_.Broadcast(&cwnm);
    }
}

void Game::OnChallengeWin(Endpoint *endpoint) {
    LOG() << base::Log::Format("0x%08lx ", endpoint)
            << endpoint->name() << " challenges the win!";

    Player *pplr = playerMgr_.GetPlayer(endpoint);
    if (pplr == NULL) {
        LOG() << base::Log::Format("0x%08lx ", endpoint)
                << "Could not find player!";
        return;
    }

#if 0
// Not implemented.
    // An endpoint is challenging who the winner is, which will invalidate
    // the winner.
    pplr->SetFlags(pplr->flags() | kfPlrChallengeWinner);
#endif
}

void Game::SendClientCommands() {
    // Calculate when the client should next execute commands. If the commands
    // are not there, the client will block. The server attempts to send the
    // commands ahead of time, so they are there when the client needs them.

    // Update cUpdatesBlock_

    msClock_ += cmsRate_;
    long cmsUpdate = params_.tGameSpeed * 10;
    long cUpdatesBlockNew = (msClock_ + cmsUpdate - 1) / cmsUpdate; 
    if (cUpdatesBlockNew == cUpdatesBlock_) {
        return;
    }
    cUpdatesWaitSend_ = cUpdatesBlock_;
    cUpdatesBlock_ = cUpdatesBlockNew;
    msCommandsSent_ = base::GetMillisecondCount();

    LOG() << "Sending ClientCommands: cUpdatesBlock_: " << cUpdatesBlock_;

    // Send UpdateNetMessage to client
    int cmsg = cmds_.size();
    int cbUnm = sizeof(UpdateNetMessage) + ((cmsg - 1) * sizeof(Message));
    UpdateNetMessage *punm = (UpdateNetMessage *)new byte[cbUnm];
    if (punm == NULL) {
        LOG() << base::Log::Format("0x%08lx ", this)
                << "Allocation Error!";
        Dispose();
        return;
    }
    punm->nmid = knmidScUpdate;
    punm->cb = cbUnm;
    punm->cUpdatesBlock = cUpdatesBlock_;
    punm->cUpdatesSync = cUpdatesSync_;
    punm->cmsgCommands = cmsg;
    Commands::const_iterator it = cmds_.begin();
    wi::Message *pcmds = punm->amsgCommands;
    for (; it != cmds_.end(); it++) {
        *pcmds++ = *it;
    }
    cmds_.clear();

    // Send updates to all players with connections.

    Player *pplr = playerMgr_.GetNextPlayerWithEndpoint(NULL);
    for (; pplr != NULL; pplr = playerMgr_.GetNextPlayerWithEndpoint(pplr)) {
        pplr->endpoint()->xpump().Send(XMsgGameNetMessage::ToBuffer(punm));
    }
    delete punm;
}

void Game::OnKillLaggingPlayer(Endpoint *endpoint,
        KillLaggingPlayerNetMessage *pnm) {
    // If the player isn't lagging don't allow the kill

    Player *pplr = playerMgr_.GetPlayerFromPid(pnm->pid);
    if (pplr == NULL || pplr->lag() != knLagKill) {
        return;
    }

    // Kill the player or reset the lag state for another timeout

    if (pnm->fYes != 0) {
        if (pplr->endpoint() != NULL) {
            pplr->endpoint()->DropGame(this, knDisconnectReasonNotResponding);
        }
    } else {
		// Set player lag state back to guilty. This way this player gets to
        // wait through the kill timeout again.

		pplr->SetLagState(knLagGuilty);
    }
}

void Game::OnClientCommands(Endpoint *endpoint, ClientCommandsNetMessage *pnm) {
    // If not yet playing, this message means nothing
    if (!playing_) {
        RLOG() << base::Log::Format("0x%08lx ", endpoint)
                << "ClientCommands received while not in game";
        return;
    }

    // Queue the messages from the client. They'll be sent out
    // on the next game timer most likely.
    for (int i = 0; i < pnm->cmsgCommands; i++) {
        cmds_.push_back(pnm->amsgCommands[i]);
    }

    LOG() << base::Log::Format("0x%08lx seq: %d ccmds:%d",
            endpoint, pnm->nSeq, pnm->cmsgCommands);

    // TODO: Validation
}

void Game::OnUpdateResult(Endpoint *endpoint, UpdateResultNetMessage *pnm) {
    // If sync error, ignore
    if (syncerror_) {
        return;
    }

    // If not yet playing, this message means nothing
    if (!playing_) {
        RLOG() << base::Log::Format("0x%08lx ", endpoint)
                << "UpdateResult received while not in game";
        return;
    }

    Player *pplr = playerMgr_.GetPlayer(endpoint);
    if (pplr == NULL) {
        LOG() << base::Log::Format("0x%08lx ", endpoint)
                << "Could not find player!";
        return;
    }

    LOG() << "Received UpdateResult with cUpdatesBlock: " <<
            pnm->ur.cUpdatesBlock << " cmsLatency: " << pnm->ur.cmsLatency;

    LatencyRecord latr;
    latr.cUpdatesBlock = pnm->ur.cUpdatesBlock;
    latr.cmsLatency = pnm->ur.cmsLatency;
    pplr->AddLatencyRecord(&latr);

    // Track the UpdateResult, in order to compare state hashes. If there
    // is an error, inform the clients and stop the game.

    if (playerMgr_.CheckSyncError(pplr, pnm->ur, &cUpdatesSync_)) {
        syncerror_ = true;
        playerMgr_.BroadcastSyncError(pnm->ur);
        return;
    }
}

void Game::OnPlayerResign(Endpoint *endpoint, PlayerResignNetMessage *pnm) {
    // When the player resigns, the player can choose to continue to watch
    // the game, so the connection must stay and receives updates.
    Player *pplr = playerMgr_.GetPlayer(endpoint);
    if (pplr != NULL) {
        // Change to computer player and tell clients about this, but
        // keep the connection. This way this client will continue to
        // receive game updates, which will allow "observing".

        pplr->SetFlags(pplr->flags() | kfPlrComputer);

        // If the player has sent win stats already, then they aren't
        // resigning, they are leaving the game.
        if (pplr->statslocked()) {
            QueuePlayerDisconnect(pplr->pid(), knDisconnectReasonLeftGame);
        } else {
            QueuePlayerDisconnect(pplr->pid(), knDisconnectReasonResign);
        }
    }
}

void Game::QueuePlayerDisconnect(Pid pid, int nReason) {
    if (!playing_) {
        // Players don't disconnect this way when not playing
        return;
    }

    // Disconnects happen via a Message, so it is synched
    // to the same update between clients via UpdateNetMessage.
    wi::Message msg;
    memset(&msg, 0, sizeof(msg));
    msg.mid = kmidPlayerDisconnect;
    msg.PlayerDisconnect.pid = pid;
    msg.PlayerDisconnect.nReason = (word)nReason;
    cmds_.push_back(msg);
}

int Game::GetNameCount() {
    return playerMgr_.GetEndpointCount();
}

const PlayerName *Game::GetNames() {
    return playerMgr_.GetEndpointNames();
}

std::vector<std::string> Game::GetIdsString(Endpoint *endpoint) {
    std::vector<std::string> responses;
    ConnectedList::iterator it = connected_.begin();
    for (; it != connected_.end(); it++) {
        Player *pplr = playerMgr_.GetPlayer(*it);
        if (pplr == NULL) {
            continue;
        }
        if (endpoint->IsAdmin()) {
            char ip[32];
            (*it)->xpump().socket()->GetRemoteAddress().IPAsString(ip,
                    sizeof(ip));
            responses.push_back(base::Format::ToString("%s: id %d ip %s\n",
                    (*it)->name(), server_.GetChatterId(endpoint, *it), ip));
        } else {
            responses.push_back(base::Format::ToString("%s: id %d\n",
                    (*it)->name(), server_.GetChatterId(endpoint, *it)));
        }
    }
    return responses;
}

Player *Game::HumanPlayerFromColorChar(const char *psz) {
    // 0: gray, 1: blue, 2: red, 3: yellow, 4: cyan
    // Don't allow swapping gray because it is neutral.

    if (strlen(psz) == 0) {
        return NULL;
    }

    Player *pplr = NULL;
    const char *s_ch2side = "\bbryc";
    Assert(strlen(s_ch2side) == kcSides);
    for (int side = 0; s_ch2side[side] != 0; side++) {
        if (s_ch2side[side] == psz[0]) {
            pplr = playerMgr_.GetPlayer(side);
            break;
        }
    }
    if (pplr == NULL) {
        return NULL;
    }
    if (pplr->endpoint() == NULL) {
        return NULL;
    }
    if (pplr->flags() & (kfPlrComputer | kfPlrComputerOvermind |
            kfPlrUnfulfilled | kfPlrObserver | kfPlrDisconnectBroadcasted)) {
        return NULL;
    }
    if ((pplr->flags() & kfPlrInUse) == 0) {
        return NULL;
    }
    return pplr;
}

const char *Game::ColorFromSide(int side) {
    // 0: gray, 1: blue, 2: red, 3: yellow, 4: cyan
    static const char *s_colors[] = {
        "neutral", "blue", "red", "yellow", "cyan"
    };
    if (side < 0 || side >= ARRAYSIZE(s_colors)) {
        return "unknown";
    }
    return s_colors[side];
}

bool Game::IsSwapAllowed(Endpoint *endpoint) {
    if (playing_) {
        return false;
    }
    if (endpoint->id() != advertiserId_) {
        return false;
    }
    return true;
}

void Game::SwapPlayersCommand(Endpoint *endpoint, const char *chat,
        std::string *response) {
    if (playing_) {
        *response = "Can't change sides after the game has begun.";
        return;
    }

    if (endpoint->id() != advertiserId_) {
        *response = "Only the creator can issue this command.";
        return;
    }

    bool error = false;
    std::string arg_a;
    if (!endpoint->GetArgument(chat, 1, &arg_a)) {
        error = true;
    }
    std::string arg_b;
    if (!endpoint->GetArgument(chat, 2, &arg_b)) {
        error = true;
    }
    Player *playerA = HumanPlayerFromColorChar(arg_a.c_str());
    Player *playerB = HumanPlayerFromColorChar(arg_b.c_str());
    if (playerA == NULL || playerB == NULL) {
        error = true;
    }
    if (playerA == playerB) {
        error = true;
    }
    if (error) {
        *response = "This command takes two arguments of either b, r, y, or c, referring to the first letter of the color of each player.";
        return;
    }

    // Unset the ready state, so that players have to agree to the new
    // arrangement, unless the player is the creator, who is already ready
    // and literally doesn't have a ready button.
    bool was_ready = false;
    if ((playerA->flags() & (kfPlrCreator | kfPlrReady)) == kfPlrReady) {
        was_ready = true;
        playerA->SetFlags(playerA->flags() & ~kfPlrReady);
        // HACK: this message is a hack-o-rama way to show the button again
        NetMessage nmFail(knmidScBeginGameFail);
        playerA->endpoint()->xpump().Send(
                XMsgGameNetMessage::ToBuffer(&nmFail));
    }
    if ((playerB->flags() & (kfPlrCreator | kfPlrReady)) == kfPlrReady) {
        was_ready = true;
        playerB->SetFlags(playerB->flags() & ~kfPlrReady);
        // HACK: this message is a hack-o-rama way to show the button again
        NetMessage nmFail(knmidScBeginGameFail);
        playerB->endpoint()->xpump().Send(
                XMsgGameNetMessage::ToBuffer(&nmFail));
    }

    // Swap player state. This also broadcasts the changes.
    playerMgr_.SwapPlayers(playerA, playerB);

    // Use SendChat so that NULL can be sent for endpoint, to get gray text
    if (!was_ready) {
        SendChat(NULL, base::Format::ToString(
                "Swapped colors. %s is %s, and %s is %s.",
                playerA->name(), ColorFromSide(playerA->side()),
                playerB->name(), ColorFromSide(playerB->side())), NULL);
    } else {
        // Make players press ready again, so the creator can't trick
        // players into playing a certain color.
        SendChat(NULL, base::Format::ToString(
                "Swapped colors. %s is %s, and %s is %s. Players must press Ready again.",
                playerA->name(), ColorFromSide(playerA->side()),
                playerB->name(), ColorFromSide(playerB->side())), NULL);
    }
}

bool Game::ProcessCommand(Endpoint *endpoint, const char *chat,
        std::string *response, bool *broadcast) {

    ModeratorCommand cmd = endpoint->GetModeratorCommand(chat);
    if (cmd != kModeratorCommandNone) {
        server_.logger().LogModCommand(endpoint, chat);
    }

    switch (endpoint->GetModeratorCommand(chat)) {
    case kModeratorCommandRules:
        *response = endpoint->server().GetChatRules();
        *broadcast = true;
        return true;

    case kModeratorCommandSwap:
        SwapPlayersCommand(endpoint, chat, response);
        return true;

    case kModeratorCommandNone:
        return false;

    default:
        break;
    }
    return true;
}

void Game::SendChat(Endpoint *endpoint, const char *chat,
        const char *unfiltered) {
    bool broadcast = false;
    std::string response;
    if (endpoint != NULL && ProcessCommand(endpoint, chat, &response,
            &broadcast)) {
        if (!broadcast) {
            if (response.size() != 0) {
                endpoint->xpump().Send(XMsgGameReceiveChat::ToBuffer("",
                        response.c_str()));
                server_.logger().LogSystemMsg(endpoint, response.c_str());
            }
            return;
        }
        chat = response.c_str();
    }

    std::string from = "";
    if (endpoint != NULL) {
        from = endpoint->GetChatName();
    }

    Room *room = server_.lobby().FindRoom(roomid_);
    const char *pszRoomName = NULL;
    bool private_room = false;
    if (room != NULL) {
        pszRoomName = room->name();
        private_room = (room->password()[0] != 0);
    }

    if (unfiltered != NULL) {
        server_.logger().LogGameChat(endpoint, roomid_, pszRoomName,
                private_room, id(), info().title(), unfiltered);
    } else {
        server_.logger().LogGameChat(endpoint, roomid_, pszRoomName,
                private_room, id(), info().title(), chat);
    }

    ConnectedList::iterator it = connected_.begin();
    for (; it != connected_.end(); it++) {
        Player *pplr = playerMgr_.GetPlayer(*it);
        if (pplr == NULL) {
            continue;
        }
        if ((*it)->muted()) {
            continue;
        }
        (*it)->xpump().Send(XMsgGameReceiveChat::ToBuffer(from.c_str(), chat));
        if (unfiltered != NULL && (*it)->seechat()) {
            (*it)->xpump().Send(XMsgGameReceiveChat::ToBuffer("", unfiltered));
        }
    }
}

void Game::SendAdminChat(const char *name, const char *chat, bool mods_only) {
    ConnectedList::iterator it = connected_.begin();
    for (; it != connected_.end(); it++) {
        Player *pplr = playerMgr_.GetPlayer(*it);
        if (pplr == NULL) {
            continue;
        }
        if (mods_only && !(*it)->IsModerator()) {
            continue;
        }
        (*it)->xpump().Send(XMsgGameReceiveChat::ToBuffer(name, chat));
    }
}

bool Game::ToggleAnonBlock(Endpoint *endpoint) {
    if (!IsAnonBlockAllowed(endpoint)) {
        return false;
    }

    ff_ ^= kfGameAnonBlock;
    if ((ff_ & kfGameAnonBlock) == 0) {
        return true;
    }

    // Remove all endpoints that are anons
    while (true) {
        bool dropped = false;
        ConnectedList::iterator it = connected_.begin();
        for (; it != connected_.end(); it++) {
            if ((*it)->anonymous()) {
                (*it)->DropGame(this, knDisconnectReasonKilled);
                dropped = true;
                break;
            }
        }
        if (!dropped) {
            break;
        }
    }
    return true;
}

bool Game::IsAnonBlockAllowed(Endpoint *endpoint) {
    if (endpoint->anonymous()) {
        return false;
    }
    if (playing_) {
        return false;
    }
    if (endpoint->id() != advertiserId_) {
        return false;
    }

    // For now don't allow toggling. Only set once to prevent abuse
    if ((ff_ & kfGameAnonBlock) != 0) {
        return false;
    }

    return true;
}

} // namespace wi
