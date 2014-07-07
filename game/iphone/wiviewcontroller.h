#ifndef __WIVIEWCONTROLLER_H__
#define __WIVIEWCONTROLLER_H__

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <UIKit/UIView.h>
#import <UIKit/UIWindow.h>
#import <UIKit/UIApplication.h>
#import "game/iphone/inputcontroller.h"
#import "game/iphone/webviewcontroller.h"
#include "base/thread.h"

@interface WiViewController : UIViewController <InputDelegate,
        WebViewControllerDelegate> {
    char titleAsk_[512];
    char ask_[512];
    int keyboardAsk_;
    int maxAsk_;
    BOOL secureAsk_;
    base::Thread *game_thread_;
    InputController *input_controller_;
    WebViewController *web_controller_;
}
- (void)setGameThread:(base::Thread *)thread;
- (void)initiateWebView:(const char *)title withUrl:(const char *)url;
- (void)initiateAsk:(const char *)title max:(int)max default:(const char *)def
        keyboard:(int)keyboard secure:(BOOL)secure;
- (void)getAskString:(char *)psz size:(int)cb;
- (void)setGameThread:(base::Thread *)thread;
@end

#endif // __WIVIEWCONTROLLER_H__
