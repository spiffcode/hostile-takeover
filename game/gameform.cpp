#include "ht.h"

#include "game/gameform.h"
#include "game/multiplayer.h"
#include "mpshared/messages.h"
#include "game/httppackmanager.h"
#include "base/format.h"

namespace wi {

GameForm::GameForm(LoginHandler& handler, const GameInfo& info,
        bool creator, Chatter& chatter) : handler_(handler), info_(info),
        creator_(creator), chatter_(chatter), joined_(false) {
    chatter_.SignalOnBlink.connect(this, &GameForm::OnChatBlink);
    chatter_.SignalOnPlayers.connect(this, &GameForm::OnPlayers);
}

GameForm::~GameForm() {
    chatter_.SignalOnBlink.disconnect(this);
    chatter_.SignalOnPlayers.disconnect(this);
    chatter_.HideChat();
}

void GameForm::ShowJoinMessage(dword result) {
    char *message = NULL;
    switch (result) {
    case knGameJoinResultSuccess:
        break;
    case knGameJoinResultFail:
    default:
        message = "Error joining this game.";
        break;
    case knGameJoinResultRoomNotFound:
        message = "This game room no longer available.";
        break;
    case knGameJoinResultGameNotFound:
        message = "This game is no longer available.";
        break;
    case knGameJoinResultGameFull:
        message = "This game is full.";
        break;
    case knGameJoinResultInProgress:
        message = "This game is already in progress.";
        break;
    }
    if (message != NULL) {
        HtMessageBox(kfMbWhiteBorder, "Join Game", message);
    }
}

dword GameForm::DoForm(LoginHandler& handler, const GameInfo& info,
        bool creator, Chatter& chatter) {
    if (gptra == NULL) {
        return knGameStartResultFail;
    }

    GameForm *pfrm = (GameForm *)gpmfrmm->LoadForm(gpiniForms, kidfGameStart,
            new GameForm(handler, info, creator, chatter));
    if (pfrm == NULL) {
        return knGameStartResultFail;
    }

    int result = 0;
    pfrm->DoModal(&result);
    delete pfrm;

    if (result == kidcCancel) {
        return knGameStartResultDone;
    }

    if (result == kidcOk) {
        return knGameStartResultStart;
    }

    return knGameStartResultFail;
}

bool GameForm::DoModal(int *presult) {
    if (creator_) {
        ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcOk);
        pbtn->SetText("BEGIN GAME");
        LabelControl *plbl = (LabelControl *)GetControlPtr(kidcGameName);
        char name[kcbPlayerName];
        handler_.GetPlayerName(name, sizeof(name));
        plbl->SetText(base::Format::ToString("%s's game", name));
    }

    // min/max players
    LabelControl *plbl = (LabelControl *)GetControlPtr(kidcMapName);
    plbl->SetText(info_.title);
    plbl = (LabelControl *)GetControlPtr(kidcNumPlayers);
    if (info_.minplayers == info_.maxplayers) {
        plbl->SetText(base::Format::ToString("%d", info_.minplayers));
    } else {
        plbl->SetText(base::Format::ToString("%d-%d", info_.minplayers,
                info_.maxplayers));
    }

    // Game speed
    char szT[16];
    GetSpeedMultiplierString(szT, info_.params.tGameSpeed);
    plbl = (LabelControl *)GetControlPtr(kidcGameSpeedLabel);
    plbl->SetText(szT);

    // If not creator, state readiness any time
    if (!creator_) {
        ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcOk);
        pbtn->Show(true);
    }

    // Join the game and bring up the form.
    Show(true);
    gptra->SetCallback(this);
    gptra->SetGameCallback(this);
    dword result = gptra->JoinGame(info_.gameid, info_.roomid);
    if (result != knGameJoinResultSuccess) {
        gptra->SetGameCallback(NULL);
        gptra->SetCallback(NULL);
        ShowJoinMessage(result);
        *presult = 0;
        return false;
    }
    joined_ = true;

    // Set up chat
    chatter_.SetChatTitle("Game Chat");
    chatter_.AddChat("", base::Format::ToString("%s created game %s.",
            info_.creator, info_.GetDescription()), true);

    *presult = 0;
    bool success = ShellForm::DoModal(presult);
    if (*presult != kidcOk && joined_) {
        gptra->LeaveGame();
    }
    gptra->SetGameCallback(NULL);
    gptra->SetCallback(NULL);

    return success; 
}

void GameForm::OnChatBlink(bool on) {
    GetControlPtr(kidcChat)->Show(on);
}

