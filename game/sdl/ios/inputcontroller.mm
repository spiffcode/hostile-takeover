#import "game/sdl/ios/inputcontroller.h"

#include "base/thread.h"
#include "game/sdl/sysmessages.h"

@interface InputController()
@property (strong, nonatomic) NSString *textFieldText;
@property (strong, nonatomic) UIAlertController *alertController;
@property (strong, nonatomic) UIAlertView *alertView;
@property int maxTextfieldCharacters;
@end

@implementation InputController

// UIAlertControllers

- (UIAlertController *)alertControllerWithTitle:(NSString *)title maxCharacters:(int)max defaultString:(NSString *)def keyboard:(int)keyboard secure:(BOOL)secure {
    [self setAlertControllerWithTitle:title maxCharacters:max defaultString:def keyboard:keyboard secure:secure];
    
    return self.alertController;
}

- (void)setAlertControllerWithTitle:(NSString *)title maxCharacters:(int)max defaultString:(NSString *)def keyboard:(int)keyboard secure:(BOOL)secure {
    self.alertController = [self controllerWithTitle:title maxCharacters:max defaultString:def keyboard:keyboard secure:secure];
    self.maxTextfieldCharacters = max;
}

- (UIAlertController *)controllerWithTitle:(NSString *)title maxCharacters:(int)max defaultString:(NSString *)def keyboard:(int)keyboard secure:(BOOL)secure {
    UIAlertController *alertController = [UIAlertController alertControllerWithTitle:title message:@"" preferredStyle:UIAlertControllerStyleAlert];
    
    [alertController addTextFieldWithConfigurationHandler:^(UITextField *textField)
    {
     textField.placeholder = title;
     textField.secureTextEntry = secure ? YES : NO;
     [textField setKeyboardType:keyboard ? UIKeyboardTypeURL : UIKeyboardTypeASCIICapable];
     [textField setText:def];
     [textField addTarget:self
                   action:@selector(alertTextFieldDidChange:)
         forControlEvents:UIControlEventEditingChanged];
    }];
    
    UIAlertAction *cancelAction = [UIAlertAction actionWithTitle:@"Cancel" style:UIAlertActionStyleCancel
        handler:nil];

    UIAlertAction *okAction = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault
        handler:^(UIAlertAction *action) {
            UITextField *textField = (UITextField *)alertController.textFields.firstObject;
            self.textFieldText = textField.text;
            
            base::Thread::current().Post(wi::kidmAskStringEvent, NULL);
        }];
    [okAction setEnabled:([def length] == 0) ? NO : YES];
    [cancelAction setEnabled:YES];

    [alertController addAction:cancelAction];
    [alertController addAction:okAction];
    
    return alertController;

}

- (void)alertTextFieldDidChange:(UITextField *)sender {
    UIAlertController *alertController = (UIAlertController *)self.alertController;
    if (alertController) {
        UITextField *textField = alertController.textFields.firstObject;
        UIAlertAction *okAction = alertController.actions.lastObject;
        
        if (textField.text.length > self.maxTextfieldCharacters) {
            if (self.maxTextfieldCharacters != -1) {
                textField.text = [textField.text substringToIndex:[textField.text length] -1];
            }
        }
        
        [okAction setEnabled:textField.text.length > 0 ? YES : NO];
    }
}

- (NSString *)askString {
    return self.textFieldText;
}

// End UIAlertControllers

// UIAlertViews

- (UIAlertView *)alertViewWithTitle:(NSString *)title maxCharacters:(int)max defaultString:(NSString *)def keyboard:(int)keyboard secure:(BOOL)secure {
    [self setAlertViewWithTitle:title maxCharacters:max defaultString:def keyboard:keyboard secure:secure];
    
    return self.alertView;
}

- (void)setAlertViewWithTitle:(NSString *)title maxCharacters:(int)max defaultString:(NSString *)def keyboard:(int)keyboard secure:(BOOL)secure {
    self.alertView = [self viewWithTitle:title maxCharacters:max defaultString:def keyboard:keyboard secure:secure];
    self.maxTextfieldCharacters = max;
}

- (UIAlertView *)viewWithTitle:(NSString *)title maxCharacters:(int)max defaultString:(NSString *)def keyboard:(int)keyboard secure:(BOOL)secure {

    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:title message:@"" delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"OK", (NSString *)nil];

    [alertView setAlertViewStyle:secure ? UIAlertViewStyleSecureTextInput : UIAlertViewStylePlainTextInput];
    [alertView textFieldAtIndex:0].delegate = self;
    [[alertView textFieldAtIndex:0] setText:def];

    return alertView;
}

- (BOOL)textField:(UITextField *)textField
        shouldChangeCharactersInRange:(NSRange)range
        replacementString:(NSString *)string {
    if (self.maxTextfieldCharacters == -1) {
        return YES;
    }
    if (textField.text.length >= self.maxTextfieldCharacters && range.length == 0) {
        return NO;
    }
    return YES;
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
    if (buttonIndex > 0) {
        self.textFieldText = [alertView textFieldAtIndex:0].text;
        base::Thread::current().Post(wi::kidmAskStringEvent, NULL);
    }
}

// End UIAlertViews

@end
