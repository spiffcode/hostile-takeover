#ifndef __LOBBY_H__
#define __LOBBY_H__

#include "inc/basictypes.h"
#include "server/endpoint.h"
#include "server/room.h"
#include "server/tracker.h"
#include "base/bytebuffer.h"
#include "base/messagehandler.h"
#include "base/sigslot.h"
#include <map>
#include <string>
#include <vector>

namespace wi {

struct RoomSorter {
    bool operator()(dword a, dword b) const {
        return a < b;
    }
};

class Server;
class Lobby : base::MessageHandler, public base::has_slots<> {
public:
    Lobby(Server *server, int max_rooms, int max_games_per_room,
            int max_players_per_room);
    ~Lobby();

    bool CanEnter(Endpoint *endpoint);
    void Enter(Endpoint *endpoint);
    void Leave(Endpoint *endpoint, bool disconnect = true); 
    Room *FindRoom(dword idroom);
    Room *NewRoom(Endpoint *endpoint, const char *name, const char *password,
            dword roomid, dword ff, dword *result);
    std::vector<dword> GetRoomIds();
    void SendAdminChat(const char *name, const char *chat,
            bool mods_only = false);
    std::vector<std::string> GetIdsString(Endpoint *endpoint);
    Tracker& room_tracker() { return room_tracker_; }

private:
    std::string StripWhitespace(const char *name);
    void Broadcast(base::ByteBuffer *bb);
    void OnRoomDelete(Room *room);
    void OnEndpointDelete(Endpoint *endpoint);
    void OnPlayersChange(Room *room);
    void OnGamesChange(Room *room);

    // MessageHandler interface
    virtual void OnMessage(base::Message *pmsg);

    typedef std::map<dword, Endpoint *> EndpointMap;
    EndpointMap endpointmap_;
    typedef std::map<dword, Room *, RoomSorter> RoomMap;
    RoomMap roommap_;
    int max_rooms_;
    int max_games_per_room_;
    int max_players_per_room_;
    Tracker room_tracker_;
    Server *server_;
};

}

#endif // __LOBBY_H__