void GameForm::OnPlayers() {
    std::string s = GetPlayersString();
    chatter_.AddChat("", s.c_str(), true);
}

const char *GameForm::GetColorName(int side) {
    switch (side) {
    case 0:
        return "gray";
    case 1:
        return "blue";
    case 2:
        return "red";
    case 3:
        return "yellow";
    case 4:
        return "cyan";
    }
    return "unknown";
}

std::string GameForm::GetPlayersPlayingString() {
    // Extract the player names from Player objects. Make this player
    // player 0.

    std::vector<std::string> names;
    names.push_back(base::Format::ToString("%s (%s)", gpplrLocal->GetName(),
            GetColorName(gpplrLocal->GetSide())));
    Player *pplr = NULL;
    while ((pplr = gplrm.GetNextHumanPlayer(pplr)) != NULL) {
        if (pplr->GetFlags() & kfPlrUnfulfilled) {
            continue;
        }
        if (pplr == gpplrLocal) {
            continue;
        }
        names.push_back(base::Format::ToString("%s (%s)", pplr->GetName(),
                GetColorName(pplr->GetSide())));
    }

    std::string s;
    if (gpplrLocal->GetFlags() & kfPlrObserver) {
        s = base::Format::ToString("You are %s and are observing. ",
                GetColorName(gpplrLocal->GetSide()));
    } else {
        s = base::Format::ToString("You are %s. ",
                GetColorName(gpplrLocal->GetSide()));
    }

    if (names.size() == 1) {
        s += "No other players are in this game.";
        return s;
    }

    if (names.size() == 2) {
        s += base::Format::ToString("%s is also in this game.",
                    names[1].c_str());
        return s;
    }

    for (int i = 1; i < names.size(); i++) {
        if (i != names.size() - 1) {
            s += base::Format::ToString("%s, ", names[i].c_str());
        } else {
            s += base::Format::ToString("and %s ", names[i].c_str());
        }
    }
    s += "are also in this game.";
    return s;
}

std::string GameForm::GetPlayersObservingString() {
    // Collect observing players. There are none if game is just starting
    // however this static method is also called in-game.
    Player *pplr = NULL;
    std::vector<std::string> observers;
    while ((pplr = gplrm.GetNextObservingPlayer(pplr)) != NULL) {
        if (pplr->GetFlags() & kfPlrUnfulfilled) {
            continue;
        }
        if (pplr == gpplrLocal) {
            continue;
        }
        observers.push_back(base::Format::ToString("%s (%s)", pplr->GetName(),
                GetColorName(pplr->GetSide())));
    }
    
    if (observers.size() == 0) {
        return "";
    }

    if (observers.size() == 1) {
        return observers[0] + " is observing.";
    }

    std::string s;
    for (int i = 0; i < observers.size(); i++) {
        if (i != observers.size() - 1) {
            s += base::Format::ToString("%s, ", observers[i].c_str());
        } else {
            s += base::Format::ToString("and %s ", observers[i].c_str());
        }
    }
    return s + "are observing.";
}

std::string GameForm::GetPlayersString() {
    std::string playing = GetPlayersPlayingString();
    std::string observing = GetPlayersObservingString();

    if (observing.size() == 0) {
        return playing;
    }
    return playing + " " + observing;
}

void GameForm::OnControlSelected(word idc) {
    switch (idc) {
            
    // "Ready"/"Begin Game" button has been pressed
        
    case kidcOk:
        OnReadyBeginGame();
        break;
        
    case kidcCancel:
        EndForm(kidcCancel);
        break;

    case kidcChat:
        chatter_.ShowChat();
        break;
    }
}

void GameForm::OnReadyBeginGame() {
    // Players only need to say they're ready once
    ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcOk);
    pbtn->Show(false);

    if (creator_) {
        // The server may fail this,
        // because a player may drop out at the last second, so
        // this dialog isn't dismissed until the ScBeginGame
        // net message is received.
        NetMessage nmsg(knmidCsRequestBeginGame);
        gptra->SendNetMessage(&nmsg);
        return;
    }
    
    // Let the server know this player is ready
    NetMessage nmsg(knmidCsClientReady);
    gptra->SendNetMessage(&nmsg);
}

void GameForm::OnReceiveChat(const char *player, const char *chat) {
    chatter_.AddChat(player, chat, false);
}

