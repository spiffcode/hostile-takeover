#include "linuxchatcontroller.h"
#include "game/sdl/hosthelpers.h"

namespace wi {

IChatControllerCallback *pcccb_;
wi::LinuxChatController *chatc_;

LinuxChatController::LinuxChatController() {
}

void LinuxChatController::Clear() {
}

void LinuxChatController::AddChat(const char *player, const char *chat) {
}

void LinuxChatController::Show(bool fShow) {
}

void LinuxChatController::SetTitle(const char *title) {
}

const char *LinuxChatController::GetTitle() {
}

IChatControllerCallback *LinuxChatController::SetCallback(
        IChatControllerCallback *pcccb) {
    IChatControllerCallback *old = pcccb_;
    pcccb_ = pcccb;
    return old;
}

void LinuxChatController::OnMessage(base::Message *pmsg) {
    switch (pmsg->id) {
    case kidmOnDismissed:
        if (pcccb_ != NULL) {
            pcccb_->OnChatDismissed();
        }
        break;

    case kidmOnSend:
        if (pcccb_ != NULL) {
            ChatParams *params = (ChatParams *)pmsg->data;
            pcccb_->OnChatSend(params->chat.c_str());
            delete params;
        }
        break;

    case kidmOnPlayers:
        if (pcccb_ != NULL) {
            pcccb_->OnPlayers();
        }
        break;
    }
}

} // namespace wi
