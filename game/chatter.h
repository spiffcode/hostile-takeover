#ifndef __CHATTER_H__
#define __CHATTER_H__

#include "game/ht.h"
#include "game/loginhandler.h"
#include "game/chatcontroller.h"
#include "base/sigslot.h"
#include <string>

namespace wi {

class Chatter : private IChatControllerCallback, private ITimeout {
public:
    Chatter(LoginHandler& handler);
    ~Chatter();

    void AddChat(const char *player, const char *chat, bool system);
    void ClearChat();
    void HideChat();
    void ShowChat();
    void SetChatTitle(const char *title);

    base::signal1<bool> SignalOnBlink;
    base::signal0<> SignalOnPlayers;

protected:
    void ShowChatButton(bool on);
    void StartBlinking();
    void StopBlinking();

    // IChatControllerCallback
    virtual void OnChatDismissed();
    virtual void OnChatSend(const char *chat);
    virtual void OnPlayers();

    // ITimeout
	virtual void OnTimeout(int id);
    
    std::string title_;
    LoginHandler& handler_;
    TimeoutTimer blinker_;
    IChatController *chatc_;
    IChatControllerCallback *old_;
    bool chatting_;
    bool on_;
};

} // namespace wi

#endif // __CHATTER_H__