void GameForm::OnNetMessage(NetMessage **ppnm) {
    NetMessage *pnm = *ppnm;
    switch (pnm->nmid) {
    case knmidScBeginGameFail:
        OnBeginGameFail();
        break;
        
    case knmidScPlayersUpdate:
        OnPlayersUpdate((PlayersUpdateNetMessage *)pnm);
        break;
        
    case knmidScGameParams:
        OnGameParams((GameParamsNetMessage *)pnm);
        break;
        
    case knmidScBeginGame:
        SetRandomSeed(((BeginGameNetMessage *)pnm)->ulRandomSeed);
        EndForm(kidcOk);
        break;
        
    case knmidScCantAcceptMoreConnections:
        HtMessageBox(kfMbWhiteBorder, "Game Full", 
    "Sorry, the requested game is full and can't accept any more players.");
        EndForm(kidcCancel);
        break;
    }
}

void GameForm::OnBeginGameFail() {
    // Beginning the game failed; show the Begin Game button again.
    // This message rarely happens, in the central server
    // case where a game is requested to begin, but the server
    // fails the request (a player left the game at the last
    // second).
    ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcOk);
    pbtn->Show(true);
}

std::vector<GameForm::PlayerReady> GameForm::GetPlayerReadies() {
    std::vector<PlayerReady> readies;
    Player *pplr = NULL;
    while ((pplr = gplrm.GetNextHumanPlayer(pplr)) != NULL) {
        if (pplr->GetFlags() & (kfPlrUnfulfilled | kfPlrComputer)) {
            continue;
        }
        PlayerReady rdy;
        strcpy(rdy.szName, pplr->GetName());
        rdy.side = pplr->GetSide();
        rdy.fReady = (pplr->GetFlags() & kfPlrReady) != 0;
        rdy.fLocal = (pplr == gpplrLocal);
        readies.push_back(rdy);
    }
    return readies;
}

