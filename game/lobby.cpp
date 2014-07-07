#include "game/lobby.h"
#include "game/loginform.h"
#include "game/lobbyform.h"
#include "game/roomform.h"
#include "game/httppackmanager.h"
#include "game/chooseserverform.h"

namespace wi {

Lobby::Lobby() {
}

Lobby::~Lobby() {
}

dword Lobby::Shell(const PackId *ppackidFind) {
#ifndef MULTIPLAYER
    return knShellResultSuccess;
#endif

    // Connect to the service
    std::string server_name;
    dword result = Connect(&server_name);
    if (result == knConnectResultAppStop) {
        Disconnect();
        return knShellResultAppStop;
    }

    if (result != knConnectResultSuccess) {
        Disconnect();
        return knShellResultError;
    }

    // Run the shell handler
    result = ShellHandler(ppackidFind, server_name);
    Disconnect();
    return result;
}

dword Lobby::Connect(std::string *server_name) {
    dword result = ChooseServerForm::DoForm(&gptra, server_name);
    if (result == knChooseServerResultAppStop) {
        return knConnectResultAppStop;
    }
    if (gptra == NULL) {
        return knConnectResultError;
    }
    return knConnectResultSuccess;
}

void Lobby::Disconnect() {
    if (gptra != NULL) {
        TransportWaitingUI twui(knWaitStrClosingTransport);
        gptra->Close();
        delete gptra;
        gptra = NULL;
    }
}

dword Lobby::ShellHandler(const PackId *ppackidFind,
        const std::string& server_name) {
    LoginHandler handler;

    bool fAutoLogin = true;
    while (true) {
        // If the app is exiting, return
        if (gevm.IsAppStopping()) {
            return knShellResultSuccess;
        }

        // First, login if not already logged in
        while (!handler.loggedin()) {
            if (!LoginForm::DoForm(handler, fAutoLogin)) {
                return knShellResultSuccess;
            }
            fAutoLogin = true;
        }

        // The user has logged in. Now enter the lobby.
        RoomInfo roominfo;
        dword result = LobbyForm::DoForm(handler, server_name, &roominfo);

        // If logging off, logoff and go back to the login form
        if (result == knLobbyResultSignOut) {
            result = handler.SignOut();
            if (result == knSignOutResultFail) {
                return knShellResultError;
            }
            fAutoLogin = false;
            continue;
        }

        if (result == knLobbyResultDone) {
            return knShellResultSuccess;
        }

        // If not entering a room go back to the main menu
        if (result != knLobbyResultEnterRoom) {
            return knShellResultError;
        }

        // Entering room
        Chatter chatter(handler);
        while (true) {
            // If the app is exiting, return
            if (gevm.IsAppStopping()) {
                return knShellResultSuccess;
            }

            // Enter room.
            GameInfo gameinfo;
            result = RoomForm::DoForm(handler, roominfo, chatter, &gameinfo);

            // Go back to lobby
            if (result == knRoomResultDone || result == knRoomResultFail) {
                break;
            }

            // Just go back to the room if it's not one of these cases
            if (result != knRoomResultJoin && result != knRoomResultCreated) {
                continue;
            }

            // Show the GameStart form.
            bool creator = (result == knRoomResultCreated);
            result = GameForm::DoForm(handler, gameinfo, creator, chatter);
            if (result == knGameStartResultDone) {
                // Go back to the room
                continue;
            }

            // If not beginning a game, go back to the room. This case
            // shouldn't happen.
            if (result != knGameStartResultStart) {
                // Go back to the room
                continue;
            }

            // Begin the game! After playing, return to the room.
            result = BeginGame(gameinfo, creator, chatter);

            if (result == knBeginResultAppStop) {
                return knShellResultAppStop;
            }

            if (result == knBeginResultTransportClosed) {
                return knShellResultSuccess;
            }

            if (result == knBeginResultSuccess) {
                // TODO: Here is a good place to post game results back to the
                // www server. Use a LoginHandler method.
            }
        }
    }
}

dword Lobby::BeginGame(const GameInfo& info, bool creator, Chatter& chatter) {
    // Force certain game options that need to be the same across clients
    word wfPerfOptionsSave = gwfPerfOptions;
    gwfPerfOptions = kfPerfMax;
    long tGameSpeedSave = gtGameSpeed;
    gtGameSpeed = info.params.tGameSpeed;
    
    // Play the game!
    word wfRole = kfRoleMultiplayer;
    if (creator) {
        wfRole |= kfRoleCreator;
    }

    // The player has already joined the game, and it is starting.
    gppackm->Mount(gpakr, &info.params.packid);
    int nRet = ggame.RunSimulation(NULL, (char *)info.params.szLvlFilename,
            wfRole, info.gameid, &chatter);
    gppackm->Unmount(gpakr, &info.params.packid);

    // Leave the game
    if (gptra != NULL) {
        gptra->LeaveGame();
    }

    // Set these options back
    gwfPerfOptions = wfPerfOptionsSave;
    gtGameSpeed = tGameSpeedSave;
    
    if (nRet == knGoAppStop) {
        return knBeginResultAppStop;
    }
    
    if (gptra != NULL && gptra->IsClosed()) {
        return knBeginResultTransportClosed;
    }
    
    if (nRet == knGoInitFailure) {
        HtMessageBox(kfMbWhiteBorder, "Launch Error", "Unable to initialize the game!");
        return knBeginResultError;
    }
    
    return knBeginResultSuccess;
}

} // namespace wi
