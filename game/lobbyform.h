#ifndef __LOBBYFORM_H__
#define __LOBBYFORM_H__

#include "game/ht.h"
#include "game/loginform.h"
#include "game/loginhandler.h"
#include <string>
#include <map>

namespace wi {

const dword knLobbyResultSignOut = 0;
const dword knLobbyResultEnterRoom = 1;
const dword knLobbyResultDone = 2;

STARTLABEL(LobbyResults)
    LABEL(knLobbyResultSignOut)
    LABEL(knLobbyResultEnterRoom)
    LABEL(knLobbyResultDone)
ENDLABEL(LobbyResults)

struct RoomSorter {
    bool operator()(dword a, dword b) const {
        return a > b;
    }
};

struct RoomInfo {
    std::string name;
    std::string password;
    dword roomid;
    bool priv;
    dword cPlayers;
    dword cGames;
};
typedef std::map<dword, RoomInfo, RoomSorter> RoomMap;

class LobbyForm : public ShellForm, ILobbyCallback, ITransportCallback,
        ITimeout {
public:
    LobbyForm(LoginHandler& handler, const std::string& server_name);
    ~LobbyForm();

    static void ShowJoinMessage(dword result);
    static dword DoForm(LoginHandler& handler, const std::string& server_name,
            RoomInfo *joininfo);
    const RoomInfo& joininfo() { return joininfo_; }

    // Form overrides
	virtual bool DoModal(int *pnResult = NULL, Sfx sfxShow = ksfxGuiFormShow,
            Sfx sfxHide = ksfxGuiFormHide);
    virtual void OnControlSelected(word idc);
    virtual void OnControlNotify(word idc, int nNotify);
    virtual bool OnFilterEvent(Event *pevt);

    // ShellForm overrides
    virtual void OnZipDone();

private:
    void Refresh(int ct = 20);
    void OnCreateRoom();
    void OnJoinRoom();
    void InitiateJoinRoom(const RoomInfo& info);
    int FindIndex(dword roomid);

    // ILobbyCallback
    virtual void OnLurkerCount(dword count);
    virtual void OnAddRoom(const char *name, dword roomid, bool priv,
            dword cPlayers, dword cGames);
    virtual void OnRemoveRoom(dword roomid);
    virtual void OnUpdateRoom(dword roomid, dword cPlayers, dword cGames);

    // ITransportCallback
    virtual void OnStatusUpdate(char *pszStatus);
    virtual void OnConnectionClose();
    virtual void OnShowMessage(const char *message);

    // ITimeout
	virtual void OnTimeout(int id);

    dword lurkers_;
    TimeoutTimer timer_;
    LoginHandler& handler_;
    RoomMap map_;
    RoomInfo joininfo_;
    bool refresh_;
    bool zipdone_;
    bool selected_main_;
    std::string message_;
    std::string server_name_;
};

}

#endif // __LOBBYFORM_H__
