#include "server/lobby.h"
#include "server/server.h"
#include "base/thread.h"
#include "base/tick.h"
#include <ctype.h>

namespace wi {

const dword kidmRoomHeartbeat = 1;

#define kctRoomHeartbeat (30 * 100)

Lobby::Lobby(Server *server, int max_rooms, int max_games_per_room,
        int max_players_per_room) : server_(server), max_rooms_(max_rooms),
        max_games_per_room_(max_games_per_room),
        max_players_per_room_(max_players_per_room) {
    thread_.PostTimer(kidmRoomHeartbeat, this, kctRoomHeartbeat);
}

Lobby::~Lobby() {
}

bool Lobby::CanEnter(Endpoint *endpoint) {
    return true;
}

void Lobby::Enter(Endpoint *endpoint) {
    endpointmap_.insert(EndpointMap::value_type(endpoint->id(), endpoint));
    endpoint->SignalOnDelete.connect(this, &Lobby::OnEndpointDelete);

    // Tell this endpoint about all the rooms
    RoomMap::iterator it = roommap_.begin();
    for (; it != roommap_.end(); it++) {
        Room *room = it->second;
        endpoint->xpump().Send(XMsgLobbyAddRoom::ToBuffer(room->name(),
                room->id(), room->password()[0] != 0, room->GetPlayerCount(),
                room->cGames()));
    }

    // Broadcast lurker count
    Broadcast(XMsgLobbyLurkerCount::ToBuffer(endpointmap_.size()));
    LOG() << endpointmap_.size() << " endpoints.";
}

void Lobby::Leave(Endpoint *endpoint, bool disconnect) {
    EndpointMap::iterator it = endpointmap_.find(endpoint->id());
    if (it == endpointmap_.end()) {
        LOG() << "couldn't find endpoint.";
        return;
    }
    endpointmap_.erase(it);
    if (disconnect) {
        endpoint->SignalOnDelete.disconnect(this);
    }

    // Broadcast lurker count
    Broadcast(XMsgLobbyLurkerCount::ToBuffer(endpointmap_.size()));
    LOG() << endpointmap_.size() << " endpoints.";
}

void Lobby::OnEndpointDelete(Endpoint *endpoint) {
    LOG() << "leaving " << endpoint->name();
    Leave(endpoint, false);
}

Room *Lobby::FindRoom(dword roomid) {
    // Return the room if it's found and active (not in a disposed state)

    RoomMap::iterator it = roommap_.find(roomid);
    if (it == roommap_.end()) {
        return NULL;
    }
    Room *room = it->second;
    if (room->active()) {
        return room;
    }
    return NULL;
}

Room *Lobby::NewRoom(Endpoint *endpoint, const char *name,
        const char *password, dword roomid, dword ff, dword *result) {
    // Default result
    *result = knLobbyCreateRoomResultFail;

    // Anon's can't create rooms anymore because of the abuse
    if (endpoint != NULL && !endpoint->IsAdmin() && endpoint->anonymous()) {
        *result = knLobbyCreateRoomResultFail;
        return NULL;
    }

    std::string cleanname;
    if (endpoint != NULL) {
        cleanname = StripWhitespace(endpoint->server().badwords().Filter(name));
    } else {
        cleanname = StripWhitespace(name);
    }
    if (strlen(cleanname.c_str()) == 0) {
        *result = knLobbyCreateRoomResultFail;
        return NULL;
    }

    // See if a room exists already with this name
    RoomMap::iterator it = roommap_.begin();
    for (; it != roommap_.end(); it++) {
        if (strcmp(it->second->name(), cleanname.c_str()) == 0) {
            *result = knLobbyCreateRoomResultExists;
            return NULL;
        }
    }

    // Allow a fixed number of rooms
    if (roommap_.size() >= max_rooms_) {
        *result = knLobbyCreateRoomResultFull;
        return NULL;
    }

    // User can't create rooms?
    if (endpoint != NULL) {
        long64 tExpires;
        if (endpoint->FindTracker(room_tracker_, &tExpires)) {
            if (base::GetMillisecondCount() < tExpires) {
                *result = knLobbyCreateRoomResultFail;
                return NULL;
            }
            endpoint->RemoveTracker(room_tracker_);
        }
    }

    // No room exists by this name; create and return it
    Room *room = new Room(server_, endpoint, cleanname.c_str(), password,
            roomid, max_games_per_room_, max_players_per_room_, ff);
    roommap_.insert(RoomMap::value_type(room->id(), room));

    // Listen for these signals from the room. These get auto-deleted
    // when the room goes away.
    room->SignalOnDelete.connect(this, &Lobby::OnRoomDelete);
    room->SignalOnPlayersChange.connect(this, &Lobby::OnPlayersChange);
    room->SignalOnGamesChange.connect(this, &Lobby::OnGamesChange);

    // Tell the endpoints about this new room
    Broadcast(XMsgLobbyAddRoom::ToBuffer(cleanname.c_str(), room->id(),
            password[0] != 0, room->GetPlayerCount(), room->cGames()));

    *result = knLobbyCreateRoomResultSuccess;
    return room;
}

std::vector<dword> Lobby::GetRoomIds() {
    std::vector<dword> ids;
    RoomMap::iterator it = roommap_.begin();
    for (; it != roommap_.end(); it++) {
        ids.push_back(it->first);
    }
    return ids;
}

void Lobby::SendAdminChat(const char *name, const char *chat, bool mods_only) {
    LOG() << "name: " << name << ", chat: " << chat;
    RoomMap::iterator it = roommap_.begin();
    for (; it != roommap_.end(); it++) {
        it->second->SendAdminChat(name, chat, mods_only);
    }
}

std::string Lobby::StripWhitespace(const char *name) {
    // Strip whitespace from the start and end.
    int cchName = strlen(name);
    const char *start = name;
    for (; start < &name[cchName]; start++) {
        if (!isspace(*start)) {
            break;
        }
    }
    const char *end = &name[cchName - 1];
    for (; end > start; end--) {
        if (!isspace(*end)) {
            break;
        }
    }
    if (end < start) { 
        return "";
    }
    return std::string(start, end - start + 1);
}

void Lobby::OnRoomDelete(Room *room) {
    LOG() << "removing room " << room->name();
    RoomMap::iterator it = roommap_.find(room->id());
    if (it == roommap_.end()) {
        RLOG() << "couldn't find room.";
        return;
    }
    roommap_.erase(it);
    LOG() << roommap_.size() << " rooms.";

    // Tell the endpoints about this room remove
    Broadcast(XMsgLobbyRemoveRoom::ToBuffer(room->id()));
}

void Lobby::OnPlayersChange(Room *room) {
    Broadcast(XMsgLobbyUpdateRoom::ToBuffer(room->id(),
            room->GetPlayerCount(), room->cGames()));
}

void Lobby::OnGamesChange(Room *room) {
    Broadcast(XMsgLobbyUpdateRoom::ToBuffer(room->id(),
            room->GetPlayerCount(), room->cGames()));
}

void Lobby::OnMessage(base::Message *pmsg) {
    if (pmsg->id != kidmRoomHeartbeat) {
        return;
    }

    RoomMap::iterator it = roommap_.begin();
    while (it != roommap_.end()) {
        if (it->second->HasHeartbeat()) {
            it->second->OnHeartbeat();
        }
        it++;
    }
}

void Lobby::Broadcast(base::ByteBuffer *bb) {
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

std::vector<std::string> Lobby::GetIdsString(Endpoint *endpoint) {
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

}
