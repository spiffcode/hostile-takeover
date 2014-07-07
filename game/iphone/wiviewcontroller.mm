#import "game/iphone/wiviewcontroller.h"
#import "game/iphone/cgview.h"
#import "game/iphone/chatviewcontroller.h"
#import "game/iphone/webviewcontroller.h"
#include "game/iphone/input.h"

@implementation WiViewController

- (id)initWithNibName:(NSString *)nibName bundle:(NSBundle *)bundle
{
    self = [super initWithNibName:nibName bundle:bundle];
    if (self == nil) {
        return nil;
    }
    input_controller_ = nil;
    game_thread_ = NULL;
    web_controller_ = nil;
    return self;
}

- (void)loadView
{
#if 1
    // As of iOS7 applicationFrame subtracts the status bar height. So we use the full screen bounds instead.
    CGRect frame = [[UIScreen mainScreen] bounds];
#else
    CGRect frame = [[UIScreen mainScreen] applicationFrame];
#endif
    UIView *view = [[CgView alloc] initWithFrame: frame];
    view.autoresizesSubviews = YES;
    view.autoresizingMask = (UIViewAutoresizingFlexibleWidth |
            UIViewAutoresizingFlexibleHeight);
    self.view = view;

#if 0
    UIButton *button = [UIButton buttonWithType:UIButtonTypeRoundedRect];
    [button setTitle:@"Hello world!" forState:UIControlStateNormal];
    frame = button.frame;
    frame.origin.x = 0;
    frame.origin.y = 0;
    frame.size.width = 100;
    frame.size.height = 100;
    button.frame = frame;
    [self.view addSubview:button];
#endif
}

// DEPRECATED in iOS 6.0. No longer sufficient to get correct orientation on iOS 7.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)way
{
    return way == UIInterfaceOrientationLandscapeRight;
}

- (BOOL)shouldAutorotate {
    return YES;
}

- (NSUInteger)supportedInterfaceOrientations {
    return UIInterfaceOrientationMaskLandscape;
}

- (UIInterfaceOrientation)preferredInterfaceOrientationForPresentation {
    return UIInterfaceOrientationLandscapeRight;
}

- (BOOL)prefersStatusBarHidden {
    return YES;
}

- (void)setGameThread:(base::Thread *)thread
{
    game_thread_ = thread;
    [(WiView *)(self.view) setGameThread:thread];
}

- (void)doShowWebView:(NSDictionary *)dict
{
    // Alloc web view controller
    web_controller_ = [[WebViewController alloc] init:self parent:self.view];

    [web_controller_ show];

    // Tell it to start loading the document
    NSString *url = [dict objectForKey:@"url"];
    NSString *title = [dict objectForKey:@"title"];
    [web_controller_ loadDocument:url withTitle:title];
}

- (void)onDone:(WebViewController *)c {
    [web_controller_ autorelease];
    web_controller_ = nil;
}

- (void)initiateWebView:(const char *)title withUrl:(const char *)url
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSString *title_s = [NSString stringWithCString:title
            encoding:[NSString defaultCStringEncoding]];
    NSString *url_s = [NSString stringWithCString:url
            encoding:[NSString defaultCStringEncoding]];
    NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
            title_s, @"title", url_s, @"url", (char *)NULL];
    [self performSelectorOnMainThread:@selector(doShowWebView:)
            withObject:dict waitUntilDone:NO];
    [pool release];
}

- (void)doAsk
{
    NSString *title = [NSString stringWithCString:titleAsk_
            encoding:[NSString defaultCStringEncoding]];
    NSString *def = [NSString stringWithCString:ask_
            encoding:[NSString defaultCStringEncoding]];

    UIKeyboardType keyboardType;
    switch (keyboardAsk_) {
    //case knKeyboardAskURL:
    case 1:
        keyboardType = UIKeyboardTypeURL;
        break;

    // case knKeyboardAskDefault:
    case 0:
    default:
        keyboardType = UIKeyboardTypeASCIICapable;
        break;
    }

    InputController *input_controller = [[InputController alloc]
            init:title default:def keyboardType:keyboardType
            delegate:self maxChars:maxAsk_ secure:secureAsk_];
    input_controller_ = input_controller;
}

- (void)onDone:(InputController *)c text:(NSString *)text {
    const char *pszText = [text cStringUsingEncoding:
            [NSString defaultCStringEncoding]];
    wi::strncpyz(ask_, pszText, sizeof(ask_));
    [input_controller_ autorelease];
    input_controller_ = nil;

    // Alerts are modeless, so this hack is needed to notify
    // the game thread
    if (game_thread_ != NULL) {
        game_thread_->Post(kidmAskStringEvent, NULL);
    }
}

- (void)getAskString:(char *)psz size:(int)cb
{
    wi::strncpyz(psz, ask_, cb);
}

- (void)initiateAsk:(const char *)title max:(int)max default:(const char *)def
        keyboard:(int)keyboard secure:(BOOL)secure
{
    // Called by the game to retrieve an URL from the user using native UI.
    // Ask the main thread to show the UI. The UI isn't modal so the result
    // needs to be retrieved later

    wi::strncpyz(titleAsk_, title, sizeof(titleAsk_));
    wi::strncpyz(ask_, def, sizeof(ask_));
    keyboardAsk_ = keyboard;
    maxAsk_ = max;
    secureAsk_ = secure;

    [self performSelectorOnMainThread:@selector(doAsk)
            withObject:nil waitUntilDone: NO];
}
@end
