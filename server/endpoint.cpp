#include "server/endpoint.h"
#include "server/game.h"
#include "server/ids.h"
#include "server/tokenauth.h"
#include "server/room.h"
#include "mpshared/xpump.h"
#include "mpshared/messages.h"
#include "base/sigslot.h"
#include "base/log.h"
#include "base/tick.h"

namespace wi {

#define MISSED_ECHO_COUNT 3
#define CHAT_FRAGMENT_LENGTH 250

const int kcMinutesBanTimeoutDefault = 5;
const int kcMinutesBanTimeoutMaximum = 60 * 24 * 2;

Endpoint::Endpoint(Server& server, base::Socket *socket, dword id,
        bool serverfull) : server_(server), id_(id),
        serverfull_(serverfull), clientid_(0),
        protocolid_(0), state_(ES_HANDSHAKE), game_(NULL), echo_(true),
        roomid_(0), name_(NULL), anonymous_(true), missed_(0), okecho_(false),
        admin_(false), muted_(false), sigvisible_(false), seechat_(false),
        roomlimiter_(2, 120) {
    did_[0] = 0;
    xpump_.Attach(socket, this, server.log());
}

Endpoint::~Endpoint() {
    LOG() << base::Log::Format("0x%08lx", this);
    SignalOnDelete(this);
    delete name_;
}

void Endpoint::SetState(State state)
{
    LOG() << base::Log::Format("0x%08lx ", this)
            << "From: " << EsLabels.Find(state_)
            << " To: " << EsLabels.Find(state);
    state_ = state;
}

bool Endpoint::CheckState(State state0, State state1)
{
#ifdef DEV_BUILD
    Assert(state0 == state_ || state1 == state_);
#endif
    if (state0 != state_ && state1 != state_) {
        LOG() << base::Log::Format("0x%08lx ", this)
                << "Warning! Current: " << EsLabels.Find(state_);
        if (state1 == (State)-1) {
            LOG() << base::Log::Format("0x%08lx ", this)
                    << " Expected: " << EsLabels.Find(state0);
        } else {
            LOG() << base::Log::Format("0x%08lx ", this)
                    << " Expected: " << EsLabels.Find(state0)
                    << " or " << EsLabels.Find(state1);
        }
        return false;
    }
    return true;
}

void Endpoint::OnHandshake(dword clientid, dword protocolid) {
    if (!CheckState(ES_HANDSHAKE)) {
        xpump_.Send(XMsgHandshakeResult::ToBuffer(knHandshakeResultFail, 0));
        return;
    }

    // Check protocol; let client know result
    clientid_ = clientid;
    protocolid_ = protocolid;

    // The server and client need to support the same protocol.
    if (protocolid != kdwProtocolCurrent) {
        xpump_.Send(XMsgHandshakeResult::ToBuffer(knHandshakeResultFail, 0));
        RLOG() << base::Log::Format("0x%08lx ", this)
                << "wrong protocolid: " << protocolid
                << " clientid: " << clientid;
        return;
    }

    if (clientid != kdwClientID) {
        xpump_.Send(XMsgHandshakeResult::ToBuffer(knHandshakeResultFail, 0));
        RLOG() << base::Log::Format("0x%08lx ", this)
                << "wrong clientid: " << clientid;
        return;
    }

    if (serverfull_) {
        xpump_.Send(XMsgHandshakeResult::ToBuffer(knHandshakeResultServerFull,
                0));
        RLOG() << base::Log::Format("0x%08lx ", this) << "server is full.";
        return;
    }

    xpump_.Send(XMsgHandshakeResult::ToBuffer(knHandshakeResultSuccess, id_));
    SetState(ES_HANDSHAKESUCCESS);

    // Echos past this point will be acknowledged
    okecho_ = true;
}

void Endpoint::OnLogin(const char *username, const char *token, const char *did) {
    LOG() << "username: " << username << " token: " << token << " did: " << did;

    if (!CheckState(ES_HANDSHAKESUCCESS)) {
        xpump_.Send(XMsgLoginResult::ToBuffer(knLoginResultFail));
        return;
    }

    // Check to see this user is blocked. Might have did_.
    if (FindTracker(server_.tracker())) {
        xpump_.Send(XMsgLoginResult::ToBuffer(knLoginResultFail));
        return;
    }

    // Anonymous login always works, if that's what the user wants
    if (TokenAuth::IsAnonymous(username, token)) {
        RememberName(name_);
        delete name_;
        name_ = AllocString(base::Format::ToString("anon%d", id_));
        UpdateDid(did);
        xpump_.Send(XMsgLoginResult::ToBuffer(knLoginResultAnonymousSuccess));
        anonymous_ = true;
        SetState(ES_READY);
        return;
    }

    // Not anonymous; try real login
    dword result = TokenAuth::Authenticate(username, token);
    LOG() << AuthResults.Find(result) << ": " << username << ", " << token << ", " << did;
    if (result == knAuthResultFail) {
        RLOG() << "FAILED LOGIN: " << AuthResults.Find(result) << ": " << username << ", " << token << ", " << did;
        xpump_.Send(XMsgLoginResult::ToBuffer(knLoginResultFail));
        return;
    }
    if (result == knAuthResultStaleToken) {
        // Token hash is valid but timed out.
        UpdateDid(did);
        xpump_.Send(XMsgLoginResult::ToBuffer(knLoginResultStaleToken));
        return;
    }

    // Update the did first, before seeing if the user is blocked
    UpdateDid(did);
    if (FindTracker(server_.tracker())) {
        xpump_.Send(XMsgLoginResult::ToBuffer(knLoginResultFail));
        return;
    }

    // Success. Take in the new name, transition to ES_READY
    RememberName(name_);
    delete name_;
    name_ = AllocString(base::Format::ToString(username, id_));
    xpump_.Send(XMsgLoginResult::ToBuffer(knLoginResultSuccess));
    anonymous_ = false;
    SetState(ES_READY);

    // Moderators see unfiltered chat by default
    seechat_ = IsModerator();
}

void Endpoint::RememberName(const char *name) {
    if (name == NULL) {
        return;
    }
    old_names_.insert(old_names_.begin(), name);
    while (old_names_.size() >= 10) {
        old_names_.pop_back();
    }
}

void Endpoint::OnSignOut() {
    LOG();

    // Only know how to do this from ES_READY, currently
    if (!CheckState(ES_READY)) {
        xpump_.Send(XMsgSignOutResult::ToBuffer(knSignOutResultFail));
        return;
    }

    // Go back to before login state
    xpump_.Send(XMsgSignOutResult::ToBuffer(knSignOutResultSuccess));
    SetState(ES_HANDSHAKESUCCESS);
    anonymous_ = true;
}

void Endpoint::OnLobbyJoin() {
    if (!CheckState(ES_READY)) {
        xpump_.Send(XMsgLobbyJoinResult::ToBuffer(
                knLobbyJoinResultNotLoggedIn));
        return;
    }

    // See if this endpoint will be allowed to enter the lobby
    if (!server_.lobby().CanEnter(this)) {
        xpump_.Send(XMsgLobbyJoinResult::ToBuffer(knLobbyJoinResultFail));
    }

    // Message that entering the lobby was a success, before actually
    // entering the lobby, since that sends a bunch of messages.
    xpump_.Send(XMsgLobbyJoinResult::ToBuffer(knLobbyJoinResultSuccess));
    server_.lobby().Enter(this);
    SetState(ES_LOBBY);
}

void Endpoint::OnLobbyCreateRoom(const char *name, const char *password) {
    if (!CheckState(ES_LOBBY)) {
        xpump_.Send(XMsgLobbyCreateRoomResult::ToBuffer(
                knLobbyCreateRoomResultFail, 0));
        return;
    }

    // Allow room creation at a pre-determined rate only
    if (!IsModerator() && roomlimiter_.IsEmpty()) {
        xpump_.Send(XMsgLobbyCreateRoomResult::ToBuffer(
                knLobbyCreateRoomResultFail, 0));
        return;
    }

    dword result = knLobbyCreateRoomResultFail;
    Room *room = server_.lobby().NewRoom(this, name, password,
            kroomidInvalid, false, &result);
    if (room == NULL) {
        xpump_.Send(XMsgLobbyCreateRoomResult::ToBuffer(result, 0));
        return;
    }

    char ip[32];
    xpump_.socket()->GetRemoteAddress().IPAsString(ip, sizeof(ip));
    RLOG() << "NewRoom:" << name << " password:" << password
            << " username:" << name_ << " id:" << id_ << " ip:" << ip;

    xpump_.Send(XMsgLobbyCreateRoomResult::ToBuffer(result, room->id()));
}

void Endpoint::OnLobbyCanJoinRoom(dword roomid, const char *password) {
    if (!CheckState(ES_LOBBY)) {
        xpump_.Send(XMsgLobbyCanJoinRoomResult::ToBuffer(
                knRoomJoinResultFail));
        return;
    }

    Room *room = server_.lobby().FindRoom(roomid);
    if (room == NULL) {
        xpump_.Send(XMsgLobbyCanJoinRoomResult::ToBuffer(
                knRoomJoinResultNotFound));
        return;
    }
    dword result = room->CanAddPlayer(this, password);
    xpump_.Send(XMsgLobbyCanJoinRoomResult::ToBuffer(result));
}

void Endpoint::OnLobbyLeave() {
    if (!CheckState(ES_LOBBY)) {
        xpump_.Send(XMsgLobbyLeaveResult::ToBuffer(knLobbyLeaveResultFail));
        return;
    }

    server_.lobby().Leave(this);
    xpump_.Send(XMsgLobbyLeaveResult::ToBuffer(knLobbyLeaveResultSuccess));
    SetState(ES_READY);
}

void Endpoint::OnRoomJoin(dword roomid, const char *password) {
    if (!CheckState(ES_READY)) {
        xpump_.Send(XMsgRoomJoinResult::ToBuffer(knRoomJoinResultFail));
        return;
    }

    Room *room = server_.lobby().FindRoom(roomid);
    if (room == NULL) {
        xpump_.Send(XMsgRoomJoinResult::ToBuffer(knRoomJoinResultNotFound));
        return;
    }

    // See if this endpoint will be allowed to join the room
    dword result = room->CanAddPlayer(this, password);
    if (result != knRoomJoinResultSuccess) {
        xpump_.Send(XMsgRoomJoinResult::ToBuffer(result));
        return;
    }

    // Message that joining the room was a success, before actually
    // joining the room, since that sends a bunch of messages.
    xpump_.Send(XMsgRoomJoinResult::ToBuffer(knRoomJoinResultSuccess));

    // Remember the room and go into ROOM state. The room won't go away as
    // long as this player is in it, or in a game that is in it.
    roomid_ = room->id();
    SetState(ES_ROOM);

    // Add the player to the room. This will subscribe the endpoint to
    // room changes.
    room->AddPlayer(this);

    // Now tell the client that room status is complete, so it can track
    // changes in room state.
    xpump_.Send(XMsgRoomStatusComplete::ToBuffer());

#if 0
// This causes the chat button to flash immediately, not good

    // If this room is being logged, tell the user
    if (room->password()[0] == 0) {
        const char *pszMsg = "Chat is subject to being logged.";
        xpump_.Send(XMsgRoomReceiveChat::ToBuffer("", pszMsg));
        server_.logger().LogSystemMsg(this, pszMsg);
    }
#endif
}

void Endpoint::OnRoomSendChat(const char *chat) {
    if (!CheckState(ES_ROOM)) {
        return;
    }
    Room *room = server_.lobby().FindRoom(roomid_);
    if (room == NULL) {
        return;
    }
    std::string response;
    if (ProcessCommand(chat, &response)) {
        if (response.size() != 0) {
            xpump_.Send(XMsgRoomReceiveChat::ToBuffer("", response.c_str()));
            server_.logger().LogSystemMsg(this, response.c_str());
        }
        return;
    }
    int minutes_remaining;
    const char *pszMsg;
    switch (server_.chatlimiter().Submit(this, chat, &minutes_remaining)) {
    case knChatLimitResultNewlyLimited:
        room->SendChat(NULL, base::Format::ToString("%s has been muted for %d minute(s) due to chat spamming.", name_, minutes_remaining));
        break;

    case knChatLimitResultLimited:
        xpump_.Send(XMsgRoomReceiveChat::ToBuffer("",
                base::Format::ToString("Chat blocked. %d minute(s) remaining.",
                minutes_remaining)));
        break;
        
    case knChatLimitResultNotLimited:
        {
            if (!room->moderated()) {
                // Unmoderated rooms aren't filtered
                room->SendChat(this, chat, NULL);
            } else {
                // Moderated rooms are filtered
                const char *filtered;
                if (FilterChat(chat, &filtered)) {
                    room->SendChat(this, filtered, chat);
                } else {
                    room->SendChat(this, filtered, NULL);
                }
            }
        }
        break;
    }
}

bool Endpoint::FilterChat(const char *chat, const char **result) {
    // Users swear vertically, and horizontally. Handle both cases.
    // Maintain a chat fragment stream to handle multi-line chat swearing.

    // Add the newline, so that filtering can treat multiline appropriately.
    std::string fragment = chat_fragment_ + '\n' + chat;
    int cch_back = -1;
    const char *filtered_frag = server_.badwords().Filter(fragment.c_str(),
            &cch_back);

    const char *filtered_chat = filtered_frag + chat_fragment_.size() + 1;

    int cch_filtered = strlen(filtered_frag);
    if (cch_back <= 0) {
        chat_fragment_ = "";
    } else {
        // Take the previous char too, for proper standalone checking
        cch_back++;
        if (cch_back > cch_filtered) {
            cch_back = cch_filtered;
        }
        if (cch_back > CHAT_FRAGMENT_LENGTH) {
            cch_back = CHAT_FRAGMENT_LENGTH;
        }
        chat_fragment_ = std::string(fragment.c_str() +
                cch_filtered - cch_back);
    }

    *result = filtered_chat;
    return strcmp(chat, filtered_chat) != 0;
}

void Endpoint::OnRoomCreateGame(const GameParams& params) {
    Room *room = server_.lobby().FindRoom(roomid_);
    if (!CheckState(ES_ROOM) || room == NULL) {
        xpump_.Send(XMsgRoomCreateGameResult::ToBuffer(0,
                knRoomCreateGameResultFail, NULL));
        return;
    }

    // See if game params are valid
    if (!ValidateGameParams(params)) { 
        xpump_.Send(XMsgRoomCreateGameResult::ToBuffer(0,
                knRoomCreateGameResultFail, NULL));
        RLOG() << base::Log::Format("0x%08lx ", this)
                << "game params invalid";
#ifdef RELEASE_LOGGING
        LogGameParams(params);
#endif
        return;
    }

    // Find this level in the cache. Does it exist?
    PackId packidUpgrade;
    switch (server_.cache().FindInfo(params.packid, params.szLvlFilename,
            &info_, &packidUpgrade)) {
    case 0:
        // Don't know about this pack at all
        xpump_.Send(XMsgRoomCreateGameResult::ToBuffer(0,
                knRoomCreateGameResultUnknownMissionPack, NULL));
        RLOG() << base::Log::Format("0x%08lx ", this)
                << "packid not found";
#ifdef RELEASE_LOGGING
        LogGameParams(params);
#endif
        return;

    case 1:
        // Know about this pack, and have this version
        break;

    case 2:
        // Know about this pack, but client has wrong version
        xpump_.Send(XMsgRoomCreateGameResult::ToBuffer(0,
                knRoomCreateGameResultUpgradeMissionPack, &packidUpgrade));
        LOG() << base::Log::Format("0x%08lx ", this)
                << "client needs packid upgrade";
#ifdef LOGGING
        LogGameParams(params);
#endif
        return;
    }
    params_ = params;

    // Make sure the room can accept a new game
    if (!room->CanAddGame(this)) {
        xpump_.Send(XMsgRoomCreateGameResult::ToBuffer(0,
                knRoomCreateGameResultRoomFull, NULL));
        RLOG() << base::Log::Format("0x%08lx ", this)
                << "room full: " << room->name();
        return;
    }

    // Create a game. It'll be joined later. It'll time out and remove
    // itself if there are no joiners after awhile.
    Game *game = server_.NewGame(this, params_, info_, roomid_);
    if (game == NULL) {
        xpump_.Send(XMsgRoomCreateGameResult::ToBuffer(0,
                knRoomCreateGameResultFail, NULL));
        return;
    }

    // Add this game to the room
    room->AddGame(this, game);

    // Tell the client about the result.
    xpump_.Send(XMsgRoomCreateGameResult::ToBuffer(game->id(),
            knRoomCreateGameResultSuccess, NULL));
}

void Endpoint::OnRoomCanJoinGame(dword gameid) {
    Room *room = server_.lobby().FindRoom(roomid_);
    if (!CheckState(ES_ROOM) || room == NULL) {
        xpump_.Send(XMsgRoomCanJoinGameResult::ToBuffer(
                knGameJoinResultFail));
        return;
    }

    Game *game = room->FindGame(gameid);
    if (game == NULL) {
        xpump_.Send(XMsgRoomCanJoinGameResult::ToBuffer(
                knGameJoinResultGameNotFound));
        return;
    }

    dword result = game->CanAddPlayer(this);
    xpump_.Send(XMsgRoomCanJoinGameResult::ToBuffer(result));
}

void Endpoint::OnRoomLeave(dword hint) {
    Room *room = server_.lobby().FindRoom(roomid_);
    if (!CheckState(ES_ROOM) || room == NULL) {
        xpump_.Send(XMsgRoomLeaveResult::ToBuffer(knRoomLeaveResultFail));
        return;
    }

    room->SignalOnDelete.disconnect(this);
    room->RemovePlayer(this, hint);
    roomid_ = 0;

    xpump_.Send(XMsgRoomLeaveResult::ToBuffer(knRoomLeaveResultSuccess));
    SetState(ES_READY);
}

void Endpoint::OnGameJoin(dword gameid, dword roomid) {
    // Games can be joined after leaving a room, which
    // means ES_READY state.

    if (!CheckState(ES_READY)) {
        xpump_.Send(XMsgGameJoinResult::ToBuffer(knGameJoinResultFail));
        return;
    }

    Room *room = server_.lobby().FindRoom(roomid);
    if (room == NULL) {
        xpump_.Send(XMsgGameJoinResult::ToBuffer(
                knGameJoinResultRoomNotFound));
        return;
    }

    Game *game = room->FindGame(gameid);
    if (game == NULL) {
        xpump_.Send(XMsgGameJoinResult::ToBuffer(
                knGameJoinResultRoomNotFound));
        return;
    }
 
    dword result = game->CanAddPlayer(this);
    if (result != knGameJoinResultSuccess) {
        xpump_.Send(XMsgGameJoinResult::ToBuffer(result));
        return;
    }

    xpump_.Send(XMsgGameJoinResult::ToBuffer(knGameJoinResultSuccess));

    // Start logging with the game id
    xpump_.SetLogId(game->id());

    // This doesn't mean this player has "joined" the game. That
    // is a separate message / state.
    game_ = game;
    game_->SignalOnDelete.connect(this, &Endpoint::OnGameDelete);
    game->AddPlayer(this);

    SetState(ES_GAME);
}

std::string Endpoint::GetChatName() {
    if (anonymous_) {
        return name_;
    }
    if (!sigvisible_) {
        return name_;
    }

    // Use a character that is illegal in usernames, so people know these tags
    // are for real. The \xa0 and " characters are illegal in usernames, but
    // double quotes can be simulated with two single quotes, so use \xa0.
    if (IsAdmin()) {
        return std::string("admin\xa0 ") + name_;
    }
    if (IsModerator()) {
        return std::string("mod\xa0 ") + name_;
    }
    return name_;
}

bool Endpoint::IsModerator() {
    if (IsAdmin()) {
        return true;
    }
    return server_.IsModerator(name_);
}

bool Endpoint::IsAdmin() {
    // Admin is sticky across usernames so stealth is possible
    if (!admin_) {
        admin_ = server_.IsAdmin(name_);
    }
    return admin_;
}

bool Endpoint::GetArgument(const char *chat, int arg_index, std::string *arg,
        const char **rest) {
    const char *arg_end = chat;
    const char *arg_start = NULL;

    for (int index = -1; index != arg_index; index++) {
        // Scan past whitespace
        const char *pch;
        for (pch = arg_end; *pch != 0; pch++) {
            if (!isspace(*pch)) {
                break;
            }
        }
        arg_start = pch;

        // Scan past non-whitespace
        for (pch = arg_start; *pch != 0; pch++) {
            if (isspace(*pch)) {
                break;
            }
        }
        arg_end = pch;
    }

    if (arg_end <= arg_start) {
        return false;
    }
    *arg = std::string(arg_start, arg_end - arg_start);

    if (rest != NULL) {
        *rest = arg_end;
        while (isspace(**rest)) {
            (*rest)++;
        }
    }
    return true;
}

ModeratorCommand Endpoint::GetModeratorCommand(const char *chat) {
    std::string arg;
    if (!GetArgument(chat, 0, &arg)) {
        return kModeratorCommandNone;
    }
    if (arg.size() == 0 || arg[0] != '/') {
        return kModeratorCommandNone;
    }
    if (strcmp(arg.c_str(), "/mute") == 0) {
        return kModeratorCommandMute;
    }
    if (strcmp(arg.c_str(), "/unmute") == 0) {
        return kModeratorCommandUnmute;
    }
    if (strcmp(arg.c_str(), "/ids") == 0) {
        return kModeratorCommandIds;
    }
    if (strcmp(arg.c_str(), "/ban") == 0) {
        return kModeratorCommandBan;
    }
    if (strcmp(arg.c_str(), "/kick") == 0) {
        return kModeratorCommandKick;
    }
    if (strcmp(arg.c_str(), "/rooms") == 0) {
        return kModeratorCommandRooms;
    }
    if (strcmp(arg.c_str(), "/games") == 0) {
        return kModeratorCommandGames;
    }
    if (strcmp(arg.c_str(), "/clear") == 0) {
        return kModeratorCommandClear;
    }
    if (strcmp(arg.c_str(), "/sig") == 0) {
        return kModeratorCommandSig;
    }
    if (strcmp(arg.c_str(), "/filter") == 0) {
        return kModeratorCommandFilter;
    }
    if (strcmp(arg.c_str(), "/names") == 0) {
        return kModeratorCommandNames;
    }
    if (strcmp(arg.c_str(), "/rules") == 0) {
        return kModeratorCommandRules;
    }
    if (strcmp(arg.c_str(), "/help") == 0) {
        return kModeratorCommandHelp;
    }
    if (strcmp(arg.c_str(), "/see") == 0) {
        return kModeratorCommandSee;
    }
    if (strcmp(arg.c_str(), "/kill") == 0) {
        return kModeratorCommandKill;
    }
    if (strcmp(arg.c_str(), "/perm") == 0) {
        return kModeratorCommandPermanent;
    }
    if (strcmp(arg.c_str(), "/reg") == 0) {
        return kModeratorCommandRegisteredOnly;
    }
    if (strcmp(arg.c_str(), "/w") == 0) {
        return kModeratorCommandWhisper;
    }
    if (strcmp(arg.c_str(), "/m") == 0) {
        return kModeratorCommandMods;
    }
    if (strcmp(arg.c_str(), "/title") == 0) {
        return kModeratorCommandTitle;
    }
    if (strcmp(arg.c_str(), "/anon") == 0) {
        return kModeratorCommandAnonBlock;
    }
    if (strcmp(arg.c_str(), "/swap") == 0) {
        return kModeratorCommandSwap;
    }
    if (strcmp(arg.c_str(), "/flag") == 0) {
        return kModeratorCommandFlag;
    }
    return kModeratorCommandUnknown;
}

bool Endpoint::ProcessCommand(const char *chat, std::string *response) {
    int id = GetModeratorCommand(chat);
    if (id == kModeratorCommandNone) {
        if (muted_) {
            *response = "You previously muted yourself. To unmute and be able to chat again, type /unmute.";
            return true;
        }
        return false;
    }

    // A mod command has been entered
    server_.logger().LogModCommand(this, chat);

    // Default response
    *response = "Unknown command. /help for help.";

    // Command available to both mods / admins, and regular players
    switch (id) {
    case kModeratorCommandAnonBlock:
        if (state_ == ES_GAME && game_ != NULL &&
                game_->ToggleAnonBlock(this)) {
            if (game_->anonblock()) {
                *response = "You have blocked anons.";
            } else {
                *response = "You have unblocked anons.";
            }
        } else {
            *response = "This command is not allowed.";
        }
        return true;

    case kModeratorCommandSwap:
        // This will get processed by the game.
        return false;

    case kModeratorCommandFlag:
        // Write an entry into the log
        {
            std::string msg;
            GetArgument(chat, 1, &msg);
            server().logger().LogMark(this, msg.c_str());
            *response = "You wrote a mark in the log.";
        }
        return true;
    }

    if (!IsModerator()) {
        switch (id) {
        case kModeratorCommandMute:
            *response = "You have muted yourself. You will not receive chat, or be able to send chat. /unmute to unmute yourself.";
            muted_ = true;
            break;

        case kModeratorCommandUnmute:
            *response = "You have unmuted yourself.";
            muted_ = false;
            break;

        case kModeratorCommandHelp:
            if (state_ == ES_GAME && game_ != NULL) {
                bool anon = game_->IsAnonBlockAllowed(this);
                bool swap = game_->IsSwapAllowed(this);
                if (anon) {
                    if (swap) {
                        *response = "/mute, /unmute, /anon, /swap, /kick, /flag [msg], /help.";
                    } else {
                        *response = "/mute, /unmute, /anon, /kick, /flag [msg], /help.";
                    }
                } else if (swap) {
                    *response = "/mute, /unmute, /swap, /kick, /flag [msg], /help.";
                } else {
                    *response = "/mute, /unmute, /kick, /flag [msg], /help.";
                }
            } else {
                *response = "/mute, /unmute, /kick, /flag [msg], /help.";
            }
            break;

        case kModeratorCommandKick:
            *response = "Ouch that hurts.";
            break;
        }
        return true;
    }

    switch (id) {
    case kModeratorCommandNames:
    case kModeratorCommandBan:
    case kModeratorCommandKill:
    case kModeratorCommandFilter:
    case kModeratorCommandClear:
    case kModeratorCommandPermanent:
    case kModeratorCommandRegisteredOnly:
        if (!IsAdmin()) {
            *response = "You need to be an admin to use this command.";
            return true;
        }
    }

    RLOG() << "mod: " << name_ << " command: " << chat;
    if (state_ == ES_GAME && game_ != NULL) {
        RLOG() << "mod: " << name_ << " in game "
                << " server id: " << server_.id()
                << " server_start: " << server_.start_time()
                << " game id: " << game_->id();
    }

    // Several of the commands take an endpoint id
    Endpoint *endpoint = NULL;
    std::string arg;
    if (GetArgument(chat, 1, &arg)) {
        dword chatter_id = 0;
        base::Format::ToDword(arg.c_str(), 10, &chatter_id);
        endpoint = server_.GetEndpointFromChatterId(chatter_id);
    }

    char ip[32];
    memset(ip, 0, sizeof(ip));
    if (endpoint != NULL) {
        endpoint->xpump().socket()->GetRemoteAddress().IPAsString(ip,
                sizeof(ip));
    }

    switch (id) {
    case kModeratorCommandMute:
        if (endpoint != NULL) {
            // Check the room the moderator is in. This still allows for
            // remote action.
            if (!CanModerate(roomid())) {
                *response = "Can't moderate in this room.";
                break;
            }
            if (endpoint->IsModerator()) {
                *response = "You cannot mute another moderator.";
                break;
            }
            std::string minutes_str;
            int minutes = kcMinutesTimeoutDefault;
            if (GetArgument(chat, 2, &minutes_str)) {
                base::Format::ToInteger(minutes_str.c_str(), 10, &minutes);
                if (minutes < 0) {
                    minutes = kcMinutesTimeoutDefault;
                }
                if (minutes > kcMinutesTimeoutMaximum) {
                    minutes = kcMinutesTimeoutMaximum;
                }
            }
            RLOG() << "mod: " << name_ << " muted: " << endpoint->name()
                    << " minutes: " << minutes << " ip address: " << ip;
            server_.chatlimiter().Mute(endpoint, minutes);
            *response = base::Format::ToString("%s has been muted for %d minute(s). Action logged.", endpoint->name(), minutes);
        } else {
            *response = "Could not find player using that id.";
        }
        break;

     case kModeratorCommandUnmute:
        if (endpoint != NULL) {
            // Check the room the moderator is in. This still allows for
            // remote action.
            if (!CanModerate(roomid())) {
                *response = "Can't moderate in this room.";
                break;
            }
            RLOG() << "mod: " << name_ << " unmuted: " << endpoint->name()
                    << " ip address: " << ip;
            server_.chatlimiter().Mute(endpoint, 0);
            *response = base::Format::ToString("%s has been unmuted. Action logged.", endpoint->name());
        } else {
            *response = "Could not find player using that id.";
        }
        break;
      
    case kModeratorCommandBan:
        if (endpoint != NULL) {
            // Check the room the moderator is in. This still allows for
            // remote action.
            if (!CanModerate(roomid())) {
                *response = "Can't moderate in this room.";
                break;
            }
            std::string minutes_str;
            int minutes = kcMinutesBanTimeoutDefault;
            if (endpoint->GetArgument(chat, 2, &minutes_str)) {
                base::Format::ToInteger(minutes_str.c_str(), 10, &minutes);
                if (minutes < 0) {
                    minutes = kcMinutesBanTimeoutDefault;
                }
                if (minutes > kcMinutesBanTimeoutMaximum) {
                    minutes = kcMinutesBanTimeoutMaximum;
                }
            }
            RLOG() << "mod: " << name_ << " banned: "
                    << endpoint->name() << " minutes: " << minutes
                    << " ip address: " << ip;
            long64 tExpires = base::GetMillisecondCount() +
                minutes * 60 * 1000;
            endpoint->AddTracker(server_.tracker(), tExpires);
            *response = base::Format::ToString("%s has been banned from this server for %d minute(s). Action logged.", endpoint->name(), minutes);
            endpoint->Dispose();
        } else {
            *response = "Could not find player using that id.";
        }
        break;

    case kModeratorCommandRooms:
        {
            std::vector<dword> roomids = server_.lobby().GetRoomIds();
            std::vector<dword>::iterator it = roomids.begin();
            for (; it != roomids.end(); it++) {
                Room *room = server_.lobby().FindRoom(*it);
                if (room != NULL) {
                    std::ostringstream s;
                    s << "room id:" << (*it) << " name:" << room->name()
                            << " creator:" << room->creator()
                            << " id:" << room->creator_id();
                    if (IsAdmin()) {
                        s << " password:" << room->password()
                                << " ip:" << room->creator_ip();
                    }
                    if (state_ == ES_GAME) {
                        xpump_.Send(XMsgGameReceiveChat::ToBuffer("", s.str().c_str()));
                    }
                    if (state_ == ES_ROOM) {
                        xpump_.Send(XMsgRoomReceiveChat::ToBuffer("", s.str().c_str()));
                    }
                    server_.logger().LogSystemMsg(this, s.str().c_str());
                    RLOG() << s.str();
                }
            }
            *response = "";
        }
        break;

    case kModeratorCommandKill:
        // Kill a room
        {
            Room *room = NULL;
            std::string roomid_str;
            if (!GetArgument(chat, 1, &roomid_str)) {
                *response = "Room id not found.";
                return true;
            }
            dword roomid = 0;
            base::Format::ToDword(roomid_str.c_str(), 10, &roomid);
            room = server_.lobby().FindRoom(roomid);
            if (room == NULL) {
                *response = "Room not found.";
                return true;
            }
            if (room->Kill()) {
                *response = "Room killed. Will be removed within 30 seconds.";
            } else {
                *response = "Room not killed.";
            }
        }
        break;

    case kModeratorCommandGames:
        {
            Room *room = NULL;
            std::string roomid_str;
            if (!GetArgument(chat, 1, &roomid_str)) {
                *response = "Room id not found.";
                return true;
            }
            dword roomid = 0;
            base::Format::ToDword(roomid_str.c_str(), 10, &roomid);
            room = server_.lobby().FindRoom(roomid);
            if (room == NULL) {
                *response = "Room not found.";
                return true;
            }
            std::vector<dword> gameids = room->GetGameIds();
            std::vector<dword>::iterator it = gameids.begin();
            for (; it != gameids.end(); it++) {
                Game *game = room->FindGame(*it);
                if (game != NULL) {
                    std::ostringstream s;
                    s << "game id:" << (*it) << " creator:" << game->creator()
                            << " title:" << game->info().title();
                    if (state_ == ES_GAME) {
                        xpump_.Send(XMsgGameReceiveChat::ToBuffer("",
                                s.str().c_str()));
                    }
                    if (state_ == ES_ROOM) {
                        xpump_.Send(XMsgRoomReceiveChat::ToBuffer("",
                                s.str().c_str()));
                    }
                    server_.logger().LogSystemMsg(this, s.str().c_str());
                    RLOG() << s.str();
                }
            }
            *response = "";
        }
        break;

    case kModeratorCommandPermanent:
        {
            Room *room = NULL;
            std::string roomid_str;
            if (!GetArgument(chat, 1, &roomid_str)) {
                *response = "Room id not found.";
                return true;
            }
            dword roomid = 0;
            base::Format::ToDword(roomid_str.c_str(), 10, &roomid);
            room = server_.lobby().FindRoom(roomid);
            if (room == NULL) {
                *response = "Room not found.";
                return true;
            }
            bool result;
            if (!room->TogglePermanent(&result)) {
                *response = "Could not toggle permanent status.";
                return true;
            }
            if (result) {
                *response = "Room is now marked permanent.";
            } else {
                *response = "Room is now marked non-permanent.";
            }
        }
        break;

    case kModeratorCommandRegisteredOnly:
        {
            Room *room = NULL;
            std::string roomid_str;
            if (!GetArgument(chat, 1, &roomid_str)) {
                *response = "Room id not found.";
                return true;
            }
            dword roomid = 0;
            base::Format::ToDword(roomid_str.c_str(), 10, &roomid);
            room = server_.lobby().FindRoom(roomid);
            if (room == NULL) {
                *response = "Room not found.";
                return true;
            }
            bool result;
            if (!room->ToggleRegistered(&result)) {
                *response = "Could not toggle registered status.";
                return true;
            }
            if (result) {
                *response = "Room now reg only.";
            } else {
                *response = "Room now open to all.";
            }
        }
        break;

    case kModeratorCommandIds:
        {
            Room *room = server_.lobby().FindRoom(roomid_);
            Game *game = game_;

            bool lobby = false;
            dword roomid = roomid_;
            std::string roomid_str;
            if (GetArgument(chat, 1, &roomid_str)) {
                if (strcmp(roomid_str.c_str(), "lobby") == 0) {
                    lobby = true;
                } else {
                    base::Format::ToDword(roomid_str.c_str(), 10, &roomid);
                    room = server_.lobby().FindRoom(roomid);
                    if (room == NULL) {
                        *response = "Room not found.";
                        return true;
                    }

                    game = NULL;
                    std::string gameid_str;
                    if (GetArgument(chat, 2, &gameid_str)) {
                        dword gameid = 0;
                        base::Format::ToDword(gameid_str.c_str(), 10, &gameid);
                        game = room->FindGame(gameid);
                        if (game == NULL) {
                            *response = "Game not found.";
                            return true;
                        }
                    }
                }
            }

            std::vector<std::string> responses;
            if (lobby) {
                responses = server_.lobby().GetIdsString(this);
            } else {
                if (game != NULL) {
                    responses = game->GetIdsString(this);
                } else if (room != NULL) {
                    responses = room->GetIdsString(this);
                }
            }

            std::vector<std::string>::iterator it = responses.begin();
            for (; it != responses.end(); it++) {
                RLOG() << "mod " << name_ << ": " << *it;
                if (state_ == ES_GAME) {
                    xpump_.Send(XMsgGameReceiveChat::ToBuffer("",
                            (*it).c_str()));
                }
                if (state_ == ES_ROOM) {
                    xpump_.Send(XMsgRoomReceiveChat::ToBuffer("",
                            (*it).c_str()));
                }
                server_.logger().LogSystemMsg(this, (*it).c_str());
            }
        }
        *response = "";
        break;

    case kModeratorCommandClear:
        {
            server_.lobby().room_tracker().Clear();
            server_.chatlimiter().tracker().Clear();
            server_.tracker().Clear();
            std::vector<dword> roomids = server_.lobby().GetRoomIds();
            std::vector<dword>::iterator it = roomids.begin();
            for (; it != roomids.end(); it++) {
                Room *room = server_.lobby().FindRoom(*it);
                if (room != NULL) {
                    room->tracker().Clear();
                }
            }
            *response = "Cleared.";
        }
        break;

    case kModeratorCommandSig:
        sigvisible_ ^= true;
        *response = GetChatName();
        break;

    case kModeratorCommandSee:
        seechat_ ^= true;
        if (seechat_) {
            *response = "You will see unfiltered chat.";
        } else {
            *response = "You will no longer see unfiltered chat.";
        }
        break;

    case kModeratorCommandFilter:
        server_.badwords().toggle();
        if (server_.badwords().on()) {
            *response = "Word filter is now on.";
        } else {
            *response = "Word filter is now off.";
        }
        break;

    case kModeratorCommandNames:
        if (endpoint != NULL) {
            if (endpoint->old_names().size() == 0) {
                *response = base::Format::ToString("No other names for %s.",
                        endpoint->name());
                return true;
            }
            std::vector<std::string>::const_iterator it =
                    endpoint->old_names().begin();
            for (; it != endpoint->old_names().end(); it++) {
                const char *s = base::Format::ToString("%s = %s",
                        endpoint->name(), (*it).c_str());
                if (state_ == ES_GAME) {
                    xpump_.Send(XMsgGameReceiveChat::ToBuffer("", s));
                }
                if (state_ == ES_ROOM) {
                    xpump_.Send(XMsgRoomReceiveChat::ToBuffer("", s));
                }
                server_.logger().LogSystemMsg(this, s);
                RLOG() << s;
            }
            *response = "";
        } else {
            *response = "Could not find player using that id.";
        }
        break;

    case kModeratorCommandMods:
        // Broadcast chat to all mods in every room / game
        {
            std::string dummy;
            const char *rest;
            if (GetArgument(chat, 0, &dummy, &rest) && *rest != 0) {
                const char *s = base::Format::ToString("%s to mods: %s",
                        name_, rest);
                server_.lobby().SendAdminChat("", s, true);
            }
        }
        *response = "";
        break;

    case kModeratorCommandWhisper:
        // Send to specific user id
        if (endpoint == NULL) {
            *response = "No id or invalid id.";
            return true;
        } else {
            std::string dummy;
            const char *rest;
            if (GetArgument(chat, 1, &dummy, &rest) && *rest != 0) {
                const char *s = base::Format::ToString("%s to %s: %s",
                        name_, endpoint->name(), rest);
                if (endpoint->state_ == ES_GAME) {
                    endpoint->xpump().Send(
                            XMsgGameReceiveChat::ToBuffer("", s));
                }
                if (endpoint->state_ == ES_ROOM) {
                    endpoint->xpump().Send(
                            XMsgRoomReceiveChat::ToBuffer("", s));
                }
                server_.logger().LogSystemMsg(this, s);
                if (endpoint != this) {
                    *response = s;
                    return true;
                }
            }
            *response = "";
        }
        break;

    case kModeratorCommandTitle:
        {
            const char *name;
            std::string roomid_str;
            if (!GetArgument(chat, 1, &roomid_str, &name)) {
                *response = "No room id specified.";
                return true;
            }

            dword roomid = 0;
            base::Format::ToDword(roomid_str.c_str(), 10, &roomid);
            Room *room = server_.lobby().FindRoom(roomid);
            if (room == NULL) {
                *response = "Room not found.";
                return true;
            }

            if (!room->SetName(server_.badwords().Filter(name))) {
                *response = "Failed setting room title.";
                return true;
            }
            *response = "Success";
        }
        break;

    case kModeratorCommandHelp:
        if (IsAdmin()) {
            if (state_ == ES_GAME) {
                *response = "/ids [lobby] [roomid] [gameid], /mute <id> [minutes], /unmute <id>, /ban <id> [minutes], /rooms, /kill <roomid>, /games <roomid>, /names <id>, /w <id>, /m, /title <roomid>, /clear, /filter, /sig, /see, /perm <roomid>, /reg <roomid>, /swap, /anon, /rules, /flag [msg], /help.";
            } else {
                *response = "/ids [lobby] [roomid] [gameid], /mute <id> [minutes], /unmute <id>, /kick <id> [minutes], /ban <id> [minutes], /rooms, /kill <roomid>, /games <roomid>, /names <id>, /w <id>, /m, /title <roomid>, /clear, /filter, /sig, /see, /perm <roomid>, /reg <roomid>, /rules, /flag [msg], /help.";
            }
        } else {
            if (state_ == ES_GAME) {
                *response = "/ids [lobby] [roomid] [gameid], /mute <id> [minutes], /unmute <id>, /rooms, /games <roomid>, /w <id>, /m, /title <roomid>, /sig, /see, /swap, /anon, /rules, /flag [msg], /help.";
            } else {
                *response = "/ids [lobby] [roomid] [gameid], /mute <id> [minutes], /unmute <id>, /kick <id> [minutes], /rooms, /games <roomid>, /w <id>, /m, /title <roomid>, /sig, /see, /rules, /flag [msg], /help.";
            }
        }
        break;

    case kModeratorCommandKick:
    case kModeratorCommandRules:
        // These command gets processed by the room, or game, since the
        // enumeration is different, so pass it through.
        return false;

    default:
        break;
    }

    return true;
}

void Endpoint::OnGameSendChat(const char *chat) {
    if (!CheckState(ES_GAME)) {
        return;
    }
    if (game_ == NULL) {
        return;
    }
    std::string response;
    if (ProcessCommand(chat, &response)) {
        if (response.size() != 0) {
            xpump_.Send(XMsgGameReceiveChat::ToBuffer("", response.c_str()));
            server_.logger().LogSystemMsg(this, response.c_str());
        }
        return;
    }
    int minutes_remaining;
    switch (server_.chatlimiter().Submit(this, chat, &minutes_remaining)) {
    case knChatLimitResultNewlyLimited:
        game_->SendChat(NULL, base::Format::ToString("%s has been muted for %d minute(s) due to chat spamming.", name_, minutes_remaining));
        break;

    case knChatLimitResultLimited:
        xpump_.Send(XMsgGameReceiveChat::ToBuffer("",
                base::Format::ToString("Chat blocked. %d minute(s) remaining.",
                minutes_remaining)));
        break;
        
    case knChatLimitResultNotLimited:
        {
            Room *room = server_.lobby().FindRoom(roomid_);
            if (room != NULL && !room->moderated()) {
                // Unmoderated rooms aren't filtered
                game_->SendChat(this, chat, NULL);
            } else {
                // Moderated rooms are filtered
                const char *filtered;
                if (FilterChat(chat, &filtered)) {
                    game_->SendChat(this, filtered, chat);
                } else {
                    game_->SendChat(this, filtered, NULL);
                }
            }
        }
        break;
    }
}

void Endpoint::OnGameLeave() {
    // Check game_ for NULL first, before state_ is checked,
    // since the server can force game_ to NULL legally.
    if (game_ == NULL) {
        xpump_.Send(XMsgGameLeaveResult::ToBuffer(
                knGameLeaveResultNotFound));
        LOG() << base::Log::Format("0x%08lx ", this)
                << "No game to disconnect from. "
                << "Can happen when the server disconnects first";
        return;
    }

    if (!CheckState(ES_GAME)) {
        xpump_.Send(XMsgGameLeaveResult::ToBuffer(
                knGameLeaveResultFail));
        LOG() << base::Log::Format("0x%08lx ", this)
                << "Not in ES_GAME state.";
        return;
    }

    game_->SignalOnDelete.disconnect(this);
    game_->RemovePlayer(this, knDisconnectReasonLeftGame);
    game_ = NULL;

    // Stop logging with the game id
    xpump_.SetLogId(0);

    xpump_.Send(XMsgGameLeaveResult::ToBuffer(knGameLeaveResultSuccess));

    // Go back to ES_READY. From here the client will most likely rejoin
    // the last room it was in.
    SetState(ES_READY);
}

void Endpoint::OnGameNetMessage(NetMessage **ppnm) {
    // Check game_ for NULL first, before state_ is checked,
    // since the server can force game_ to NULL legally.
    if (game_ == NULL) {
        LOG() << base::Log::Format("0x%08lx ", this)
                << "No game for NetMessage. "
                << "Can happen when the server disconnects first";
        return;
    }

    if (!CheckState(ES_GAME)) {
        LOG() << base::Log::Format("0x%08lx ", this) << "Not in ES_GAME!";
        return;
    }
    game_->OnNetMessage(this, *ppnm);
}

void Endpoint::OnGameDelete(Game *game) {
    LOG() << "game: " << game->info().title() << " created by: " <<
            game->creator();
    DropGame(game);
}

void Endpoint::DropGame(Game *game, int reason) {
    LOG() << base::Log::Format("0x%08lx ", this)
            << "Dropping game, reason: " << reason;

    Assert(game == game_);
    if (game == NULL || game != game_) {
        return;
    }
    game_->RemovePlayer(this, reason);
    game_->SignalOnDelete.disconnect(this);
    xpump_.Send(XMsgGameKilled::ToBuffer(game_->id()));
    game_ = NULL;
    SetState(ES_READY);
}

void Endpoint::OnError(int error) {
    LOG() << base::Log::Format("0x%08lx ", this) << error;
    Dispose();
}

void Endpoint::OnClose(int error) {
    LOG() << base::Log::Format("0x%08lx ", this) << error;
    Dispose();
}

void Endpoint::OnCloseOk() {
    LOG() << base::Log::Format("0x%08lx ", this);
    Dispose();
}

void Endpoint::OnHeartbeat() {
    // If a game is playing, there is a game timer to monitor clients
    if (game_ != NULL && game_->playing()) {
        return;
    }

    if (!echo_) {
        LOG() << base::Log::Format("0x%08lx ", this)
                << "client ping timeout";
#ifndef DEV_BUILD
        // When a user brings up audio controls (double click home), or
        // gets a phone call, the iPhone OS freezes the underlying
        // application, so it won't be returning echoes. Make it miss a
        // few before killing it, since these cases happen.
        missed_++;
        if (missed_ == MISSED_ECHO_COUNT) {
            Dispose();
            return;
        }
#endif
    }
    xpump_.Send(XMsgEcho::ToBuffer());
    echo_ = false;
}

void Endpoint::OnEcho() {
    // Don't acknowledge echos until it is ok
    if (!okecho_) {
        return;
    }

    // Echo received from client
    echo_ = true;   
    missed_ = 0;
}

void Endpoint::UpdateDid(const char *did) {
    // The most accurate one is the first one seen
    if (did_[0] == 0) {
        strncpyz(did_, did, sizeof(did_));
    }
}

void Endpoint::AddTracker(Tracker& tracker, long64 tExpires) {
    const base::SocketAddress remote = xpump().socket()->GetRemoteAddress();
    char szT[64];
    remote.IPToString(remote.ip(), szT, sizeof(szT));
    tracker.Add(szT, tExpires);
    if (did_[0] != 0) {
        tracker.Add(did_, tExpires);
    }
}

void Endpoint::RemoveTracker(Tracker& tracker) {
    const base::SocketAddress remote = xpump().socket()->GetRemoteAddress();
    char szT[64];
    remote.IPToString(remote.ip(), szT, sizeof(szT));
    tracker.Remove(szT);
    if (did_[0] != 0) {
        tracker.Remove(did_);
    }
}

bool Endpoint::FindTracker(Tracker& tracker, long64 *tExpires) {
    const base::SocketAddress remote = xpump().socket()->GetRemoteAddress();
    char szT[64];
    remote.IPToString(remote.ip(), szT, sizeof(szT));
    if (tracker.Find(szT, tExpires)) {
        return true;
    }
    if (did_[0] != 0) {
        if (tracker.Find(did_, tExpires)) {
            return true;
        }
    }
    return false;
}

bool Endpoint::CanModerate(dword roomid) {
    if (IsAdmin()) {
        return true;
    }

    Room *room = server_.lobby().FindRoom(roomid);
    if (room == NULL) {
        return false;
    }

    return room->moderated();
}

std::string Endpoint::GetRoomName()
{
    Room *room = server_.lobby().FindRoom(roomid_);
    if (room == NULL) {
        return "";
    }
    return room->name();
}

std::string Endpoint::GetGameName()
{
    if (game_ == NULL) {
        return "";
    }
    return game_->info().title();
}

dword Endpoint::gameid() {
    return game_ == NULL ? 0 : game_->id();
}

} // namespace wi
