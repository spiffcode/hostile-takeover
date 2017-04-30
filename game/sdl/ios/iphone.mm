#import "game/sdl/ios/SDL_uikitappdelegate.h"

// Catigorize the SDLUIKitDelegate class and override the method that
// loads the SDL_AppDelegate to load the subclassed IPhone class instead.

@implementation SDLUIKitDelegate (LoadIPhoneAppDelegate)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-protocol-method-implementation"
+ (NSString *)getAppDelegateClassName {
    return @"IPhone";
}
#pragma clang diagnostic pop

@end

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include "game/ht.h"
#include "base/log.h"
#include "base/thread.h"
#include "game/sdl/sysmessages.h"
#include "game/sdl/hosthelpers.h"

#import "SDL.h"
#import "game/sdl/ios/iphone.h"
#import "game/sdl/ios/webview.h"
#import "game/sdl/ios/inputcontroller.h"
#import "game/sdl/ios/chatview.h"

#define DEVICE_OS [[[UIDevice currentDevice] systemVersion] intValue]

@interface IPhone() {
    wi::IChatController *m_pchat;
}
@end

@implementation IPhone

- (id)init {
    if (![super init])
        return NULL;

    m_pchat = NULL;
    
    self.webView = [[Webview alloc] init];
    self.inputController = [[InputController alloc] init];
    self.chatView = [[ChatView alloc] init];

    return self;
}

+ (NSString *)getAppDelegateClassName {
    return @"IPhoneAppDelegate";
}

- (void)applicationWillTerminate:(UIApplication *)application {
    [super applicationWillTerminate:application];
    wi::HostAppStop();
}

+ (void)presentViewController:(UIViewController* )viewController animated:(BOOL)animated completion:(void (^)(void))completion {
    // When this is done, the ViewController isn't in the window's hierarchy. Only use this method
    // if you cannot use [self presentView:] (i.e. for this only for UIAlertControllers)
    [[[[UIApplication sharedApplication] keyWindow] rootViewController] presentViewController:viewController animated:animated completion:completion];
}

+ (void)presentView:(UIView *)view {
    // When portrait mode is enabled for the app, somewhere between SDL's window management
    // and iOS, the true orientation is lost. Normally iOS returns an unknown orientation,
    // which essentially has portrait bounds and size. However, most of the time we're using
    // landscape mode. So we will assume that [self topView] is WI's view and resize
    // any view we want to present to that. Any of these views need to be able to resize their
    // content based on that.
    // Disabling portrait mode for the app altogether would be the best option, but if that is
    // done, we cannot use some of Apple's views (i.e. UIImagePickerView to allow users to set
    // their avatar).
    [view setFrame:[self topView].frame];

    // Devices running <= iOS 7 need to use screen size insteado of topView.frame
    if (DEVICE_OS <= 7) {
        int cxScreen, cyScreen;

        wi::SurfaceProperties props;
        wi::HostHelpers::GetSurfaceProperties(&props);
        cxScreen = props.cxWidth;
        cyScreen = props.cyHeight;
        
        [view setFrame:CGRectMake(0, 0, cxScreen, cyScreen)];
    }

    [[self topView] addSubview:view];
}

- (void)initiateWebView:(NSString *)url title:(NSString *)title {
    [self.webView loadDocument:url withTitle:title];
    [IPhone presentView:self.webView];
}

- (void)initiateAsk:(NSString *)title maxCharacters:(int)max defaultString:(NSString *)def keyboard:(int)keyboard secure:(BOOL)secure {

    if ([UIAlertController class]) {
        [IPhone presentViewController:[self.inputController alertControllerWithTitle:title maxCharacters:max defaultString:def keyboard:keyboard secure:secure] animated:YES completion:nil];
    } else {
        UIAlertView *alertView = [self.inputController alertViewWithTitle:title maxCharacters:max defaultString:def keyboard:keyboard secure:secure];
        [alertView show];
    }
}

- (NSString *)askString {
    return [self.inputController askString];
}

- (wi::IChatController *)getChatController {
    if (m_pchat == NULL) {
        m_pchat = new wi::ChatController(self.chatView);
    }
    return m_pchat;
}

+ (UIView *)topView {
    UIWindow *keyWindow = [[UIApplication sharedApplication] keyWindow];

    // This *should* get the topmost view
    UIView *topView;
    for (UIView *subview in keyWindow.subviews) {
        topView = subview;
    }

    return topView;
}

- (void)forceDeviceIntoLandscape {
    if ([[UIApplication sharedApplication] respondsToSelector:@selector(setStatusBarOrientation:)]) {
        [[UIApplication sharedApplication] setStatusBarOrientation:UIInterfaceOrientationLandscapeRight];
    }

    [[UIDevice currentDevice] setValue:
        [NSNumber numberWithInteger:
        UIInterfaceOrientationLandscapeRight]
        forKey:@"orientation"];
}

- (int)deviceOS {
    return DEVICE_OS;
}

- (NSString *)getPlatformString {
    return [NSString stringWithFormat:@"%@ %@",
        [[UIDevice currentDevice] systemName],
        [[UIDevice currentDevice] systemVersion]];
}

@end
