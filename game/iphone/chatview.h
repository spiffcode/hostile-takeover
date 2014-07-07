#ifndef __CHATVIEW_H__
#define __CHATVIEW_H__

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <UIKit/UIView.h>
#import <UIKit/UIWindow.h>

@interface ChatView : UIView {
    UINavigationBar *navBar_;
    UINavigationItem *navItem_;
    CGRect rect_;
    //UITextField *textField_;
}
- (id)initWithFrame:(CGRect)rect;
- (void)dealloc;
- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event;
@end

#endif // __CHATVIEW_H__
