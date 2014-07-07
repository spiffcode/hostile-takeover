#ifndef __CHATVIEWCONTROLLER_H__
#define __CHATVIEWCONTROLLER_H__

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <UIKit/UIView.h>
#import <UIKit/UIWindow.h>

namespace wi {
class ChatController;
}

@class ChatViewController;

@protocol ChatViewControllerDelegate<NSObject>
- (void)onDone:(ChatViewController *)c;
@end

@interface ChatViewController : NSObject <UITableViewDelegate,
        UITableViewDataSource, UITextFieldDelegate> {
    UIView *parent_;
    UIView *view_;
    id<ChatViewControllerDelegate> delegate_;
    UITableView *tableView_;
    UIToolbar *toolbar_;
    UITextField *textField_;
    bool keyboardShown_;
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
}
- (id)init:(id<ChatViewControllerDelegate>)delegate parent:(UIView *)parent;
- (void)loadView;
- (void)show;
- (void)layoutViews;
- (void)registerNotifications;
- (void)testChat;
- (int)getRowHeight:(int)rowIndex forWidth:(int)width;
- (void)reCreateTableView;
- (void)layoutTableView;
- (void)suspendUpdates:(BOOL)suspend;
- (NSArray *)updateChatModel;
@end

// +++

#import <game/iphone/wiviewcontroller.h>
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
    ChatController(WiViewController *vcwi, ChatViewController *vcchat);

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
    WiViewController *vcwi_;
    ChatViewController *vcchat_;
    IChatControllerCallback *pcccb_;
};

} // namespace wi

#endif // __CHATVIEWCONTROLLER_H__