void GameForm::OnPlayersUpdate(PlayersUpdateNetMessage *pnm) {
    std::vector<PlayerReady> oldreadies = GetPlayerReadies();
    gplrm.Init((PlayersUpdateNetMessage *)pnm);
    std::vector<PlayerReady> newreadies = GetPlayerReadies();

    // Announce joins and leaves
    bool sfx = false;
    for (int i = 0; i < newreadies.size(); i++) {
        bool found = false;
        for (int j = 0; j < oldreadies.size(); j++) {
            if (strcmp(newreadies[i].szName, oldreadies[j].szName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            chatter_.AddChat("", base::Format::ToString(
                    "%s (%s) joined this game.",
                    newreadies[i].szName,
                    GetColorName(newreadies[i].side)),
                    true);

            // sfx on add if it isn't this player
            if (!newreadies[i].fLocal) {
                sfx = true;
            }
        }
    }
    for (int j = 0; j < oldreadies.size(); j++) {
        bool found = false;
        for (int i = 0; i < newreadies.size(); i++) {
            if (strcmp(newreadies[i].szName, oldreadies[j].szName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            chatter_.AddChat("", base::Format::ToString(
                    "%s (%s) left this game.",
                    oldreadies[j].szName,
                    GetColorName(oldreadies[j].side)),
                    true);
            sfx = true;
        }
    }
    if (sfx) {
        gsndm.PlaySfx(ksfxGuiScrollingListSelectItem);
    }

    // Announce ready transitions from not-ready to ready
    for (int i = 0; i < newreadies.size(); i++) {
        bool found = false;
        bool ready = false;
        for (int j = 0; j < oldreadies.size(); j++) {
            if (strcmp(newreadies[i].szName, oldreadies[j].szName) == 0) {
                if (!oldreadies[j].fReady && newreadies[i].fReady) {
                    ready = true;
                    break;
                }
                found = true;
            }
        }
        if (!found && newreadies[i].fReady) {
            ready = true;
        }
        if (ready) {
            chatter_.AddChat("", base::Format::ToString("%s (%s) is READY.",
                    newreadies[i].szName,
                    GetColorName(newreadies[i].side)), true);
        }
    }

    RefreshPlayerList();
    
    const char *pszCreator = gplrm.GetCreatorName();
    if (pszCreator != NULL) {
        LabelControl *plbl = (LabelControl *)GetControlPtr(kidcGameName);
        plbl->SetText(base::Format::ToString("%s's game", pszCreator));
    }
    
    if (creator_) {
        ShowOrHideBeginGameButton();
    }
}

void GameForm::OnGameParams(GameParamsNetMessage *pgpnm) {
    // Check to see if this client is compatible with this server,
    // and if this client has the same mission the server has
    if (!IsExpectedGameParams(pgpnm->rams)) {
        EndForm(kidcCancel);
        return;
    }
    
    // Join the game, NetMessage variety. This causes the server
    // to allocate a player for this client, and report back error
    // if there are no slots available.
    char name[kcbPlayerName];
    handler_.GetPlayerName(name, sizeof(name));
    PlayerJoinNetMessage pjnm;
    strncpyz(pjnm.szPlayerName, name, sizeof(pjnm.szPlayerName));
    gptra->SendNetMessage(&pjnm);
   
    // Pretend the creator pressed the ready button
    if (creator_) { 
        NetMessage nmsg(knmidCsClientReady);
        gptra->SendNetMessage(&nmsg);
    }
}

void GameForm::OnGameDisconnect() {
    chatter_.HideChat();
    HtMessageBox(kfMbWhiteBorder, "Game Cancelled", 
            "This game has been cancelled. Press OK to continue.");
    joined_ = false;
    EndForm(kidcCancel);
}

void GameForm::OnStatusUpdate(char *pszStatus) {
    // There's no status we care to display on this form
}

void GameForm::OnConnectionClose() {
    Event evt;
    memset(&evt, 0, sizeof(evt));
    evt.idf = m_idf;
    evt.eType = connectionCloseEvent;
    gevm.PostEvent(&evt);
}

void GameForm::OnShowMessage(const char *message) {
    message_ = message;
    Event evt;
    memset(&evt, 0, sizeof(evt));
    evt.idf = m_idf;
    evt.eType = showMessageEvent;
    gevm.PostEvent(&evt);
}

bool GameForm::OnFilterEvent(Event *pevt) {
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

bool GameForm::IsExpectedGameParams(const GameParams& params) {
    // This is all really unnecessary.
    if (memcmp(&params.packid, &info_.params.packid, sizeof(PackId)) != 0) {
        return false;
    }
    if (params.dwVersionSimulation != info_.params.dwVersionSimulation) {
        return false;
    }
    if (params.tGameSpeed != info_.params.tGameSpeed) {
        return false;
    }
    if (strcmp(params.szLvlFilename, info_.params.szLvlFilename) != 0) {
        return false;
    }
    return true;
}

int GameForm::GetPlayerCountNeeded(bool fReady) {
    int cplrTotal = 0;
    int cplrReady = 0;
    Player *pplr = NULL;
    while ((pplr = gplrm.GetNextHumanPlayer(pplr)) != NULL) {
        word wf = pplr->GetFlags();
        if ((wf & kfPlrUnfulfilled) != 0) {
            continue;
        }
        if (wf & kfPlrReady) {
            cplrReady++;
        }
        cplrTotal++;
    }
    
    int cNeeded;
    if (fReady) {
        cNeeded = info_.minplayers - cplrReady;
    } else {
        cNeeded = info_.minplayers - cplrTotal;
    }
    if (cNeeded <= 0) {
        return 0;
    }
    return cNeeded;
}

void GameForm::ShowOrHideBeginGameButton() {
    bool fHaveNeededPlayers = (GetPlayerCountNeeded(true) == 0);

    bool fAllPlayersReady =  true;
    Player *pplr = NULL;
    while ((pplr = gplrm.GetNextHumanPlayer(pplr)) != NULL) {
        word wf = pplr->GetFlags();
        if ((wf & kfPlrUnfulfilled) != 0) {
            continue;
        }
        if ((wf & kfPlrReady) == 0) {
            fAllPlayersReady = false;
            break;
        }
    }

    ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcOk);
    pbtn->Show(fHaveNeededPlayers && fAllPlayersReady);
}

void GameForm::RefreshPlayerList() {
    ListControl *plstc = (ListControl *)GetControlPtr(kidcPlayerList);
    plstc->Clear();
    
    Player *pplr = NULL;
    int cAdded = 0;
    while ((pplr = gplrm.GetNextHumanPlayer(pplr)) != NULL) {
        
        // Don't show unfulfilled players
        
        if (pplr->GetFlags() & kfPlrUnfulfilled) {
            continue;
        }
        
        char szT[100];
        sprintf(szT, "%s (%s) is %s", pplr->GetName(),
                GetColorName(pplr->GetSide()),
                pplr->GetFlags() & kfPlrReady ? "READY" : "not ready");
        plstc->Add(szT, pplr);
        cAdded++;
    }
    int cNeeded = GetPlayerCountNeeded(false);
    if (cNeeded != 0) {
        plstc->Add(base::Format::ToString("%d more player%s needed",
                cNeeded, cNeeded != 1 ? "s" : ""), NULL);
    }
}
    
} // namespace wi
