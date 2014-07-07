#include "server/room.h"
#include "server/endpoint.h"
#include "server/game.h"
#include "server/levelinfo.h"
#include "base/tick.h"
#include "base/log.h"
#include "mpshared/mpht.h"

namespace wi {

#define kctTimeout (120 * 100)

const int kcMinutesKickTimeoutDefault = 5;
const int kcMinutesKickTimeoutMaximum = 6 * 60;

dword Room::s_roomidCounter_;

Room::Room(Server *server, Endpoint *creator, const char *name,
        const char *password, dword roomid, int max_games, int max_players,
        dword ff) : server_(server), max_games_(max_games),
        max_players_(max_players), ff_(ff), id_(NewRoomId(roomid)) {
    name_ = AllocString(name);
    password_ = AllocString(password);
    timeout_ = base::GetTickCount() + kctTimeout;

    if (creator != NULL) {
        creator_ = AllocString(creator->name());
        creator_id_ = creator->id();
        creator->xpump().socket()->GetRemoteAddress().IPAsString(creator_ip_,
                sizeof(creator_ip_));
    } else {
        creator_ = AllocString("wis");
        creator_id_ = 0;
        strncpyz(creator_ip_, "0.0.0.0", sizeof(creator_ip_));
    }
}

Room::~Room() {
    LOG() << base::Log::Format("0x%08lx", this);
    Assert(endpointmap_.size() == 0);
    SignalOnDelete(this);
    delete creator_;
    delete name_;
    delete password_;
}

bool Room::SetName(const char *name) {
    if (*name == 0) {
        return false;
    }
    if (ff_ & kfRmLocked) {
        return false;
    }
    delete name_;
    name_ = AllocString(name);
    return true;
}

dword Room::NewRoomId(dword roomid) {
    // Rather than roomid being a static here, it makes more sense for it to be
    // an instance variable in lobby.
    if (roomid != kroomidInvalid) {
        return roomid;
    }
    s_roomidCounter_++;
    while (s_roomidCounter_ == kroomidInvalid ||
            s_roomidCounter_ == kroomidAdmin ||
            s_roomidCounter_ == kroomidMain ||
            s_roomidCounter_ == kroomidRegistered ||
            s_roomidCounter_ == kroomidUnmoderated) {
        s_roomidCounter_++;
    }
    return s_roomidCounter_;
}

std::vector<std::string> Room::GetIdsString(Endpoint *endpoint) {
    std::vector<std::string> responses;
    EndpointMap::iterator it = endpointmap_.begin();
    for (; it != endpointmap_.end(); it++) {
        if (endpoint->IsAdmin()) {
            char ip[32];
            it->second->xpump().socket()->
                    GetRemoteAddress().IPAsString(ip, sizeof(ip));
            responses.push_back(base::Format::ToString("%s: id %d ip %s",
                    it->second->name(),
                    it->second->server().GetChatterId(endpoint, it->second),
                    ip));
        } else {
            responses.push_back(base::Format::ToString("%s: id %d",
                    it->second->name(),
                    it->second->server().GetChatterId(endpoint, it->second)));
        }
    }
    return responses;
}

bool Room::ProcessCommand(Endpoint *endpoint, const char *chat,
        std::string *response, bool *broadcast) {
    ModeratorCommand cmd = endpoint->GetModeratorCommand(chat);
    if (cmd != kModeratorCommandNone) {
        server_->logger().LogModCommand(endpoint, chat);
    }

    switch (cmd) {
    case kModeratorCommandKick:
        {
            // Check the room the moderator is in, not the room the player is
            // in. This way, remote action is still available.
            if (!endpoint->CanModerate(endpoint->roomid())) {
                *response = "Can't moderate in this room.";
                return true;
            }

            Endpoint *endpointKick = NULL;
            std::string arg;
            if (endpoint->GetArgument(chat, 1, &arg)) {
                dword chatter_id = 0;
                base::Format::ToDword(arg.c_str(), 10, &chatter_id);
                endpointKick = endpoint->server().GetEndpointFromChatterId(
                        chatter_id);
            }
            if (endpointKick == NULL) {
                *response = "Could not find player using that id.";
                break;
            }
            if (endpointKick->IsModerator()) {
                *response = "You cannot kick another moderator.";
                break;
            }
            std::string minutes_str;
            int minutes = kcMinutesKickTimeoutDefault;
            if (endpoint->GetArgument(chat, 2, &minutes_str)) {
                base::Format::ToInteger(minutes_str.c_str(), 10, &minutes);
                if (minutes < 0) {
                    minutes = kcMinutesKickTimeoutDefault;
                }
                if (minutes > kcMinutesKickTimeoutMaximum) {
                    minutes = kcMinutesKickTimeoutMaximum;
                }
            }
            char ip[32];
            endpointKick->xpump().socket()->GetRemoteAddress().IPAsString(ip,
                    sizeof(ip));
            RLOG() << "mod: " << endpoint->name() << " kicked: "
                    << endpointKick->name() << " minutes: " << minutes
                    << " ip address: " << ip;
            long64 tExpires = base::GetMillisecondCount() +
                    minutes * 60 * 1000;
            endpointKick->AddTracker(tracker_, tExpires);
            endpointKick->AddTracker(endpointKick->server().lobby().room_tracker(), tExpires);
            endpointKick->server().chatlimiter().Mute(endpointKick, minutes);

            *response = base::Format::ToString("%s has been kicked from this room, is muted, and can't create new rooms for %d minute(s). Action logged.", endpointKick->name(), minutes);
            endpointKick->Dispose();
        }
        return true;

    case kModeratorCommandRules:
        // Check the room the moderator is in, not the room the player is
        // in. This way, remote action is still available.
        if (!endpoint->CanModerate(endpoint->roomid())) {
            *response = "Can't moderate in this room.";
            return true;
        }

        *response = endpoint->server().GetChatRules();
        *broadcast = true;
        return true;

    case kModeratorCommandNone:
        return false;

    default:
        break;
    } 
    return true;
}

bool Room::Kill() {
    if (ff_ & kfRmPermanent) {
        return false;
    }

    // Kick the players in the games, and the players in this room
    {
        GameMap::iterator it = gamemap_.begin();
        for (; it != gamemap_.end(); it++) {
            it->second->KickPlayers();
        }
    }
    {
        EndpointMap::iterator it = endpointmap_.begin();
        for (; it != endpointmap_.end(); it++) {
            if (!it->second->IsModerator()) {
                it->second->Dispose();
            }
        }
    }

    // Expire when possible
    ff_ |= kfRmForceExpire;
    return true;
}

bool Room::TogglePermanent(bool *result) {
    if (ff_ & (kfRmLocked | kfRmForceExpire)) {
        return false;
    }
    ff_ ^= kfRmPermanent;
    *result = ((ff_ & kfRmPermanent) != 0);
    return true;
}

bool Room::ToggleRegistered(bool *result) {
    ff_ ^= kfRmRegisteredOnly;
    *result = ((ff_ & kfRmRegisteredOnly) != 0);
    return true;
}

std::vector<dword> Room::GetGameIds() {
    std::vector<dword> ids;
    GameMap::iterator it = gamemap_.begin();
    for (; it != gamemap_.end(); it++) {
        ids.push_back(it->first);
    }
    return ids;
}

void Room::SendChat(Endpoint *endpoint, const char *chat,
        const char *unfiltered) {
    bool broadcast = false;
    std::string response;
    if (endpoint != NULL && ProcessCommand(endpoint, chat, &response,
            &broadcast)) {
        if (!broadcast) {
            if (response.size() != 0) {
                endpoint->xpump().Send(XMsgRoomReceiveChat::ToBuffer("",
                        response.c_str()));
               server_->logger().LogSystemMsg(endpoint, response.c_str());
            }
            return;
        }
        chat = response.c_str();
    }

    std::string from = "";
    if (endpoint != NULL) {
        from = endpoint->GetChatName();
    }

    if (unfiltered != NULL) {
        server_->logger().LogRoomChat(endpoint, id(), name(),
                password()[0] != 0, unfiltered);
    } else {
        server_->logger().LogRoomChat(endpoint, id(), name(),
                password()[0] != 0, chat);
    }

    LOG() << from << " said: " << chat;
    EndpointMap::iterator it = endpointmap_.begin();
    for (; it != endpointmap_.end(); it++) {
        if (it->second->muted()) {
            continue;
        }
        it->second->xpump().Send(XMsgRoomReceiveChat::ToBuffer(from.c_str(),
                chat));
        if (unfiltered != NULL && it->second->seechat()) {
            it->second->xpump().Send(XMsgRoomReceiveChat::ToBuffer("",
                    unfiltered));
        }
    }
}

void Room::SendAdminChat(const char *name, const char *chat, bool mods_only) {
    for (EndpointMap::iterator it = endpointmap_.begin();
            it != endpointmap_.end(); it++) {
        Endpoint *endpoint = it->second;
        if (mods_only && !endpoint->IsModerator()) {
            continue;
        }
        endpoint->xpump().Send(XMsgRoomReceiveChat::ToBuffer(name, chat));
    }

    for (GameMap::iterator it = gamemap_.begin(); it != gamemap_.end(); it++) {
        Game *game = it->second;
        game->SendAdminChat(name, chat, mods_only);
    }
}

bool Room::CanAddGame(Endpoint *endpoint) {
    return gamemap_.size() <= max_games_;
}

void Room::AddGame(Endpoint *endpoint, Game *game) {
    gamemap_.insert(GameMap::value_type(game->id(), game));
    game->SignalOnDelete.connect(this, &Room::OnGameDelete);
    game->SignalOnInProgress.connect(this, &Room::OnGameInProgress);
    game->SignalOnPlayersChange.connect(this, &Room::OnGamePlayersChange);
    Broadcast(XMsgRoomAddGame::ToBuffer(endpoint->name(), game->id(),
            game->params(), game->info().minplayers(),
            game->info().maxplayers(), game->info().title(), gamemap_.size()));
    SignalOnGamesChange(this);
    LOG() << gamemap_.size() << " games.";

    // Timeout the room in the future. This gives players joining this game
    // time to join, since they need to leave the room in order to join.
    timeout_ = base::GetTickCount() + kctTimeout;
}

void Room::RemoveGame(Game *game, bool disconnect) {
    GameMap::iterator it = gamemap_.find(game->id());
    if (it == gamemap_.end()) {
        LOG() << "Game not found!";
        return;
    }
    gamemap_.erase(it);
    if (disconnect) {
        game->SignalOnDelete.disconnect(this);
        game->SignalOnInProgress.disconnect(this);
        game->SignalOnPlayersChange.disconnect(this);
    }
    Broadcast(XMsgRoomRemoveGame::ToBuffer(game->id(), gamemap_.size()));
    SignalOnGamesChange(this);
    LOG() << gamemap_.size() << " games.";

    // Timeout the room in the future. This gives players leaving this
    // game time to come back to the room before the room times out.
    timeout_ = base::GetTickCount() + kctTimeout;
}

dword Room::CanAddPlayer(Endpoint *endpoint, const char *password) {
    if (disposed_) {
        return knRoomJoinResultNotFound;
    }
    // Admins can always join rooms
    if (endpoint->IsAdmin()) {
        return knRoomJoinResultSuccess;
    }

    if ((ff_ & kfRmForceExpire) != 0) {
        return knRoomJoinResultFail;
    }

    bool check = true;
    if (check && endpointmap_.size() >= max_players_) {
        return knRoomJoinResultFull;
    }
    if (registeredonly() && endpoint->anonymous()) {
        return knRoomJoinResultFail;
    }
    if (strcmp(password, password_) != 0) {
        return knRoomJoinResultWrongPassword;
    }

    // user blocked in this room?
    long64 tExpires;
    if (endpoint->FindTracker(tracker_, &tExpires)) {
        if (base::GetMillisecondCount() < tExpires) {
            return knRoomJoinResultFail;
        }
        endpoint->RemoveTracker(tracker_);
    }

    return knRoomJoinResultSuccess;
}

void Room::AddPlayer(Endpoint *endpoint) {
    // Tell this endpoint about the games in this room
    for (GameMap::iterator it = gamemap_.begin(); it != gamemap_.end(); it++) {
        Game *game = it->second;
        endpoint->xpump().Send(XMsgRoomAddGame::ToBuffer(game->creator(),
                game->id(), game->params(), game->info().minplayers(),
                game->info().maxplayers(), game->info().title(),
                gamemap_.size()));
        endpoint->xpump().Send(XMsgRoomGamePlayerNames::ToBuffer(game->id(),
                game->GetNameCount(), game->GetNames()));
        if (game->playing()) {
            endpoint->xpump().Send(XMsgRoomGameInProgress::ToBuffer(
                    game->id()));
        }
    }

    // Tell this endpoint about the players in this room
    EndpointMap::iterator it = endpointmap_.begin();
    for (; it != endpointmap_.end(); it++) {
        Endpoint *endpointT = it->second;
        endpoint->xpump().Send(XMsgRoomAddPlayer::ToBuffer(endpointT->name()));
    }

    // Tell other endpoints about this player joining the room
    Broadcast(XMsgRoomAddPlayer::ToBuffer(endpoint->name()));

    // This room wants to know when this endpoint goes away
    endpointmap_.insert(EndpointMap::value_type(endpoint->id(), endpoint));
    endpoint->SignalOnDelete.connect(this, &Room::OnEndpointDelete);

    // Reset the timeout for this room
    timeout_ = base::GetTickCount() + kctTimeout;

    // Tell listeners about the player change
    SignalOnPlayersChange(this);
}

void Room::RemovePlayer(Endpoint *endpoint, dword hint, bool disconnect) {
    EndpointMap::iterator it = endpointmap_.find(endpoint->id());
    if (it == endpointmap_.end()) {
        LOG() << "Player not found!";
        return;
    }
    endpointmap_.erase(it);

    if (disconnect) {
        endpoint->SignalOnDelete.disconnect(this);
    }

    // Tell other endpoints about this player going away
    Broadcast(XMsgRoomRemovePlayer::ToBuffer(hint, endpoint->name()));

    // Tell listeners about the player change
    SignalOnPlayersChange(this);
}

void Room::OnEndpointDelete(Endpoint *endpoint) {
    LOG() << "removing player " << endpoint->name();
    RemovePlayer(endpoint, 0, false);
}

void Room::OnGameDelete(Game *game) {
    LOG() << "removing game " << game->info().title() << " created by: " <<
            game->creator();
    RemoveGame(game, false);
}

void Room::OnGameInProgress(Game *game) {
    Broadcast(XMsgRoomGameInProgress::ToBuffer(game->id()));
}

void Room::OnGamePlayersChange(Game *game) {
    Broadcast(XMsgRoomGamePlayerNames::ToBuffer(game->id(),
            game->GetNameCount(), game->GetNames()));
    SignalOnPlayersChange(this);
}

void Room::Broadcast(base::ByteBuffer *bb) {
    // Once sent, byte buffers are owned by the xpump instance. Clone a
    // new byte buffer for each xpump instance beyond the first one.
    EndpointMap::iterator it = endpointmap_.begin();
    for (int i = 0; it != endpointmap_.end(); it++, i++) {
        Endpoint *endpoint = it->second;
        base::ByteBuffer *bbNext = NULL;
        if (i < endpointmap_.size() - 1) {
            bbNext = bb->Clone();
        }
        endpoint->xpump().Send(bb);
        bb = bbNext;
    }
}

Game *Room::FindGame(dword id) {
    GameMap::iterator it = gamemap_.find(id);
    if (it == gamemap_.end()) {
        return NULL;
    }
    return it->second;
}

void Room::OnHeartbeat() {
    // Don't time out permanent rooms
    if (ff_ & kfRmPermanent) {
        return;
    }

    // If there are players in the room, the timeout is always in the future
    if (endpointmap_.size() != 0) {
        timeout_ = base::GetTickCount() + kctTimeout;
        return;
    }

    // If games are present, timeout in the future
    if (gamemap_.size() != 0) {
        timeout_ = base::GetTickCount() + kctTimeout;
        return;
    }

    // If timed out, remove the room.
    if ((ff_ & kfRmForceExpire) != 0 || base::GetTickCount() >= timeout_) {
        // Dispose asynchronously destroys this room. If a player joins just
        // before the dispose, it would be bad, so the dispose_ flag is checked
        // here and there.
        Dispose();
    }
}

int Room::GetPlayerCount() {
    int cPlayers = endpointmap_.size();
    for (GameMap::iterator it = gamemap_.begin(); it != gamemap_.end(); it++) {
        cPlayers += it->second->GetNameCount();
    }
    return cPlayers;
}

} // namespace wi
