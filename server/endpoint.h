#ifndef __ENDPOINT_H__
#define __ENDPOINT_H__

#include <ctype.h>
#include "base/misc.h"
#include "base/socket.h"
#include "base/sigslot.h"
#include "base/messagequeue.h"
#include "inc/basictypes.h"
#include "mpshared/xpump.h"
#include "server/levelinfo.h"
#include "server/lobby.h"
#include "server/tokenbucket.h"
#include "server/tracker.h"
#include <vector>

namespace wi {

class Server;
class Lobby;
class Room;
class Game;

enum ModeratorCommand {
    kModeratorCommandNone, kModeratorCommandUnknown, kModeratorCommandIds,
    kModeratorCommandMute, kModeratorCommandUnmute,
    kModeratorCommandKick, kModeratorCommandBan, kModeratorCommandRules,
    kModeratorCommandRooms, kModeratorCommandGames, kModeratorCommandClear,
    kModeratorCommandSig, kModeratorCommandFilter, kModeratorCommandNames,
    kModeratorCommandSee, kModeratorCommandKill, kModeratorCommandPermanent,
    kModeratorCommandMods, kModeratorCommandWhisper, kModeratorCommandTitle,
    kModeratorCommandRegisteredOnly, kModeratorCommandAnonBlock,
    kModeratorCommandSwap, kModeratorCommandFlag, kModeratorCommandHelp
};

class Endpoint : public base::MessageHandler, XPumpNotify,
        public base::has_slots<> {
public:
    Endpoint(Server& server, base::Socket *socket, dword id, bool serverfull);
    ~Endpoint();
    void DropGame(Game *game, int reason = knDisconnectReasonAbnormal);
    bool HasHeartbeat() { return true; }
    void OnHeartbeat();
    bool GetArgument(const char *chat, int arg_index, std::string *arg,
            const char **rest = NULL);
    ModeratorCommand GetModeratorCommand(const char *chat);
    std::string GetChatName();
    bool IsModerator();
    bool IsAdmin();
    void AddTracker(Tracker& tracker, long64 tExpires);
    void RemoveTracker(Tracker& tracker);
    bool FindTracker(Tracker& tracker, long64 *tExpires = NULL);
    bool CanModerate(dword roomid);
    std::string GetRoomName();
    std::string GetGameName();

    XPump& xpump() { return xpump_; }
    Server& server() { return server_; }
    dword id() { return id_; }
    const char *name() { return name_; }
    bool anonymous() { return anonymous_; }
    dword clientid() { return clientid_; }
    bool muted() { return muted_; }
    bool sigvisible() { return sigvisible_; }
    bool seechat() { return seechat_; }
    const char *did() { return did_; }
    dword roomid() { return roomid_; }
    dword gameid();

    std::vector<std::string>& old_names() { return old_names_; }

    enum State {
        ES_HANDSHAKE, ES_HANDSHAKESUCCESS, ES_READY, ES_LOBBY, ES_ROOM, ES_GAME
    };
    base::signal1<Endpoint *> SignalOnDelete;

private:
    void SetState(State state);
    bool CheckState(State state0, State state1 = (State)-1);
    void OnGameDelete(Game *game);
    bool ProcessCommand(const char *chat, std::string *response);
    void RememberName(const char *name);
    bool FilterChat(const char *chat, const char **result);
    void UpdateDid(const char *did);

    // XPumpNotify overrides
    virtual void OnHandshake(dword clientid, dword protocolid);
    virtual void OnEcho();
    virtual void OnLogin(const char *username, const char *token, const char *did);
    virtual void OnSignOut();
    virtual void OnLobbyJoin();
    virtual void OnLobbyCreateRoom(const char *name, const char *password);
    virtual void OnLobbyCanJoinRoom(dword roomid, const char *password);
    virtual void OnLobbyLeave();
    virtual void OnRoomJoin(dword roomid, const char *password);
    virtual void OnRoomSendChat(const char *chat);
    virtual void OnRoomCreateGame(const GameParams& params);
    virtual void OnRoomCanJoinGame(dword gameid);
    virtual void OnRoomLeave(dword hint);
    virtual void OnGameJoin(dword gameid, dword roomid);
    virtual void OnGameSendChat(const char *chat);
    virtual void OnGameNetMessage(NetMessage **ppnm);
    virtual void OnGameLeave();
    virtual void OnError(int error);
    virtual void OnClose(int error);
    virtual void OnCloseOk();

    XPump xpump_;
    State state_;
    dword clientid_;
    dword protocolid_;
    Server& server_;
    Game *game_;
    GameParams params_;
    LevelInfo info_;
    bool echo_;
    bool okecho_;
    int missed_;
    dword id_;
    dword roomid_;
    const char *name_;
    bool anonymous_;
    bool serverfull_;
    bool admin_;
    bool muted_;
    bool sigvisible_;
    bool seechat_;
    std::vector<std::string> old_names_;
    std::string chat_fragment_;
    TokenBucket roomlimiter_;
    char did_[64];
};

STARTLABEL(EsLabels)
    LABEL(Endpoint::ES_HANDSHAKE)
    LABEL(Endpoint::ES_HANDSHAKESUCCESS)
    LABEL(Endpoint::ES_READY)
    LABEL(Endpoint::ES_LOBBY)
    LABEL(Endpoint::ES_ROOM)
    LABEL(Endpoint::ES_GAME)
ENDLABEL(EsLabels)

} // namespace wi

#endif // __ENDPOINT_H__
