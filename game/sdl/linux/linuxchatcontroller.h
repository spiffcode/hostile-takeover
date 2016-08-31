#ifndef __LINUXCHATCONTROLLER_H__
#define __LINUXCHATCONTROLLER_H__

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
class LinuxChatController : public IChatController, public base::MessageHandler {
public:
    LinuxChatController();

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

#endif // __LINUXCHATCONTROLLER_H__
