#ifndef __WEBVIEWCONTROLLER_H__
#define __WEBVIEWCONTROLLER_H__

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <UIKit/UIView.h>
#import <UIKit/UIWindow.h>

@class WebViewController;

@protocol WebViewControllerDelegate<NSObject>
- (void)onDone:(WebViewController *)c;
@end

@interface WebViewController : NSObject <UIWebViewDelegate> {
    UIView *parent_;
    UIView *view_;
    UIWebView *webView_;
    UIActivityIndicatorView *activityView_;
    UIButton *backButton_;
    UIButton *forwardButton_;
    id<WebViewControllerDelegate> delegate_;
}
- (void)loadView;
- (void)show;
- (void)loadDocument:(NSString *)url withTitle:(NSString *)title;
- (void)setDocument:(NSString *)doc;
- (id)init:(id<WebViewControllerDelegate>)delegate parent:(UIView *)parent;
@end

#endif // __WEBVIEWCONTROLLER_H__
