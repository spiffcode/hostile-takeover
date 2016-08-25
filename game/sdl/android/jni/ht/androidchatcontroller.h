#ifndef __ANDROID_CHATCONTROLLER_H__
#define __ANDROID_CHATCONTROLLER_H__

#include <string>
#include "game/sdl/hosthelpers.h"
#include "base/thread.h"
#include "game/chatcontroller.h"

#define kidmOnSend 1
#define kidmOnDismissed 2
#define kidmOnPlayers 3

namespace wi {

struct ChatParams : base::MessageData {
    ChatParams(const std::string& chat) : chat(chat) {}
    std::string chat;
};

// Game thread c++ interface to the ChatViewController
class AndroidChatController : public IChatController, public base::MessageHandler {
public:
    AndroidChatController();

    void Clear();
    void AddChat(const char *player, const char *chat);
    void Show(bool fShow);
    void SetTitle(const char *title);
    const char *GetTitle();
    IChatControllerCallback *SetCallback(IChatControllerCallback *pcccb);

private:
    // MessageHandler
    virtual void OnMessage(base::Message *pmsg);
};

} // namespace wi

#endif // __ANDROID_CHATCONTROLLER_H__
