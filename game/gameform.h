#ifndef __GAMEFORM_H__
#define __GAMEFORM_H__

#include "game/ht.h"
#include "game/roomform.h"
#include "game/chatter.h"
#include "mpshared/netmessage.h"
#include "base/misc.h"
#include "base/sigslot.h"
#include <vector>
#include <string>

namespace wi {

const dword knGameStartResultDone = 0;
const dword knGameStartResultFail = 1;
const dword knGameStartResultStart = 2;

STARTLABEL(GameStartResults)
    LABEL(knGameStartResultDone)
    LABEL(knGameStartResultFail)
    LABEL(knGameStartResultStart)
ENDLABEL(GameStartResults)
 
class GameForm : public ShellForm, ITransportCallback, IGameCallback,
        public base::has_slots<> {
public:
    GameForm(LoginHandler& handler, const GameInfo& info, bool creator,
            Chatter& chatter);
    virtual ~GameForm();

    static void ShowJoinMessage(dword result);
    static dword DoForm(LoginHandler& handler, const GameInfo& info,
            bool creator, Chatter& chatter);
        
    virtual bool DoModal(int *pnResult = NULL);
    virtual void OnControlSelected(word idc);
    virtual bool OnFilterEvent(Event *pevt);
    
	// ITransportCallback
    virtual void OnStatusUpdate(char *pszStatus);
    virtual void OnConnectionClose();
    virtual void OnShowMessage(const char *message);
   
    // IGameCallback
    virtual void OnReceiveChat(const char *player, const char *chat);
    virtual void OnNetMessage(NetMessage **ppnm);
    virtual void OnGameDisconnect();

    // Utility helpers
    static std::string GetPlayersString();
    static const char *GetColorName(int side);

private:
    static std::string GetPlayersObservingString();
    static std::string GetPlayersPlayingString();

    void OnChatBlink(bool on);
    void OnPlayers();
    void OnReadyBeginGame();
    void OnBeginGameFail();
    void OnPlayersUpdate(PlayersUpdateNetMessage *pnm);
    void OnGameParams(GameParamsNetMessage *pgpnm);
    bool IsExpectedGameParams(const GameParams& params);
    int GetPlayerCountNeeded(bool fReady);
    void ShowOrHideBeginGameButton();
    void RefreshPlayerList();
    std::vector<std::pair<Player *, bool> > GetPlayerReadies();
  
    Chatter& chatter_;
    LoginHandler& handler_; 
    const GameInfo& info_;
    bool creator_;
    bool joined_;
    std::string message_;
};
    
} // namespace wi

#endif // __GAMEFORM_H__
