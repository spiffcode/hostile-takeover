#ifndef __CHATVIEWCONTROLLER_H__
#define __CHATVIEWCONTROLLER_H__

#import <Foundation/Foundation.h>
#include "game/sdl/hosthelpers.h"

namespace wi {
class ChatController;
}

@interface ChatViewController : NSObject <NSTableViewDelegate, NSTableViewDataSource> {
    wi::ChatController *chatc_;
    wi::SurfaceProperties pprops_;
    
    NSWindow *window_;
    NSView *view_;
    
    NSTableView *tableView_;
    NSTableColumn *tableColumn_;
    
    NSMutableArray *chatEntries_;
    NSMutableArray *userEntries_;
    
    NSView *toolbar_;
    NSView *toolbar2_;
    
    NSString *title_;
    NSTextView *titleLabel_;
    
    NSTextField *textField_;
    NSFont *entryFont_;
    CGFloat width10Spaces_;
    BOOL clear_;
}

- (void)setController:(wi::ChatController *)chatc;
- (wi::ChatController *)controller;
@end

// +++

#include "base/thread.h"
#include "game/chatcontroller.h"
#include <string>

#define kidmOnSend 1
#define kidmOnDismissed 2
#define kidmOnPlayers 3

namespace wi {

struct ChatParams : base::MessageData {
    ChatParams(const std::string& chat) : chat(chat) {}
    std::string chat;
};

// Game thread c++ interface to the ChatViewController
class ChatController : public IChatController, public base::MessageHandler {
public:
    ChatController(ChatViewController *vcchat);

    // IChatController
    void Clear();
    void AddChat(const char *player, const char *chat);
    void Show(bool fShow);
    void SetTitle(const char *title);
    const char *GetTitle();
    IChatControllerCallback *SetCallback(IChatControllerCallback *pcccb);

private:
    // MessageHandler
    virtual void OnMessage(base::Message *pmsg);

    std::string title_;
    //WiViewController *vcwi_;
    ChatViewController *vcchat_;
    IChatControllerCallback *pcccb_;
};

} // namespace wi

#endif // __CHATVIEWCONTROLLER_H__