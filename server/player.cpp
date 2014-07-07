#include "server/player.h"
#include "base/tick.h"

namespace wi {

// lag time period to be declared laggy player
#define kctLagGrace 200

// to become non-laggy, the player must be not laggy for this period
#define kctLagRedemption 0

// if still laggy after this period, recommend kill
#define kctLagKill 3000

Player::Player() {
    Init(0);
}

bool Player::Init(Pid pid) {
    anonymous_ = false;
    havestats_ = false;
    cur_ = 0;
	wf_ = 0;
	pid_ = pid;
	side_ = ksideNeutral;
    endpoint_ = NULL;
	lag_ = knLagNone;
	nLagState_ = knLagNone;
    tLagStart_ = 0;
    tLastLag_ = 0;
    updates_ = -1;
    strncpyz(name_, "Unfulfilled", sizeof(name_));
    memset(alatr_, 0, sizeof(alatr_));
    memset(&ws_, 0, sizeof(ws_));
    memset(did_, 0, sizeof(did_));
    return true;
}

void Player::SetEndpoint(Endpoint *endpoint) {
    endpoint_ = endpoint;
    if (endpoint == NULL) {
        return;
    }
    strncpyz(name_, endpoint->name(), sizeof(name_));
    anonymous_ = endpoint->anonymous();
    address_ = endpoint->xpump().socket()->GetRemoteAddress();
    strncpyz(did_, endpoint->did(), sizeof(did_));
    lag_ = knLagNone;
    nLagState_ = knLagNone;
    tLagStart_ = 0;
    tLastLag_ = 0;
}

int Player::GetLagTimeout() {
	// Return seconds until this player gets to the kill state

	long64 ctElapsed = base::GetTickCount() - tLagStart_;
	if (ctElapsed > kctLagKill) {
		return 0;
    }
	return (int)((kctLagKill - ctElapsed + 50) / 100);
}

void Player::SetLagState(int nLagState) {
    nLagState_ = nLagState;
    tLastLag_ = base::GetTickCount();
    tLagStart_ = tLastLag_;
}

int Player::UpdateLagState(long cUpdates) {
	if (wf_ & kfPlrDisconnectBroadcasted) {
        SetLagState(knLagNone);
		return knLagNone;
	}

	if (updates_ < cUpdates) {
		// Player is behind

		switch (nLagState_) {
		case knLagNone:
			// Remember that pplr is lagging and when this started

			tLagStart_ = base::GetTickCount();
			nLagState_ = knLagGrace;
			break;

		case knLagGrace:
            // This state is effectively a grace period. If the player remains
            // laggy through this period, it becomes guilty of lag
			
            if (base::GetTickCount() - tLagStart_ >= kctLagGrace) {
                nLagState_ = knLagGuilty;
                tLastLag_ = base::GetTickCount();
            }
            break;

		case knLagGuilty:
		case knLagKill:
            // This player has lagged long enough to be guilty as charged.  Now
            // the player needs to be *not* laggy over a fixed period to get
            // out of this state

			tLastLag_ = base::GetTickCount();
			if (base::GetTickCount() - tLagStart_ >= kctLagKill) {
				nLagState_ = knLagKill;
            }
			break;
		}
	} else {
		// Player is not behind

		switch (nLagState_) {
		case knLagNone:
			// All is ok

			break;

		case knLagGrace:
            // This player has "caught up" during the grace period. Assume
            // there is no lag now.

			nLagState_ = knLagNone;
			break;

		case knLagGuilty:
		case knLagKill:
            // Guilty of lag and yet the player has caught up. If the player
            // can keep this state it will be declared not laggy

			if (base::GetTickCount() - tLastLag_ >= kctLagRedemption) {
				nLagState_ = knLagNone;
				break;
			}
		}
	}

	return nLagState_;
}

void Player::AddLatencyRecord(LatencyRecord *platr) {
    memmove(&alatr_[1], &alatr_[0], sizeof(alatr_) - sizeof(alatr_[0]));
    alatr_[0] = *platr;
    clatr_++;
    if (clatr_ > ARRAYSIZE(alatr_)) {
        clatr_ = ARRAYSIZE(alatr_);
    }
    updates_ = platr->cUpdatesBlock;
}

int Player::GetLatencyRecordCount() {
    return clatr_;
}

const LatencyRecord *Player::GetLatencyRecord(int i) {
    if (i < 0 || i >= clatr_) {
        return NULL;
    }
    return &alatr_[i];
}

void Player::SaveWinStats(const WinStats& ws, bool lock) {
    // Ignore if already locked
    if (ws_.ff & kfwsLocked) {
        return;
    }

    // Otherwise keep as most recent.  Filter out flags from these win stats,
    // since these came from the client. Flags will be added in later, when
    // these results get posted.
    ws_ = ws;
    ws_.ff &= (kfwsWinner | kfwsLoser);
    if (lock) {
        ws_.ff |= kfwsLocked;
    }
    havestats_ = true;
}

void Player::GetPlayerStats(PlayerStats *ps) {
    // Players can be human or computer. Humans can be in a game, and leave
    // before the game ends. Losers are given a chance to dispute winners.
    // Clients may or may not have sent win stats.

    memset(ps, 0, sizeof(*ps));
    strncpyz(ps->name, name_, sizeof(ps->name));
    ps->pid = pid_;
    address_.IPAsString(ps->ip, sizeof(ps->ip));
    strncpyz(ps->did, did_, sizeof(ps->did));

    // If this player was never used, then there are no stats for it
    if (!(wf_ & kfPlrInUse)) {
        return;
    }

    // Return stats that exist, if any
    if (havestats_) {
        ps->ws = ws_;
        ps->ws.ff |= kfwsReceivedStats;
    }

    // Humans can join, and then leave. When they leave, the player turns
    // into a computer, with kfPlrComputer set. kfPlrHumanJoined means this
    // player started out human (even if the player didn't stay till the end
    // of the game).
    if (wf_ & kfPlrHumanJoined) {
        ps->ws.ff |= kfwsHuman;
    } else {
        ps->ws.ff |= kfwsComputer;
    }

    // Remember if the player is anonymous.
    if (anonymous_) {
        ps->ws.ff |= kfwsAnonymous;
    }

#if 0
// not implemented
    // Winner/Loser is already encoded in WinStats.
    // If this player challenged the winner, note it
    if (wf_ & kfPlrChallengeWinner) {
        ps->ws.ff |= kfwsWinChallenger;
    }
#endif

    // If this player was removed at game start, remember that since the
    // stats are invalid.
    if (wf_ & kfPlrRemovedAtGameStart) {
        ps->ws.ff |= kfwsRemovedAtGameStart;
    }
}

} // namespace wi
