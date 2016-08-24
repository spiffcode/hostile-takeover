#ifndef __CHATVIEW_H__
#define __CHATVIEW_H__

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <UIKit/UIView.h>
#import <UIKit/UIWindow.h>

namespace wi {
class ChatController;
}

@class ChatView;

@protocol ChatViewDelegate<NSObject>
- (void)onDone:(ChatView *)c;
@end

@interface ChatView : UIView <UITableViewDelegate,
    UITableViewDataSource, UITextFieldDelegate> {
    
    UITableView *tableView_;
    UIToolbar *toolbar_;
    UITextField *textField_;
    NSMutableArray *chatEntries_;
    UIFont *entryFont_;
    CGFloat width10Spaces_;
    wi::ChatController *chatc_;
    NSString *title_;
    UIToolbar *toolbar2_;
    UILabel *titleLabel_;
    NSMutableArray *chatQueue_;
    BOOL suspended_;
    BOOL clear_;
    UIBarButtonItem *keyboardButton_;
}
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
    ChatController(ChatView *vcchat);

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
    ChatView *vcchat_;
    IChatControllerCallback *pcccb_;
};

} // namespace wi

#endif // __CHATVIEW_H__