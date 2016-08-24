#import <UIKit/UIKit.h>

@interface Webview : UIView <UIWebViewDelegate> {
    UIActivityIndicatorView *activityView_;
    UIButton *backButton_;
    UIButton *forwardButton_;
    NSString *url_;
    NSString *title_;
    UIToolbar *toolbar_;
    UIWebView *contentView_;
}

- (void)loadDocument:(NSString *)url withTitle:(NSString *)title;
@end
