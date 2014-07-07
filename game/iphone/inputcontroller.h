#ifndef __INPUTCONTROLLER_H__
#define __INPUTCONTROLLER_H__

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <UIKit/UIView.h>
#import <UIKit/UIWindow.h>

@protocol InputDelgate;
@class InputController;

@protocol InputDelegate<NSObject>
- (void)onDone:(InputController *)c text:(NSString *)text;
@end

@interface InputController : NSObject <UITextFieldDelegate> {
    NSString *title_;
    NSString *default_string_;
    UIKeyboardType keyboard_type_;
    BOOL secure_;
    UIAlertView *alert_view_;
    int cchMax_;
    id<InputDelegate> delegate_;
}
- (id)init:(NSString *)title default:(NSString *)default_string
        keyboardType:(UIKeyboardType)keyboard_type delegate:(id)delegate
        maxChars:(int)cchMax secure:(BOOL)secure;
- (void)loadView;
@end

#endif // __INPUTCONTROLLER_H__
