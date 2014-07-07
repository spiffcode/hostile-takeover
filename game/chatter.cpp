#include "game/chatter.h"

namespace wi {

Chatter::Chatter(LoginHandler& handler) : handler_(handler), chatting_(false),
        on_(true) {
    chatc_ = HostGetChatController();
    old_ = chatc_->SetCallback(this);
    ClearChat();
}

Chatter::~Chatter() {
    chatc_->SetCallback(old_);
    chatc_->Show(false);
}

void Chatter::AddChat(const char *player, const char *chat, bool system) {
    // Received chat from server, add it to the chat window
    if (system) {
        chatc_->AddChat("", chat);
    } else {
        chatc_->AddChat(player, chat);
        if (!chatting_) {
            char name[kcbPlayerName];
            handler_.GetPlayerName(name, sizeof(name));
            if (strcmp(name, player) != 0) {
                gsndm.PlaySfx(ksfxGuiCheckBoxTap);
                StartBlinking();
            }
        }
    }
}

void Chatter::ClearChat() {
    chatc_->Clear();
    StopBlinking();
}

void Chatter::ShowChat() {
    if (!chatting_) {
        chatc_->Show(true);
        chatting_ = true;
        StopBlinking();
    }
}

void Chatter::SetChatTitle(const char *title) {
    if (strcmp(title, title_.c_str()) != 0) {
        title_ = title;
        ClearChat();
        StopBlinking();
    }
    chatc_->SetTitle(title);
}

void Chatter::HideChat() {
    if (chatting_) {
        chatc_->Show(false);
        chatting_ = false;
    }
}

void Chatter::OnChatDismissed() {
    // Chat window was dismissed; remember this so next time chat comes in
    // the blinker will start.
    chatting_ = false;
}

void Chatter::OnChatSend(const char *chat) {
    // User pressed the Send button; send chat to the server
    if (gptra != NULL) {
        gptra->SendChat(chat);
    }
}

void Chatter::OnPlayers() {
    SignalOnPlayers();
}

void Chatter::OnTimeout(int id) {
    ShowChatButton(!on_);
}

void Chatter::ShowChatButton(bool on) {
    if (on != on_) {
        on_ = on;
        SignalOnBlink(on);
    }
}

void Chatter::StartBlinking() {
    blinker_.Start(this, 1000, 0, false);
}

void Chatter::StopBlinking() {
    blinker_.Stop();
    ShowChatButton(true);
}

} // namespace wi
