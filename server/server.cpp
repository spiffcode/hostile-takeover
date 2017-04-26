#include "inc/rip.h"
#include "base/log.h"
#include "base/tick.h"
#include "base/thread.h"
#include "server/game.h"
#include "server/server.h"

namespace wi {

const dword kidmFlushTimer = 1;
const dword kidmGameHeartbeat = 2;
const dword kidmEndpointHeartbeat = 3;

#define kctFlushTimer (30 * 100)
#define kctGameHeartbeat 25
#define kctEndpointHeartbeat (30 * 100)

// Change ChatterId calculation if these are changed.
const dword kffEndpointIdMask = 0x7ff;
const dword kcBitsFreeShiftEndpointId = 5;
const dword kffEndpointIdFreeMaskAfterShift = 0x1f;

Server::Server(StatsPoster& poster, XMsgLog *log, LevelInfoCache& cache,
        dword id, bool checksync, int max_rooms, int max_games_per_room,
        int max_players_per_room, int max_players,
        const std::string& modlist_path, const std::string& badwords_path,
        bool account_sharing) :
        poster_(poster), log_(log), cache_(cache),
        lobby_(this, max_rooms, max_games_per_room, max_players_per_room),
        id_(id), checksync_(checksync), max_players_(max_players),
        modlist_watcher_(modlist_path), badwords_(badwords_path),
        listener_(NULL), gameidCounter_(1), endpointidCounter_(1),
        endpoint_count_thread_safe_(0), updater_(NULL),
        logger_("log", id), anons_allowed_(true), account_sharing_(account_sharing) {

    start_time_ = base::GetSecondsUnixEpocUTC();

    // TODO: Move logging into separate logging class w/async thread
    if (log_ != NULL) {
        thread_.PostTimer(kidmFlushTimer, this, kctFlushTimer);
    }
    thread_.PostTimer(kidmGameHeartbeat, this, kctGameHeartbeat);
    thread_.PostTimer(kidmEndpointHeartbeat, this, kctEndpointHeartbeat);
    modlist_watcher_.SignalOnFileUpdated.connect(this,
            &Server::OnModeratorListUpdated);
}

Server::~Server() {
    if (listener_ != NULL) {
        listener_->Close();
        delete listener_;
    }
    if (updater_ != NULL) {
        updater_->SignalOnResponse.disconnect(this);
    }
}

void Server::OnMessage(base::Message *pmsg) {
    switch (pmsg->id) {
    case kidmFlushTimer:
        if (log_ != NULL) {
            log_->Flush();
        }
        break;

    case kidmGameHeartbeat:
        {
            GameMap::iterator it = gamemap_.begin();
            while (it != gamemap_.end()) {
                if (it->second->HasHeartbeat()) {
                    it->second->OnHeartbeat();
                }
                it++;
            }
        }
        break;
    
    case kidmEndpointHeartbeat:
        {
            EndpointMap::iterator it = endpointmap_.begin();
            while (it != endpointmap_.end()) {
                if (it->second->HasHeartbeat()) {
                    it->second->OnHeartbeat();
                }
                it++;
            }
        }
        break;
    }
}

bool Server::Listen(const base::SocketAddress& addr) {
    if (listener_ == NULL) {
        listener_ = thread_.ss().CreateSocket(SOCK_STREAM, this);
        if (listener_ == NULL) {
            return false;
        }
    }

    if (listener_->Bind(addr, true) < 0) {
        return false;
    }
    if (listener_->Listen() < 0) {
        return false;
    }

    RLOG() << "Listening on: " << listener_->GetLocalAddress().ToString();
    return true;
}

void Server::OnIncomingConnection(base::Socket *incoming) {
    bool serverfull = (endpointmap_.size() >= max_players_);
    dword id = NewEndpointId();
    Endpoint *endpoint = new Endpoint(*this, incoming, id, serverfull);
    if (endpoint == NULL) {
        RLOG() << "Error creating endpoint! Closing incoming socket.";
        delete incoming;
        return;
    }
    endpointmap_.insert(EndpointMap::value_type(id, endpoint));
    endpoint_count_thread_safe_ = endpointmap_.size();
    endpoint->SignalOnDelete.connect(this, &Server::OnEndpointDelete);
    LOG() << endpointmap_.size() << " endpoints.";
}

Endpoint *Server::GetEndpoint(dword id) {
    EndpointMap::iterator it = endpointmap_.find(id);
    if (it == endpointmap_.end()) {
        return NULL;
    }
    return it->second;
}

void Server::OnEndpointDelete(Endpoint *endpoint) {
    EndpointMap::iterator it = endpointmap_.find(endpoint->id());
    if (it == endpointmap_.end()) {
        RLOG() << "couldn't find endpoint.";
        return;
    }
    endpointmap_.erase(it);
    endpoint_count_thread_safe_ = endpointmap_.size();
    LOG() << endpointmap_.size() << " endpoints.";
}

Game *Server::NewGame(Endpoint *endpoint, const GameParams& params,
        const LevelInfo& info, dword roomid) {
    Game *game = new Game(endpoint, params, info, *this, roomid, 0);
    gamemap_.insert(GameMap::value_type(game->id(), game));
    game->SignalOnDelete.connect(this, &Server::OnGameDelete);
    return game;
}

void Server::OnGameDelete(Game *game) {
    GameMap::iterator it = gamemap_.find(game->id());
    if (it == gamemap_.end()) {
        RLOG() << "couldn't find game.";
        return;
    }
    gamemap_.erase(it);
    LOG() << gamemap_.size() << " games.";
}

void Server::OnConnectEvent(base::Socket *socket) {
    Assert(false);
}

void Server::OnReadEvent(base::Socket *socket) {
    Assert(socket == listener_);
    while (true) {
        base::Socket *incoming = socket->Accept(NULL);
        if (incoming == NULL) {
            if (socket->IsBlocking()) {
                return;
            }
            continue;
        }

        // If the ip is banned currently, don't allow the connection.
        base::SocketAddress addr = incoming->GetRemoteAddress();
        char szT[64];
        addr.IPToString(addr.ip(), szT, sizeof(szT));
        if (tracker_.Find(szT)) {
            delete incoming;
            continue;
        }
        LOG() << "Incoming connection from: " << addr.ToString();
        OnIncomingConnection(incoming);
    }
}

void Server::OnWriteEvent(base::Socket *socket) {
    LOG() << "Unexpected Write event on listening socket";
    Assert(false);
}

void Server::OnCloseEvent(base::Socket *socket) {
    LOG() << "Unexpected Close event on listening socket";
}

const char *Server::GetChatRules() {
    return "RULES: No swearing, name calling, sexism, racism, offensive language, or offensive symbolism allowed. No spamming allowed. Repeat offenders may be muted, kicked, banned, and reported to the admin.";
}

std::string Server::GetAnnouncements() {
    return announcements_;
}

void Server::SetAnnouncements(std::string announcements) {
    announcements_ = announcements;
}

dword Server::GetChatterId(Endpoint *endpointAsker, Endpoint *endpoint) {
    dword shifted = (endpoint->id() << kcBitsFreeShiftEndpointId);
    dword h = 0;
    if (!endpointAsker->IsAdmin()) {
        h = HashString(endpoint->name());
    }
    return (shifted | (h & kffEndpointIdFreeMaskAfterShift));
}

Endpoint *Server::GetEndpointFromChatterId(dword chatter_id) {
    dword id = (chatter_id >> kcBitsFreeShiftEndpointId) & kffEndpointIdMask;
    return GetEndpoint(id);
}

dword Server::NewEndpointId() {
    // This is user visible; it is used for the anonymous login id,
    // so keep the integer as small as possible

    while (true) {
        endpointidCounter_++;
        if (endpointidCounter_ > kffEndpointIdMask) {
            endpointidCounter_ = 1;
        }
        EndpointMap::iterator it = endpointmap_.find(endpointidCounter_);
        if (it == endpointmap_.end()) {
            return endpointidCounter_;
        }
    }
}

void Server::SetUpdater(ServerInfoUpdater *updater) {
    updater_ = updater;
    updater_->SignalOnResponse.connect(this, &Server::OnUpdaterResponse);
}

void Server::OnUpdaterResponse(ServerInfoUpdater *updater,
        const base::ByteBuffer& response) {
    LOG() << std::string((const char *)response.Data(), response.Length());

    // Commands get sent to the game server as responses to ServerInfo
    // requests. This is where the responses get processed. Try to
    // parse as a json object, and if success do command dispatching.
    json::JsonBuilder builder;
    builder.Start(NULL);
    builder.Update((const char *)response.Data(), response.Length());
    json::JsonObject *obj = builder.End();
    if (obj == NULL || obj->type() != json::JSONTYPE_MAP) {
        delete obj;
        return;
    }
    json::JsonMap *map = (json::JsonMap *)obj;

    // Look for a command
    const json::JsonObject *objT = map->GetObject("command");
    if (objT == NULL || objT->type() != json::JSONTYPE_STRING) {
        delete obj;
        return;
    }

    json::JsonString *obj_command = (json::JsonString *)objT;
    OnCommand(obj_command->GetString(), map);
    delete obj;
}

void Server::OnCommand(const std::string command, const json::JsonMap *map) {
    if (command == "drain") {
        if (updater_ != NULL) {
            updater_->set_drain();
        }
        return;
    } else if (command == "undrain") {
        if (updater_ != NULL) {
            updater_->clear_drain();
        }
        return;
    } else if (command == "chat") {
        const json::JsonObject *objN = map->GetObject("name");
        if (objN == NULL || objN->type() != json::JSONTYPE_STRING) {
            return;
        }

        const json::JsonObject *objM = map->GetObject("message");
        if (objM == NULL || objM->type() != json::JSONTYPE_STRING) {
            return;
        }

        json::JsonString *name = (json::JsonString *)objN;
        json::JsonString *message = (json::JsonString *)objM;
        lobby_.SendAdminChat(name->GetString(), message->GetString());
    } else if (command == "setinfo") {
        const json::JsonObject *objK = map->GetObject("key");
        if (objK == NULL || objK->type() != json::JSONTYPE_STRING) {
            return;
        }
        const json::JsonObject *objV = map->GetObject("value");
        if (objV == NULL || (objV->type() != json::JSONTYPE_STRING &&
            objV->type() != json::JSONTYPE_NUMBER)) {
            return;
        }

        json::JsonString *key = (json::JsonString *)objK;
        json::JsonString *value = (json::JsonString *)objV;

        if (updater_ != NULL) {
            updater_->SetInfo(key->GetString(),
            objV->type() == json::JSONTYPE_NUMBER ? ((json::JsonNumber *)value)->GetString() :
                ((json::JsonString *)value)->GetString());
        }
    }
}

void Server::OnModeratorListUpdated(ThreadedFileWatcher *watcher) {
    FILE *f = fopen(watcher->filename().c_str(), "r");
    if (f != NULL) {
        std::vector<std::string> moderator_names;
        std::vector<std::string> admin_names;
        char sz[256];
        while (fgets(sz, sizeof(sz), f) != NULL) {
            int cch;
            const char *start = StripWhitespace(sz, &cch);
            if (cch == 0) {
                continue;
            }
            if (strncmp(start, "mod,", 4) == 0) {
                std::string name(start + 4, cch - 4);
                //RLOG() << "moderator: '" << name << "'";
                moderator_names.push_back(name);
            }
            if (strncmp(start, "admin,", 6) == 0) {
                std::string name(start + 6, cch - 6);
                //RLOG() << "admin: '" << name << "'";
                admin_names.push_back(name);
            }
        }
        fclose(f);
        moderator_names_ = moderator_names;
        admin_names_ = admin_names;
    }
    RLOG() << watcher->filename() << " has changed! " << moderator_names_.size() << " moderators, " << admin_names_.size() << " admins.";
}

bool Server::IsModerator(const char *name) {
    for (int i = 0; i < moderator_names_.size(); i++) {
        if (stricmp(name, moderator_names_[i].c_str()) == 0) {
            return true;
        }
    }
    return false;
}

bool Server::IsAdmin(const char *name) {
    for (int i = 0; i < admin_names_.size(); i++) {
        if (stricmp(name, admin_names_[i].c_str()) == 0) {
            return true;
        }
    }
    return false;
}

bool Server::AnonsAllowed() {
    return anons_allowed_;
}

void Server::SetAnonsAllowed(bool anons_allowed) {
    anons_allowed_ = anons_allowed;
}

bool Server::SharedAccountExists(Endpoint *endpointAsker, const char *name) {
    if (endpointAsker == NULL)
        return false;

    EndpointMap::iterator it = endpointmap_.begin();
    for (; it != endpointmap_.end(); it++) {
        if (it->second == NULL)
            continue;

        if (it->second->name() == NULL)
            continue;

        // Do the names match?
        if (strcmp(name, it->second->name()) == 0) {

            // Don't count endpointAsker
            if (endpointAsker->id() != it->second->id()) {

                // We found a shared account
                return true;
            }
        }
    }
    return false;
}

void Server::DisconnectSharedAccounts(Endpoint *endpointAsker, const char *name) {
    if (endpointAsker == NULL)
        return;

    EndpointMap::iterator it = endpointmap_.begin();
    for (; it != endpointmap_.end(); it++) {
        if (it->second == NULL)
            continue;

        if (it->second->name() == NULL)
            continue;

        // Do the names match?
        if (strcmp(name, it->second->name()) == 0) {

            // Son't disconnect the asker, only other endpoints
            if (endpointAsker->id() != it->second->id()) {

                // We found a shared account, dispose of it
                it->second->Dispose();
            }
        }
    }
}

bool Server::AccountSharing() {
    return account_sharing_;
}

} // namespace wi
