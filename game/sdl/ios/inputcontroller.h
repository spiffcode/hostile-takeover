#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface InputController : NSObject <UIAlertViewDelegate, UITextFieldDelegate>

- (UIAlertController *)alertControllerWithTitle:(NSString *)title maxCharacters:(int)max defaultString:(NSString *)def keyboard:(int)keyboard secure:(BOOL)secure;
- (NSString *)askString;
- (UIAlertView *)alertViewWithTitle:(NSString *)title maxCharacters:(int)max defaultString:(NSString *)def keyboard:(int)keyboard secure:(BOOL)secure;

@end
