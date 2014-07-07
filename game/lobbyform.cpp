#include "game/lobbyform.h"
#include "game/createroomform.h"
#include "game/roomform.h"
#include "server/room.h"

namespace wi {

LobbyForm::LobbyForm(LoginHandler& handler, const std::string& server_name) :
        handler_(handler), server_name_(server_name), refresh_(true),
        lurkers_(0), zipdone_(false), selected_main_(false) {
}

LobbyForm::~LobbyForm() {
}

void LobbyForm::ShowJoinMessage(dword result) {
    // Show an appropriate message if it was not a success
    const char *message = NULL;
    switch (result) {
    case knLobbyJoinResultSuccess:
        break;
    case knLobbyJoinResultNotLoggedIn:
        message = "Not logged in";
        break;
    case knLobbyJoinResultFull:
        message = "Lobby is full, try again later.";
        break;
    case knLobbyJoinResultFail:
    default:
        message = "Failure entering lobby";
        break;
    }

    if (message != NULL) {
        HtMessageBox(kfMbWhiteBorder, "Enter Lobby Problem", message);
    }
}

dword LobbyForm::DoForm(LoginHandler &handler, const std::string& server_name,
        RoomInfo *joininfo) {
    // The user is logged in by the time this gets called.
    LobbyForm *pfrm = (LobbyForm *)gpmfrmm->LoadForm(gpiniForms, kidfLobby,
            new LobbyForm(handler, server_name));
    if (pfrm == NULL) {
        return false;
    }
    int result = 0;
    pfrm->DoModal(&result);
    *joininfo = pfrm->joininfo();
    delete pfrm;

    if (result == kidcSignOut) {
        return knLobbyResultSignOut;
    }
    if (result == kidcCancel) {
        return knLobbyResultDone;
    }
    if (result == kidcJoinRoom) {
        return knLobbyResultEnterRoom;
    }
    return knLobbyResultDone;
}

bool LobbyForm::DoModal(int *presult, Sfx sfxShow,  Sfx sfxHide) {
    char name[kcbPlayerName*2];
    handler_.GetPlayerName(name, sizeof(name));
    LabelControl *plbl = (LabelControl *)GetControlPtr(kidcPlayerName);
    plbl->Show(false);
    plbl = (LabelControl *)GetControlPtr(kidcPlayerNameLabel);
    const char *s = base::Format::ToString("%s / %s", name,
        server_name_.c_str());
    plbl->SetText(s);

    ListControl *plstc = (ListControl *)GetControlPtr(kidcRoomList);
    Rect rcList;
    plstc->GetRect(&rcList);
    Font *pfnt = plstc->GetFont();
    int cxPrivate = pfnt->GetTextExtent("PRIVATE");
    int cxStart = rcList.Width() / 20; // 10;
    plstc->SetTabStops(0, cxStart, rcList.Width() - cxStart - cxPrivate);
    plstc->SetTabFlags(0, kfLstTabEllipsis, 0);
    GetControlPtr(kidcJoinRoom)->Show(false);

    Show(true);
    gptra->SetCallback(this);
    dword result = gptra->JoinLobby(this);
    if (result != knLobbyJoinResultSuccess) {
        gptra->SetCallback(NULL);
        ShowJoinMessage(result);
        *presult = 0;
        return false;
    }
    bool success = ShellForm::DoModal(presult, sfxShow, sfxHide);
    gptra->LeaveLobby();
    gptra->SetCallback(NULL);

    return success;
}

void LobbyForm::OnControlSelected(word idc) {
    switch (idc) {
    case kidcNewRoom:
        OnCreateRoom();
        break;

    case kidcJoinRoom:
        OnJoinRoom();
        break;

    case kidcCancel:
    case kidcSignOut:
        EndForm(idc);
        break;
    }
}

void LobbyForm::OnControlNotify(word idc, int nNotify) {
    if (idc == kidcRoomList && nNotify == knNotifySelectionChange) {
        Refresh(0);
    }
    Form::OnControlNotify(idc, nNotify);
}

void LobbyForm::OnJoinRoom() {
    ListControl *plstc = (ListControl *)GetControlPtr(kidcRoomList);
    dword roomid = (dword)plstc->GetSelectedItemData();
    RoomMap::iterator it = map_.find(roomid);
    if (it == map_.end()) {
        return;
    }
    if (it->second.priv) {
        HostInitiateAsk("Enter Password:", kcbPassword - 1,
                it->second.password.c_str(), knKeyboardAskDefault,
                true);
        return;
    }
    InitiateJoinRoom(it->second);
}

void LobbyForm::OnCreateRoom() {
    char roomname[kcbRoomname];
    char password[kcbPassword];
    if (!CreateRoomForm::DoForm(roomname, sizeof(roomname), password,
            sizeof(password))) {
        return;
    }

    if (gptra == NULL) {
        return;
    }

    dword roomid;
    dword result = gptra->CreateRoom(roomname, password, &roomid);
    switch (result) {
    case knLobbyCreateRoomResultSuccess:
        {
            // Update the list and select the created room, in case
            // the join fails.
            Refresh(-1);
            ListControl *plstc = (ListControl *)GetControlPtr(kidcRoomList);
            refresh_ = false;
            plstc->Select(FindIndex(roomid), true, true);
            refresh_ = true;

            // OnAddRoom has been called already. Shove the password
            // into the info and join the room.
            RoomMap::iterator it = map_.find(roomid);
            if (it != map_.end()) {
                it->second.password = password;
                InitiateJoinRoom(it->second);
            }
        }
        break;

    case knLobbyCreateRoomResultFail:
        HtMessageBox(kfMbWhiteBorder, "Create Room",
                "Error creating this room.");
        break;

    case knLobbyCreateRoomResultFull:
        HtMessageBox(kfMbWhiteBorder, "Create Room", "Too many rooms. Join an existing room");
        break;

    case knLobbyCreateRoomResultExists:
        HtMessageBox(kfMbWhiteBorder, "Create Room",
                "A room with this name already exists.");
        break;

    }
}

int LobbyForm::FindIndex(dword roomid) {
    int index = 0;
    RoomMap::iterator it = map_.begin();
    for (; it != map_.end(); it++) {
        if (it->first == roomid) {
            return index;
        }
        index++;
    }
    return -1;
}

void LobbyForm::InitiateJoinRoom(const RoomInfo& info) {
    if (gptra == NULL) {
        return;
    }

    // Before joining, check to see if this room is joinable.
    // Actually joining also checks, but this is a way to provide the user
    // with feedback before exiting this form (actual joining happens
    // in RoomForm).
    dword result = gptra->CanJoinRoom(info.roomid, info.password.c_str());
    RoomForm::ShowJoinMessage(result);
    switch (result) {
    case knRoomJoinResultWrongPassword:
        OnJoinRoom();
        break;

    case knRoomJoinResultSuccess:
        joininfo_ = info;
        EndForm(kidcJoinRoom);
        break;
    }
}

void LobbyForm::Refresh(int ct) {
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

void LobbyForm::OnTimeout(int id) {
    refresh_ = false;
    ListControl *plstc = (ListControl *)GetControlPtr(kidcRoomList);
    dword roomid = (dword)plstc->GetSelectedItemData();
    if (!selected_main_) {
        roomid = kroomidMain;
    }
    plstc->Clear();
    RoomMap::iterator it = map_.begin();
    for (; it != map_.end(); it++) {
        RoomInfo& info = it->second;
        const char *s;
        if (info.priv) {
            s = base::Format::ToString("\t%s (%d %s, %d %s)\tPRIVATE",
                    info.name.c_str(), info.cPlayers,
                    info.cPlayers == 1 ? "player" : "players",
                    info.cGames,
                    info.cGames == 1 ? "game" : "games");
        } else {
            s = base::Format::ToString("\t%s (%d %s, %d %s)",
                    info.name.c_str(), info.cPlayers,
                    info.cPlayers == 1 ? "player" : "players",
                    info.cGames,
                    info.cGames == 1 ? "game" : "games");
        }
        plstc->Add(s, (void *)info.roomid);
    }
    int selected = FindIndex(roomid);
    if (selected < 0 && map_.size() != 0) {
        plstc->Select(0, true, true);
    } else {
        plstc->Select(selected, true);
        selected_main_ = true;
    }
    Control *pctl = GetControlPtr(kidcJoinRoom);
    refresh_ = true;
    pctl->Show(plstc->GetSelectedItemIndex() >= 0);
}

void LobbyForm::OnLurkerCount(dword count) {
    lurkers_ = count;
    if (zipdone_) {
        LabelControl *plbl = (LabelControl *)GetControlPtr(kidcLurkerCount);
        plbl->SetText(base::Format::ToString("Lurkers: %d", count));
    }
}

void LobbyForm::OnZipDone() {
    zipdone_ = true;
    OnLurkerCount(lurkers_);
}

void LobbyForm::OnAddRoom(const char *name, dword roomid, bool priv,
        dword cPlayers, dword cGames) {
    RoomInfo info;
    info.name = name;
    info.roomid = roomid;
    info.priv = priv;
    info.cPlayers = cPlayers;
    info.cGames = cGames;
    map_.insert(RoomMap::value_type(roomid, info));
    Refresh();
}

void LobbyForm::OnRemoveRoom(dword roomid) {
    RoomMap::iterator it = map_.find(roomid);
    if (it == map_.end()) {
        LOG() << "Couldn't find room!";
        return;
    }
    map_.erase(it);
    Refresh();
}

void LobbyForm::OnUpdateRoom(dword roomid, dword cPlayers, dword cGames) {
    RoomMap::iterator it = map_.find(roomid);
    if (it == map_.end()) {
        LOG() << "Couldn't find room!";
        return;
    }
    it->second.cPlayers = cPlayers;
    it->second.cGames = cGames;
    Refresh();
}

void LobbyForm::OnStatusUpdate(char *pszStatus) {
}

void LobbyForm::OnConnectionClose() {
    Event evt;
    memset(&evt, 0, sizeof(evt));
    evt.idf = m_idf;
    evt.eType = connectionCloseEvent;
    gevm.PostEvent(&evt);
}

void LobbyForm::OnShowMessage(const char *message) {
    message_ = message;
    Event evt;
    memset(&evt, 0, sizeof(evt));
    evt.idf = m_idf;
    evt.eType = showMessageEvent;
    gevm.PostEvent(&evt);
}

bool LobbyForm::OnFilterEvent(Event *pevt) {
    if (pevt->eType == askStringEvent) {
        if (gptra == NULL) {
            return true;
        }

        char s[512];
        HostGetAskString(s, sizeof(s));

        ListControl *plstc = (ListControl *)GetControlPtr(kidcRoomList);
        dword roomid = (dword)plstc->GetSelectedItemData();
        RoomMap::iterator it = map_.find(roomid);
        if (it != map_.end()) {
            it->second.password = s;
            InitiateJoinRoom(it->second);
        }
        return true;
    }

    if (pevt->eType == connectionCloseEvent) {
        HtMessageBox(kfMbWhiteBorder, "Comm Problem", "The server has closed your connection.");
        EndForm(kidcCancel);
        return true;
    }

    if (pevt->eType == showMessageEvent) {
        HtMessageBox(kfMbWhiteBorder, "Server Message", message_.c_str());
        message_ = "";
        return true;
    }
    return false;
}

} // namespace wi
