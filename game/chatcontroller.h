#ifndef __CHATCONTROLLER_H__
#define __CHATCONTROLLER_H__

namespace wi {

class IChatControllerCallback {
public:
    virtual void OnChatDismissed() = 0;
    virtual void OnChatSend(const char *chat) = 0;
    virtual void OnPlayers() = 0;
};

class IChatController {
public:
    virtual void Clear() = 0;
    virtual void AddChat(const char *player, const char *chat) = 0;
    virtual void Show(bool fShow) = 0;
    virtual void SetTitle(const char *title) = 0;
    virtual const char *GetTitle() = 0;
    virtual IChatControllerCallback *SetCallback(
            IChatControllerCallback *pcccb) = 0;
};

} // namespace wi

#endif // __CHATCONTROLLER_H__
