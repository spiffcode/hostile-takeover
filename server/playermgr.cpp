#include "server/playermgr.h"

namespace wi {

#define CHECK_SYNC_ERRORS

PlayerMgr::PlayerMgr(Server& server) : server_(server), tLagAcknowledge_(0) {
}

void PlayerMgr::Init(const LevelInfo& info) {
    // Create Players for all sides. Until we know otherwise, assume
    // the human players will be unfulfilled and need to be removed.
    // Map authors can set up the players incorrectly. For example,
    // a map may be set to 2 player, but there is 1 human and 3
    // computer players. Fix that up.

    int cHuman = 0;
    for (Side side = kside1; side < kcSides; side++) {
        word wf = 0;
        int intelligence = info.GetIntelligence(side);
        switch (intelligence) {
        case knIntelligenceComputer:
        case knIntelligenceComputerNeutral:
            wf = kfPlrComputer | kfPlrReady;
            break;

        case knIntelligenceComputerOvermind:
            wf = kfPlrComputer | kfPlrComputerOvermind | kfPlrReady;
            break;

        case knIntelligenceHuman:
            cHuman++;
            wf = kfPlrUnfulfilled;
            break;
        }
        Player *player = AllocPlayer(wf);
        if (player == NULL) {
            continue;
        }
        player->SetSide(side);
    }

    // Convert some computer players to human if necessary

    int cHumansNeeded = info.maxplayers() - cHuman;
    if (cHumansNeeded > 0) {
        for (Side side = kside1; cHumansNeeded > 0 && side < kcSides; side++) {
            Player *pplr = GetPlayer(side);
            if (pplr == NULL || !(pplr->wf_ & kfPlrComputer)) {
                continue;
            }
            pplr->wf_ &= ~(kfPlrComputer | kfPlrComputerOvermind | kfPlrReady);
            pplr->wf_ |= kfPlrUnfulfilled;
            cHumansNeeded--;
        }
    }

    // Really only necessary for the computer players, but some players
    // are slotted for human, but then get converted to computer players
    // when the game starts (if there are unfulfilled slots), so name
    // them all AI

    int count = 1;
    for (Side side = kside1; side < kcSides; side++) {
        Player *pplr = GetPlayer(side);
        if (pplr == NULL) {
            continue;
        }
        pplr->SetName("AI");
    }
}

Player *PlayerMgr::AllocPlayer(word wf) {
	Player *player = players_;
	for (Pid pid = 0; pid < ARRAYSIZE(players_); pid++, player++) {
		if (player->wf_ & kfPlrInUse) {
			continue;
        }

		player->Init(pid);
		player->wf_ |= kfPlrInUse | wf;
		return player;
	}

	RLOG() << "this shouldn't happen!";
	return NULL;
}

void PlayerMgr::SwapPlayers(Player *playerA, Player *playerB) {
    Endpoint *endpointA = playerA->endpoint();
    word flagsA = playerA->flags();
    Endpoint *endpointB = playerB->endpoint();
    word flagsB = playerB->flags();
    playerA->SetEndpoint(endpointB);
    playerA->SetFlags(flagsB);
    playerB->SetEndpoint(endpointA);
    playerB->SetFlags(flagsA);
    BroadcastPlayersUpdate();
}

int PlayerMgr::GetEndpointCount() {
    // This counts players with endpoints (that are receiving updates),
    // whether human or observer (player with endpoint marked as computer).

    int cEndpoints = 0;
	Player *pplr = players_;
	for (int i = 0; i < kcPlayersMax; i++, pplr++) {
        if (pplr->endpoint_ != NULL) {
            cEndpoints++;
        }
	}
    return cEndpoints;
}

const PlayerName *PlayerMgr::GetEndpointNames() {
    // The creator always goes first
    Player *pplr = players_;
    PlayerName *name = names_;
    for (int i = 0; i < kcPlayersMax; i++, pplr++) {
        if (pplr->endpoint_ != NULL && (pplr->wf_ & kfPlrCreator) != 0) {
            strncpyz(name->szPlayerName, pplr->name_,
                    sizeof(name->szPlayerName));
            name++;
        }
    }

    // Then the other players
    pplr = players_;
    for (int i = 0; i < kcPlayersMax; i++, pplr++) {
        if (pplr->endpoint_ != NULL && (pplr->wf_ & kfPlrCreator) == 0) {
            strncpyz(name->szPlayerName, pplr->name_,
                    sizeof(name->szPlayerName));
            name++;
        }
    }

    return names_;
}

Player *PlayerMgr::GetPlayer(Side side) {
	Player *pplr = players_;
	for (int i = 0; i < kcPlayersMax; i++, pplr++) {
		if (pplr->side_ == side) {
			return pplr;
        }
	}
	return NULL;
}

Player *PlayerMgr::GetPlayer(Endpoint *endpoint) {
	Player *pplr = players_;
	for (int i = 0; i < kcPlayersMax; i++, pplr++) {
		if (pplr->endpoint() == endpoint) {
			return pplr;
        }
	}
	return NULL;
}

Player *PlayerMgr::GetPlayerFromPid(Pid pid) {
    // Pid is unsigned so this is a full range check
    if (pid != kpidNeutral && pid < kcPlayersMax) {
        return &players_[((int)pid)];
    }
    return NULL;
}

Player *PlayerMgr::GetNextHumanPlayer(Player *pplr) {
	int i = pplr == NULL ? 0 : pplr->pid_ + 1;
	pplr = &players_[i];
	for (; i < kcPlayersMax; i++, pplr++) {
		if ((pplr->wf_ & (kfPlrInUse | kfPlrComputer)) == kfPlrInUse) {
			return pplr;
        }
	}

	return NULL;
}

Player *PlayerMgr::GetNextPlayerWithEndpoint(Player *pplr) {
    // Players that resign but are observing have kfPlrComputer set
    // and endpoint_ != NULL. These players still receive updates,
    // participate in lag detection, can be kicked, etc.

	int i = pplr == NULL ? 0 : pplr->pid_ + 1;
	pplr = &players_[i];
	for (; i < kcPlayersMax; i++, pplr++) {
		if ((pplr->wf_ & kfPlrInUse) && pplr->endpoint_ != NULL) {
			return pplr;
        }
	}

	return NULL;
}

Player *PlayerMgr::GetNextPlayer(Player *pplr) {
	int i = (pplr == NULL) ? 0 : pplr->pid_ + 1;
	pplr = &players_[i];
	for (; i < kcPlayersMax; i++, pplr++) {
		if (pplr->wf_ & kfPlrInUse) {
			return pplr;
        }
	}

	return NULL;
}

void PlayerMgr::Broadcast(NetMessage *pnm, Pid pidIgnore) {
    for (int i = 0; i < ARRAYSIZE(players_); i++) {
        Player *pplr = &players_[i];
		if (!(pplr->wf_ & kfPlrInUse)) {
			continue;
        }
        if (pplr->pid_ == pidIgnore) {
            continue;
        }
        Endpoint *endpoint = pplr->endpoint();
        if (endpoint != NULL) {
            endpoint->xpump().Send(XMsgGameNetMessage::ToBuffer(pnm));
        }
	}
}

void PlayerMgr::BroadcastPlayersUpdate() {
	// Allocate a properly sized PlayersUpdateNetMessage

	int cplrs = GetPlayerCount();
	int cb = sizeof(PlayersUpdateNetMessage) + sizeof(PlayerRecord) *
            (cplrs - 1);
	PlayersUpdateNetMessage *ppunm = (PlayersUpdateNetMessage *)new byte[cb];
	if (ppunm == NULL)
		return;

	// Fill the message in with all the juicy pplr data
    memset(ppunm, 0, cb);
	ppunm->nmid = knmidScPlayersUpdate;
	ppunm->cb = cb;
	ppunm->cplrr = cplrs;

	Player *pplr = players_;
	PlayerRecord *record = ppunm->aplrr;
	for (int i = 0; i < kcPlayersMax; i++, pplr++) {
		if (!(pplr->wf_ & kfPlrInUse)) {
            continue;
        }
        strncpyz(record->szName, pplr->name_, sizeof(record->szName));
        record->side = pplr->side_;
        record->wf = 0;
        if (pplr->wf_ & kfPlrReady)
            record->wf |= kfPlrrReady;
        if (pplr->wf_ & kfPlrComputer)
            record->wf |= kfPlrrComputer;
        if (pplr->wf_ & kfPlrComputerOvermind)
            record->wf |= kfPlrrComputerOvermind;
        if (pplr->wf_ & kfPlrUnfulfilled)
            record->wf |= kfPlrrUnfulfilled;
        if (pplr->wf_ & kfPlrCreator)
            record->wf |= kfPlrrCreator;
        record->pid = pplr->pid_;
        record++;
	}

	// Send it off to all pplrs
	pplr = players_;
	for (int i = 0; i < kcPlayersMax; i++, pplr++) {
		if (!(pplr->wf_ & kfPlrInUse)) {
            continue;
        }

        // Computer pplrs don't need to know about the other pplrs
        if (pplr->endpoint_ == NULL) {
            continue;
        }

        // Mark this Connection's PlayerRecord so it knows which
        // Player it is.
        PlayerRecord *record = ppunm->aplrr;
        for (int j = 0; j < cplrs; j++, record++) {
            if (pplr->pid() == record->pid) {
                record->wf |= kfPlrrLocal;
            } else {
                record->wf &= ~kfPlrrLocal;
            }
        }

        pplr->endpoint()->xpump().Send(XMsgGameNetMessage::ToBuffer(ppunm));
	}

	delete ppunm;
}

void PlayerMgr::BroadcastPlayerDisconnect(Pid pid, int nReason)
{
	Player *pplr = GetPlayerFromPid(pid);
	if (pplr == NULL)
		return;

	// If already notified of disconnection, do nothing else
	// This prevents knDisconnectReasonAbnormal after Kicked or Resigned.

	if (pplr->wf_ & kfPlrDisconnectBroadcasted) {
		return;
    }
	pplr->wf_ |= kfPlrDisconnectBroadcasted;

	// Tell the other clients about it

	PlayerDisconnectNetMessage pdnm;
	pdnm.pid = pid;
	pdnm.nReason = nReason;
	Broadcast(&pdnm);
}

bool PlayerMgr::HaveAllPlayersReachedUpdate(long cUpdates) {
	Player *pplr = players_;
	for (int i = 0; i < kcPlayersMax; i++, pplr++) {
		if ((pplr->wf_ & kfPlrInUse) && pplr->endpoint_ != NULL) {
            if (pplr->updates() < cUpdates) {
                return false;
            }
        }
    }
    return true;
}

Player *PlayerMgr::FindLaggingPlayer(long cUpdates) {
    // Update lag info for each player, return the most laggy

    long cUpdatesMostLagging = -999;
    Player *pplrMostLagging = NULL;

	Player *pplr = players_;
	for (int i = 0; i < kcPlayersMax; i++, pplr++) {
		if (!(pplr->wf_ & kfPlrInUse) || pplr->endpoint_ == NULL) {
            continue;
        }
        int nLagState = pplr->UpdateLagState(cUpdates);
        if (nLagState == knLagGuilty || nLagState == knLagKill) {
            long cUpdatesLagging = cUpdates - pplr->updates();
            if (cUpdatesLagging > cUpdatesMostLagging) {
                cUpdatesMostLagging = cUpdatesLagging;
                pplrMostLagging = pplr;
            }
        }
    }

	return pplrMostLagging;
}

bool PlayerMgr::AllPlayersLagging() {
	Player *pplr = players_;
	for (int i = 0; i < kcPlayersMax; i++, pplr++) {
		if (!(pplr->wf_ & kfPlrInUse) || pplr->endpoint_ == NULL) {
            continue;
        }
        if (pplr->lag() != knLagKill) {
            return false;
        }
    }
    return true;
}

int PlayerMgr::GetPlayerCount()
{
	int cplr = 0;
	Player *pplr = players_;
	for (int i = 0; i < kcPlayersMax; i++, pplr++) {
		if (pplr->wf_ & kfPlrInUse) {
			cplr++;
        }
	}

	return cplr;
}

bool PlayerMgr::CheckSyncError(Player *pplr, const UpdateResult& ur,
        long *pcUpdatesSync) {
    // Add to the end. It should always fit, but just in case, check for that.

    if (pplr->cur_ >= ARRAYSIZE(pplr->aur_)) {
        LOG() << "UpdateResult array filled up!";
        return true;
    }
    pplr->aur_[pplr->cur_] = ur;
    pplr->cur_++;

    // Starting from the beginning, march forward through URs for all players.
    // If an UR has been received for all players, compare the hash. If there
    // is a mismatch, then a sync error has been discovered. If not, left
    // shift the UR arrays, and continue.

    long cUpdatesBlock = pplr->aur_[0].cUpdatesBlock;
    long hash = pplr->aur_[0].hash;
    bool syncerror = false;
    bool advance = true;

    while (true) {
        Player *pplrT = players_;
        for (int i = 0; i < kcPlayersMax; i++, pplrT++) {
            if (!(pplrT->wf_ & kfPlrInUse) || pplrT->endpoint_ == NULL) {
                continue;
            }
            if (pplrT->cur_ == 0) {
                advance = false;
                break;
            }
            if (pplrT->aur_[0].cUpdatesBlock != cUpdatesBlock) {
                advance = false;
                break;
            }
            if (server_.checksync() && pplrT->aur_[0].hash != hash) {
                advance = false;
                syncerror = true;
                break;
            }
        }
        if (!advance) {
            break;
        }

        // Update the caller's idea of where the players are
        *pcUpdatesSync = cUpdatesBlock;

        // All clients have reached this update result. Left shift.
        pplrT = players_;
        for (int i = 0; i < kcPlayersMax; i++, pplrT++) {
            if (!(pplrT->wf_ & kfPlrInUse) || pplrT->endpoint_ == NULL) {
                continue;
            }
            if (pplrT->cur_ == 0) {
                LOG() << "shouldn't happen";
                // Shouldn't happen since this was checked already
                continue;
            }

            // Left shift by one entry
            memmove(&pplrT->aur_[0], &pplrT->aur_[1], ELEMENTSIZE(pplrT->aur_) *
                    (pplrT->cur_ - 1));
            pplrT->cur_--;
        }

        // Loop and try the next entry. If this is working correctly, it won't
        // find advancement.
    }

    return syncerror;
}

void PlayerMgr::BroadcastSyncError(const UpdateResult& ur) {
    // Prepare the net message

    SyncErrorNetMessage senm;
    Player *pplrT = players_;
    for (int i = 0; i < kcPlayersMax; i++, pplrT++) {
        if (pplrT->cur_ != 0) {
            senm.aur[i] = pplrT->aur_[0];
        } else {
            memset(&senm.aur[i], 0xff, ELEMENTSIZE(senm.aur));
        }
    }
    senm.urLastStraw = ur;

    pplrT = players_;
    for (int i = 0; i < kcPlayersMax; i++, pplrT++) {
        if (!(pplrT->wf_ & kfPlrInUse) || pplrT->endpoint_ == NULL) {
            continue;
        }
        pplrT->endpoint()->xpump().Send(XMsgGameNetMessage::ToBuffer(&senm));
	}
}

} // namespace wi
