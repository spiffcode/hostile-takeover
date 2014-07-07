#include "game/ht.h"
#include "game/gameform.h"
#include "game/creategameform.h"
#include "mpshared/messages.h"

namespace wi {
    
RoomForm::RoomForm(LoginHandler& handler, const RoomInfo& roominfo,
        Chatter& chatter) : handler_(handler), roominfo_(roominfo),
        chatter_(chatter), refresh_(true), creator_(false), status_(true) {
    chatter_.SetChatTitle("Room Chat");
    chatter_.SignalOnBlink.connect(this, &RoomForm::OnChatBlink);
    chatter_.SignalOnPlayers.connect(this, &RoomForm::OnPlayers);
}

RoomForm::~RoomForm() {
    chatter_.SignalOnBlink.disconnect(this);
    chatter_.SignalOnPlayers.disconnect(this);
    chatter_.HideChat();
}

void RoomForm::ShowJoinMessage(dword result) {
    char *message = NULL;
    switch (result) {
    case knRoomJoinResultFail:
    default:
        message = "Could not join room.";
        break;

    case knRoomJoinResultFull:
        message = "The room is full. Try again later.";
        break;

    case knRoomJoinResultNotFound:
        message = "Can't find this room. Try another room.";
        break;

    case knRoomJoinResultWrongPassword:
        message = "Incorrect password. Try again.";
        break;

    case knRoomJoinResultSuccess:
        break;
    }
    if (message != NULL) {
        HtMessageBox(kfMbWhiteBorder, "Join Room", message);
    }
}

dword RoomForm::DoForm(LoginHandler& handler, const RoomInfo& roominfo,
        Chatter& chatter, GameInfo *joininfo) {

    if (gptra == NULL) {
        return knRoomResultFail;
    }
    RoomForm *pfrm = (RoomForm *)gpmfrmm->LoadForm(gpiniForms, kidfRoom,
            new RoomForm(handler, roominfo, chatter));
    if (pfrm == NULL) {
        return knRoomResultFail;
    }

    int result = 0;
    pfrm->DoModal(&result);
    *joininfo = pfrm->joininfo();
    bool creator = pfrm->creator();
    delete pfrm;

    // Map result codes
    if (result == kidcCancel) {
        return knRoomResultDone;
    }
    if (result == kidcJoinGame) {
        if (creator) {
            return knRoomResultCreated;
        } else {
            return knRoomResultJoin;
        }
    }
    return knRoomResultFail;
}

bool RoomForm::DoModal(int *presult) {
    // Initialize controls
    LabelControl *plbl = (LabelControl *)GetControlPtr(kidcRoomName);
    plbl->SetText(roominfo_.name.c_str());

    // Turn on ellipsis mode in list control
    ListControl *plstc = (ListControl *)GetControlPtr(kidcGameList);
    plstc->SetTabFlags(kfLstTabEllipsis);
   
    // Hide Join Game until a game shows up
    GetControlPtr(kidcJoinGame)->Show(false);

    // Hide the list, and show the message, until a game shows up
    plstc->Show(false);
    GetControlPtr(kidcNoGames)->Show(true);

    // Add this player to the list
    char name[kcbPlayerName];
    handler_.GetPlayerName(name, sizeof(name));
    OnAddPlayer(name);

    Show(true);
    gptra->SetCallback(this);
    dword result = gptra->JoinRoom(roominfo_.roomid,
            roominfo_.password.c_str(), this);
    if (result != knRoomJoinResultSuccess) {
        gptra->SetCallback(NULL);
        ShowJoinMessage(result);
        *presult = 0;
        return false;
    }
    *presult = 0;
    bool success = ShellForm::DoModal(presult);
    if (*presult == kidcJoinGame) {
        gptra->LeaveRoom(knLeaveRoomHintJoinGame);
    } else {
        gptra->LeaveRoom(knLeaveRoomHintNone);
    }
    gptra->SetCallback(NULL);

    return success;
}

void RoomForm::OnChatBlink(bool on) {
    GetControlPtr(kidcChat)->Show(on);
}

void RoomForm::OnPlayers() {
    std::string s = GetPlayersString();
    chatter_.AddChat("", s.c_str(), true);
}

void RoomForm::OnControlSelected(word idc) {
    switch (idc) {
    case kidcNewGame:
        OnCreateGame();
        break;

    case kidcJoinGame:
        OnJoinGame();
        break;

    case kidcCancel:
        EndForm(idc);
        break;

    case kidcChat:
        chatter_.ShowChat();
        break;
    }
}

void RoomForm::OnControlNotify(word idc, int nNotify) {
    if (idc == kidcGameList && nNotify == knNotifySelectionChange) {
        HideShowJoinGame();
    }
    ShellForm::OnControlNotify(idc, nNotify);
}

void RoomForm::OnJoinGame() {
    ListControl *plstc = (ListControl *)GetControlPtr(kidcGameList);
    dword gameid = (dword)plstc->GetSelectedItemData();
    GameMap::iterator it = map_.find(gameid);
    if (it == map_.end()) {
        return;
    }
    GameInfo& info = it->second;
    if (!AskInstallMissionPack(&info.params.packid, "Join Game", info.title)) {
        return;
    }
    InitiateJoinGame(info);
}

void RoomForm::OnCreateGame() {
    GameParams params;
    if (!CreateGameForm::DoForm(handler_, NULL, chatter_, &params)) {
        return;
    }

    while (true) {
        dword gameid;
        PackId packidUpgrade;
        dword result = gptra->CreateGame(&params, &packidUpgrade, &gameid);
        switch (result) {
        case knRoomCreateGameResultSuccess:
            {
                // Update teh list and select the game, in case the join
                // fails
                creator_ = true;
                ListControl *plstc = (ListControl *)GetControlPtr(kidcGameList);
                refresh_ = false;
                plstc->Select(FindIndex(gameid), true, true);
                refresh_ = true;

                // OnAddGame has been called already. Join the game
                GameMap::iterator it = map_.find(gameid);
                if (it != map_.end()) {
                    InitiateJoinGame(it->second);
                }
            }
            break;

        case knRoomCreateGameResultFail:
            HtMessageBox(kfMbWhiteBorder, "Create Game",
                "Unable to create the game.");
            break;

        case knRoomCreateGameResultUnknownMissionPack:
            HtMessageBox(kfMbWhiteBorder, "Create Game",
                 "Unknown mission pack!");
            break;

        case knRoomCreateGameResultUpgradeMissionPack:
            if (AskInstallMissionPack(&packidUpgrade, "Advertise Game",
                    NULL)) {
                params.packid = packidUpgrade;
                continue;
            }
            break;

        case knRoomCreateGameResultRoomFull:
            HtMessageBox(kfMbWhiteBorder, "Create Game",
                    "Room full. Cannot create another game in this room.");
            break;
        }
        break;
    }
}

int RoomForm::FindIndex(dword gameid) {
    int index = 0;
    GameMap::iterator it = map_.begin();
    for (; it != map_.end(); it++) {
        if (it->first == gameid) {
            return index;
        }
        index++;
    }
    return -1;
}

void RoomForm::InitiateJoinGame(const GameInfo& info) {
    if (gptra == NULL) {
        return;
    }

    // Before joining, check to see if this game is joinable.
    // Joining also checks, but this is a way to provide the user
    // with feedback before exiting this form.

    dword result = gptra->CanJoinGame(info.gameid);
    GameForm::ShowJoinMessage(result);
    if (result == knGameJoinResultSuccess) {
        joininfo_ = info;
        EndForm(kidcJoinGame);
    }
}

void RoomForm::OnAddGame(const char *player, dword gameid,
        const GameParams& params, dword minplayers, dword maxplayers,
        const char *title, dword ctotal) {
    GameMap::iterator it = map_.find(gameid);
    if (it != map_.end()) {
        return;
    }
    GameInfo *info = new GameInfo;
    if (info == NULL) {
        return;
    }
    memset(info, 0, sizeof(*info));
    info->roomid = roominfo_.roomid;
    info->gameid = gameid;
    info->params = params;
    info->minplayers = minplayers;
    info->maxplayers = maxplayers;
    strncpyz(info->title, title, sizeof(info->title));
    strncpyz(info->creator, player, sizeof(info->creator));
    map_.insert(GameMap::value_type(gameid, *info));
    Refresh();

    if (!status_) {
        chatter_.AddChat(player, base::Format::ToString(
                "%s created and joined game %s.", player,
                info->GetDescription()), true);
    }
}

void RoomForm::OnRemoveGame(dword gameid, dword ctotal) {
    GameMap::iterator it = map_.find(gameid);
    if (it == map_.end()) {
        return;
    }
    map_.erase(gameid);
    Refresh();    
}

void RoomForm::OnGameInProgress(dword gameid) {
    GameMap::iterator it = map_.find(gameid);
    if (it == map_.end()) {
        return;
    }
    it->second.playing = true;
    Refresh();

    if (!status_) {
        chatter_.AddChat("", base::Format::ToString(
                "%s's game %s has started.",
                it->second.creator, it->second.GetDescription()), true);
    }
}

void RoomForm::OnGamePlayerNames(dword gameid, dword cnames,
        const PlayerName *anames) {
    GameMap::iterator it = map_.find(gameid);
    if (it == map_.end()) {
        return;
    }
    if (cnames > ARRAYSIZE(it->second.anames)) {
        return;
    }
    GameInfo& info = it->second;

    // Find the players being added to the game
    int cadded = 0;
    PlayerName added[kcPlayersMax];
    for (int i = 0; i < cnames; i++) {
        bool found = false;
        for (int j = 0; j < info.cnames; j++) {
            if (strcmp(anames[i].szPlayerName,
                    info.anames[j].szPlayerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            strncpyz(added[cadded].szPlayerName, anames[i].szPlayerName,
                    sizeof(added[cadded].szPlayerName));
            cadded++;
        }
    }
    
    // Find the players being removed from the game
    int cremoved = 0;
    PlayerName removed[kcPlayersMax];
    for (int j = 0; j < info.cnames; j++) {
        bool found = false;
        for (int i = 0; i < cnames; i++) {
            if (strcmp(anames[i].szPlayerName,
                    info.anames[j].szPlayerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            strncpyz(removed[cremoved].szPlayerName,
                    info.anames[j].szPlayerName,
                    sizeof(removed[cremoved].szPlayerName));
            cremoved++;
        }
    }
    
    // Update the players in the game       
    memcpy(info.anames, anames, cnames * sizeof(PlayerName));
    info.cnames = cnames;
    Refresh();

    // Callbacks before Join has returned are current state, not changes
    // in state.
    if (status_) {
        return;
    }

    // Add to the chat window
    for (int i = 0; i < cadded; i++) {
        // Don't post if this player created the game
        if (strcmp(added[i].szPlayerName, info.creator) == 0) {
            continue;
        }

        // Format: X joined Y's game Desert River Riches
        chatter_.AddChat(added[i].szPlayerName,
                base::Format::ToString("%s joined %s's game %s",
                added[i].szPlayerName, info.creator, info.title), true);
    }

    for (int i = 0; i < cremoved; i++) {
        if (strcmp(removed[i].szPlayerName, info.creator) == 0) {
            if (!info.playing) {
                 // Format: X cancelled Desert River Riches.
                chatter_.AddChat(removed[i].szPlayerName,
                        base::Format::ToString("%s cancelled %s.",
                        removed[i].szPlayerName, info.title), true);
            } else {
                // Format: X left Desert River Riches.
                chatter_.AddChat(removed[i].szPlayerName,
                        base::Format::ToString("%s left %s.",
                        removed[i].szPlayerName, info.title), true);
            }
        } else {
            if (!info.playing) {
                // Format: X left Y's game Desert River Riches before it
                // started.
                chatter_.AddChat(removed[i].szPlayerName,
                        base::Format::ToString(
                        "%s left %s's game %s before it started.",
                        removed[i].szPlayerName, info.creator, info.title),
                        true);
            } else {
                // Format: X left Y's game Desert River Riches.
                chatter_.AddChat(removed[i].szPlayerName,
                        base::Format::ToString("%s left %s's game %s.",
                        removed[i].szPlayerName, info.creator, info.title),
                        true);
            }
        }
    }
}

void RoomForm::Refresh(int ct) {
    if (!refresh_) {
        return;
    }
    if (ct == -1) {
        timer_.Stop();
        OnTimeout(0);
        return;
    }
    if (ct == 0) {
        timer_.Stop();
    }
    if (timer_.IsStarted()) {
        return;
    }
    timer_.Start(this, ct * 10);
}

void RoomForm::OnTimeout(int id) {
    refresh_ = false;
    ListControl *plstc = (ListControl *)GetControlPtr(kidcGameList);
    dword gameid = (dword)plstc->GetSelectedItemData();
    plstc->Clear();
    GameMap::iterator it = map_.begin();
    for (; it != map_.end(); it++) {
        GameInfo& info = it->second;
        char s[80];
        GetString(info, s, sizeof(s));
        plstc->Add(s, (void *)info.gameid);
    }
    int selected = FindIndex(gameid);
    if (selected < 0 && map_.size() != 0) {
        plstc->Select(0, true, true);
    } else {
        plstc->Select(selected, true);
    }
    refresh_ = true;
    HideShowJoinGame();

    // Update room label with player count.
    int count = players_.size();
    for (GameMap::iterator it = map_.begin(); it != map_.end(); it++) {
        count += it->second.cnames;
    }
    LabelControl *plbl = (LabelControl *)GetControlPtr(kidcRoomName);
    plbl->SetText(base::Format::ToString("%s (%d %s, %d %s)",
            roominfo_.name.c_str(), count,
            count == 1 ? "player" : "players", map_.size(),
            map_.size() == 1 ? "game" : "games"));
}

void RoomForm::HideShowJoinGame() {
    ListControl *plstc = (ListControl *)GetControlPtr(kidcGameList);
    dword gameid = (dword)plstc->GetSelectedItemData();
    GameMap::iterator it = map_.find(gameid);
    bool show = true;
    if (it == map_.end()) {
        show = false;
    } else {
        GameInfo& info = it->second;
        if (info.playing) {
            show = false;
        }
        if (info.cnames == info.maxplayers) {
            show = false;
        }
    }
    Control *pctl = GetControlPtr(kidcJoinGame);
    pctl->Show(show);

    // Show the "no games" message if there are no games
    if (map_.size() == 0) {
        GetControlPtr(kidcGameList)->Show(false);
        GetControlPtr(kidcNoGames)->Show(true);
    } else {
        GetControlPtr(kidcGameList)->Show(true);
        GetControlPtr(kidcNoGames)->Show(false);
    }
}

void RoomForm::OnAddPlayer(const char *player) {
    players_.push_back(player);
    Refresh();

    if (!status_) {
        chatter_.AddChat(player,
                base::Format::ToString("%s entered the room.", player), true);
    }
}

void RoomForm::OnRemovePlayer(dword hint, const char *player) {
    // Remove the player from the list
    {
        std::vector<std::string>::iterator it;
        it = std::find(players_.begin(), players_.end(), player);
        if (it != players_.end()) {
            players_.erase(it);
        }
        Refresh();
    }

    // If the player is creating or joining a game, don't add a system
    // message. Otherwise, do add one.
    bool add = true;
    GameMap::iterator it = map_.begin();
    for (; it != map_.end(); it++) {
        if (strcmp(player, it->second.creator) == 0) {
            add = false;
            break;
        }
    }

    // If joining a game, a system message will be added later with that
    // notification.
    if (hint == knLeaveRoomHintJoinGame) {
        add = false;
    }

    if (add && !status_) {
        chatter_.AddChat(player, base::Format::ToString("%s left the room.",
                player), true);
    }
}

void RoomForm::OnReceiveChat(const char *player, const char *chat) {
    chatter_.AddChat(player, chat, false);
}

void RoomForm::OnStatusComplete() {
    // Callbacks after this mean change in status, not current status.
    status_ = false;

    // Add a system message to chat. The current player is in players_[0]
    std::string s = base::Format::ToString("%s has entered the %s game room. ",
            players_[0].c_str(), roominfo_.name.c_str());

    s += GetPlayersString();
    chatter_.AddChat("", s.c_str(), true);
}

std::string RoomForm::GetPlayersString() {
    std::string s;
    if (players_.size() == 1) {
        s += "No other players are in the room.";
        return s;
    }

    if (players_.size() == 2) {
        s += base::Format::ToString("%s is also in the room.",
                    players_[1].c_str());
        return s;
    }

    for (int i = 1; i < players_.size(); i++) {
        if (i != players_.size() - 1) {
            s += base::Format::ToString("%s, ", players_[i].c_str());
        } else {
            s += base::Format::ToString("and %s ", players_[i].c_str());
        }
    }
    s += "are also in the room.";
    return s;
}

void RoomForm::GetString(GameInfo& info, char *s, int cb) {
    // Format:
    // Desert River Riches, 1.0x, 2-4 players: jim, bob, sam
    // Desert River Riches, 1.0x, 2 players: jim, bob
    // (play) Desert River Riches, 1.5x, 2-4 players: jim, bob...
  
    const char *d = info.GetDescription();
    if (info.playing) {
        snprintf(s, cb, "(play) %s: ", d);
    } else {
        snprintf(s, cb, "%s: ", d);
    }

    // Add the players one by one.
    for (int i = 0; i < info.cnames; i++) {
        int cch = strlen(s);
        if (i != 0) {
            if (cb - cch < 3) {
                break;
            }
            strcat(s, ", ");
            cch += 2;
        }
        strncpyz(&s[cch], info.anames[i].szPlayerName, cb - cch);
    }
}

void RoomForm::OnStatusUpdate(char *pszStatus) {
}

void RoomForm::OnConnectionClose() {
    Event evt;
    memset(&evt, 0, sizeof(evt));
    evt.idf = m_idf;
    evt.eType = connectionCloseEvent;
    gevm.PostEvent(&evt);
}

void RoomForm::OnShowMessage(const char *message) {
    message_ = message;
    Event evt;
    memset(&evt, 0, sizeof(evt));
    evt.idf = m_idf;
    evt.eType = showMessageEvent;
    gevm.PostEvent(&evt);
}

bool RoomForm::OnFilterEvent(Event *pevt) {
    if (pevt->eType == connectionCloseEvent) {
        chatter_.HideChat();
        HtMessageBox(kfMbWhiteBorder, "Comm Problem", "The server has closed your connection.");
        EndForm(kidcCancel);
        return true;
    }

    if (pevt->eType == showMessageEvent) {
        chatter_.HideChat();
        HtMessageBox(kfMbWhiteBorder, "Server Message", message_.c_str());
        message_ = "";
        return true;
    }
    return false;
}


const char *GameInfo::GetDescription() const {
    // Format if not playing:
    // Desert River Riches, 1.0x, 2-4 players:
    // Format if playing:
    // Desert River Riches

    if (playing) {
        return title;
    }

    char szSpeed[32];
    GetSpeedMultiplierString(szSpeed, params.tGameSpeed);

    if (minplayers == maxplayers) {
        return base::Format::ToString("%s, %s, %d players", title,
                szSpeed, minplayers);
    } else {
        return base::Format::ToString("%s, %s, %d-%d players",
                title, szSpeed, minplayers, maxplayers);
    }
}

} // namespace wi
