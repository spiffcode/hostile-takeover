#ifndef __INPUTCONTROLLER_H__
#define __INPUTCONTROLLER_H__

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>

@interface InputController : NSObject <NSAlertDelegate, NSTextFieldDelegate> {
    NSString *title_;
    NSString *default_string_;
    BOOL secure_;
    NSAlert *alert_view_;
    int cchMax_;
    NSString *ask_;
}

- (void)showInputAlertWithTitle:(NSString *)title defaultText:(NSString *)default_string maxChars:(int)cchMax secure:(BOOL)secure;
- (NSString *)askString;
@end

#endif // __INPUTCONTROLLER_H__
