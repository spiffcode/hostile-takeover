#ifndef __SERVER_H__
#define __SERVER_H__

#include "inc/basictypes.h"
#include "base/socketserver.h"
#include "base/socketaddress.h"
#include "base/socket.h"
#include "base/sigslot.h"
#include "base/messagehandler.h"
#include "mpshared/xmsglog.h"
#include "server/chatlimiter.h"
#include "server/endpoint.h"
#include "server/room.h"
#include "server/levelinfocache.h"
#include "server/serverinfoupdater.h"
#include "server/filewatcher.h"
#include "server/tracker.h"
#include "server/badwords.h"
#include "server/logger.h"
#include "yajl/wrapper/jsonbuilder.h"
#include <map>

namespace wi {

class Endpoint;
class Game;
class StatsPoster;
class ServerInfoUpdater;

class Server : base::MessageHandler, base::SocketNotify,
        public base::has_slots<> {
public:
    Server(StatsPoster& stats, XMsgLog *log, LevelInfoCache& cache,
            dword id, bool checksync, int max_rooms, int max_games_per_room,
            int max_players_per_room, int max_players,
            const std::string& modlist_path, const std::string& badwords_path);
    ~Server();

    bool Listen(const base::SocketAddress& addr);
    Game *NewGame(Endpoint *endpoint, const GameParams& params,
            const LevelInfo& info, dword roomid);
    dword NewEndpointId();
    void SetUpdater(ServerInfoUpdater *updater);
    Endpoint *GetEndpoint(dword id);
    bool IsModerator(const char *name);
    bool IsAdmin(const char *name);
    dword GetChatterId(Endpoint *endpointAsker, Endpoint *endpoint);
    Endpoint *GetEndpointFromChatterId(dword id);
    const char *GetChatRules();

    ChatLimiter& chatlimiter() { return chatlimiter_; }
    LevelInfoCache& cache() { return cache_; }
    Lobby& lobby() { return lobby_; }
    XMsgLog *log() { return log_; }
    StatsPoster& poster() { return poster_; }
    bool checksync() { return checksync_; }
    dword start_time() { return start_time_; }
    dword id() { return id_; }
    int endpoint_count_thread_safe() { return endpoint_count_thread_safe_; }
    base::SocketAddress listen_address() { return listener_->GetLocalAddress(); }
    Tracker& tracker() { return tracker_; }
    BadWords& badwords() { return badwords_; }
    Logger& logger() { return logger_; }

private:
    void OnEndpointDelete(Endpoint *endpoint);
    void OnGameDelete(Game *game);
    void OnIncomingConnection(base::Socket *incoming);
    void OnUpdaterResponse(ServerInfoUpdater *updater,
            const base::ByteBuffer& response);
    void OnCommand(const std::string command, const json::JsonMap *map);
    void OnModeratorListUpdated(ThreadedFileWatcher *watcher);

    // SocketNotify interface
    virtual void OnConnectEvent(base::Socket *socket);
    virtual void OnReadEvent(base::Socket *socket);
    virtual void OnWriteEvent(base::Socket *socket);
    virtual void OnCloseEvent(base::Socket *socket);

    // MessageHandler interface
    virtual void OnMessage(base::Message *pmsg);

    XMsgLog *log_;
    base::Socket *listener_;
    LevelInfoCache& cache_;
    bool checksync_;
    dword gameidCounter_;
    dword endpointidCounter_;
    dword start_time_;
    dword id_;

    Logger logger_;
    ChatLimiter chatlimiter_;
    Lobby lobby_;
    typedef std::map<dword, Endpoint *> EndpointMap;
    EndpointMap endpointmap_;
    int endpoint_count_thread_safe_;
    typedef std::map<dword, Game*> GameMap;
    GameMap gamemap_;
    StatsPoster& poster_;
    int max_players_;
    ServerInfoUpdater *updater_;
    ThreadedFileWatcher modlist_watcher_;
    std::vector<std::string> moderator_names_;
    std::vector<std::string> admin_names_;
    Tracker tracker_;
    BadWords badwords_;
};

} // namespace wi

#endif // __SERVER_H__
