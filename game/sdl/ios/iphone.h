#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#import "game/sdl/ios/SDL_uikitappdelegate.h"

@class Webview;
@class InputController;
@class ChatView;

@interface IPhone : SDLUIKitDelegate
@property (strong, nonatomic) Webview *webView;
@property (strong, nonatomic) InputController *inputController;
@property (strong, nonatomic) ChatView *chatView;

- (void)initiateWebView:(NSString *)url title:(NSString *)title;
- (void)initiateAsk:(NSString *)title maxCharacters:(int)max defaultString:(NSString *)string keyboard:(int)keyboard secure:(BOOL)secure;
- (NSString *)askString;
- (wi::IChatController *)getChatController;
+ (void)presentViewController:(UIViewController* )viewController animated:(BOOL)animated completion:(void (^)(void))completion;
+ (void)presentView:(UIView *)view;
- (void)forceDeviceIntoLandscape;
- (int)deviceOS;
@end
