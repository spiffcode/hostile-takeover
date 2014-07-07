#ifndef __ROOM_H__
#define __ROOM_H__

#include "inc/basictypes.h"
#include "base/sigslot.h"
#include "base/bytebuffer.h"
#include "base/messagehandler.h"
#include "mpshared/misc.h"
#include "server/tracker.h"
#include <map>
#include <string>
#include <vector>

namespace wi {

// Ensures Main is at top of sort, Admin is at bottom.
// Higher values are higher in the list.
const dword kroomidMain = (dword)-1;
const dword kroomidAdmin = 1;
const dword kroomidUnmoderated = 2;
const dword kroomidRegistered = 3;
const dword kroomidInvalid = 0;

// Room flags
const dword kfRmPermanent = 1;
const dword kfRmUnlimited = 2;
const dword kfRmRegisteredOnly = 4;
const dword kfRmForceExpire = 8;
const dword kfRmLocked = 16;
const dword kfRmUnmoderated = 32;

class Endpoint;
class Game;
class Server;

class Room : base::MessageHandler, public base::has_slots<> {
public:
    Room(Server *server, Endpoint *creator, const char *name,
            const char *password, dword roomid, int max_games_,
            int max_players_, dword ff);
    ~Room();

    void SendChat(Endpoint *endpoint, const char *chat,
            const char *unfiltered = NULL);
    void SendAdminChat(const char *name, const char *chat,
            bool mods_only = false);
    bool CanAddGame(Endpoint *endpoint);
    void AddGame(Endpoint *endpoint, Game *game);
    void RemoveGame(Game *game, bool disconnect = true);
    dword CanAddPlayer(Endpoint *endpoint, const char *password);
    void AddPlayer(Endpoint *endpoint);
    void RemovePlayer(Endpoint *endpoint, dword hint = 0,
            bool disconnect = true);
    Game *FindGame(dword id);
    std::vector<dword> GetGameIds();
    int GetPlayerCount();
    bool Kill();
    bool TogglePermanent(bool *result);
    bool ToggleRegistered(bool *result);
    bool HasHeartbeat() { return true; }
    void OnHeartbeat();
    std::vector<std::string> GetIdsString(Endpoint *endpoint);
    bool SetName(const char *name);

    bool active() { return !disposed_; }
    const char *name() { return name_; }
    const char *password() { return password_; }
    const char *creator() { return creator_; }
    const char *creator_ip() { return creator_ip_; }
    dword creator_id() { return creator_id_; }
    dword id() { return id_; }
    int cGames() { return gamemap_.size(); }
    bool unlimited() { return (ff_ & kfRmUnlimited) != 0; }
    bool registeredonly() { return (ff_ & kfRmRegisteredOnly) != 0; }
    bool moderated() { return (ff_ & kfRmUnmoderated) == 0; }
    Tracker& tracker() { return tracker_; }

    base::signal1<Room *> SignalOnDelete;
    base::signal1<Room *> SignalOnPlayersChange;
    base::signal1<Room *> SignalOnGamesChange;

private:
    void OnEndpointDelete(Endpoint *endpoint);
    void OnGameDelete(Game *game);
    void OnGameInProgress(Game *game);
    void OnGamePlayersChange(Game *game);
    void Broadcast(base::ByteBuffer *bb);
    dword NewRoomId(dword roomid);
    bool ProcessCommand(Endpoint *endpoint, const char *chat,
            std::string *response, bool *broadcast);

    int max_games_;
    int max_players_;
    long64 timeout_;
    const char *name_;
    const char *password_;
    const char *creator_;
    char creator_ip_[32];
    dword creator_id_;
    typedef std::map<dword,Game *> GameMap;
    GameMap gamemap_;
    typedef std::map<dword, Endpoint*> EndpointMap;
    EndpointMap endpointmap_;
    dword ff_;
    dword id_;
    static dword s_roomidCounter_;
    Tracker tracker_;
    Server *server_;
};

} // namespace wi

#endif // __ROOM_H__
