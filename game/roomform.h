#ifndef __ROOMFORM_H__
#define __ROOMFORM_H__

#include "game/ht.h"
#include "game/loginhandler.h"
#include "game/lobbyform.h"
#include "game/chatter.h"
#include "base/sigslot.h"
#include <map>
#include <vector>
#include <string>

namespace wi {

const dword knLeaveRoomHintNone = 0;
const dword knLeaveRoomHintJoinGame = 1;

STARTLABEL(LeaveRoomHints)
    LABEL(knLeaveRoomHintNone)
    LABEL(knLeaveRoomHintJoinGame)
ENDLABEL(LeaveRoomHints)

const dword knRoomResultDone = 0;
const dword knRoomResultFail = 1;
const dword knRoomResultJoin = 2;
const dword knRoomResultCreated = 3;

STARTLABEL(RoomResults)
    LABEL(knRoomResultDone)
    LABEL(knRoomResultFail)
    LABEL(knRoomResultJoin)
    LABEL(knRoomResultCreated)
ENDLABEL(RoomResults)

struct GameSorter {
    bool operator()(dword id_a, dword id_b) const {
        return id_a > id_b;
    }
};

struct GameInfo {
    dword roomid;
    dword gameid;
    GameParams params;
    int minplayers;
    int maxplayers;
    char title[kcbLevelTitle];
    char creator[kcbPlayerName];
    bool playing;
    int cnames;
    PlayerName anames[kcPlayersMax];

    const char *GetDescription() const;
};
typedef std::map<dword, GameInfo, GameSorter> GameMap;

class RoomForm : public ShellForm, ITransportCallback, IRoomCallback,
        ITimeout, public base::has_slots<> {
public:
    RoomForm(LoginHandler& handler, const RoomInfo& roominfo,
            Chatter& chatter);
    virtual ~RoomForm() secMultiplayer;

    static dword DoForm(LoginHandler& handler, const RoomInfo& roominfo,
            Chatter& chatter, GameInfo *gameinfo);
    static void ShowJoinMessage(dword result);
    const GameInfo& joininfo() { return joininfo_; }
    bool creator() { return creator_; }

    // Form methods    
    bool DoModal(int *pnResult = NULL);
    void OnControlSelected(word idc);
    void OnControlNotify(word idc, int nNotify);    
    bool OnFilterEvent(Event *pevt);
    
    // IRoomCallback methods
    void OnAddGame(const char *player, dword gameid, const GameParams& params,
            dword minplayers, dword maxplayers, const char *title,
            dword ctotal);
    void OnRemoveGame(dword gameid, dword ctotal);
    void OnGameInProgress(dword gameid);
    void OnGamePlayerNames(dword gameid, dword cnames,
            const PlayerName *anames);
    void OnAddPlayer(const char *player);
    void OnRemovePlayer(dword hint, const char *player);
    void OnReceiveChat(const char *player, const char *chat);
    void OnStatusComplete();
    
    // ITransportCallback methods
    void OnStatusUpdate(char *pszStatus);    
    void OnShowMessage(const char *message);
    void OnConnectionClose();
   
    // ITimeout
    void OnTimeout(int id);

private:
    std::string GetPlayersString();
    void OnChatBlink(bool on);
    void OnPlayers();
    void HideShowJoinGame();
    void OnJoinGame();
    void OnCreateGame();
    void InitiateJoinGame(const GameInfo& info);
    void Refresh(int ct = 20);
    void GetString(GameInfo& info, char *s, int cb);
    int FindIndex(dword gameid);

    Chatter& chatter_;
    LoginHandler& handler_;
    TimeoutTimer timer_;
    GameMap map_;
    RoomInfo roominfo_;
    GameInfo joininfo_;
    bool refresh_;
    bool creator_;
    bool status_;
    std::vector<std::string> players_;
    std::string message_;
};
    
} // namespace wi

#endif // __ROOMFORM_H__
