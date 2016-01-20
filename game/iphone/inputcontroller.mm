#import "game/iphone/inputcontroller.h"

@implementation InputController

- (id)init:(NSString *)title default:(NSString *)default_string
        keyboardType:(UIKeyboardType)keyboard_type delegate:(id)delegate
        maxChars:(int)cchMax secure:(BOOL)secure controller:(UIViewController *)controller {

    title_ = title;
    [title_ retain]; 
    default_string_ = default_string;
    [default_string_ retain];
    keyboard_type_ = keyboard_type;
    delegate_ = delegate;
    [delegate_ retain];
    cchMax_ = cchMax;
    secure_ = secure;

    [self loadAlert:controller];

    return self;
}

- (void)dealloc {
    [title_ release];
    [default_string_ release];
    [delegate_ release];
    [super dealloc];
} 

- (void)loadAlert:(UIViewController *)controller {
    UIAlertController *alert = [UIAlertController
            alertControllerWithTitle:title_
            message:nil
            preferredStyle:UIAlertControllerStyleAlert];

    [alert addTextFieldWithConfigurationHandler:^(UITextField *text_field) {
                text_field.text = default_string_;
                text_field.keyboardType = keyboard_type_;
                text_field.autocapitalizationType = UITextAutocapitalizationTypeNone;
                text_field.autocorrectionType = UITextAutocorrectionTypeNo;
                text_field.spellCheckingType = UITextSpellCheckingTypeNo;
                text_field.enablesReturnKeyAutomatically = YES;
                if (secure_) {
                    [text_field setSecureTextEntry:YES];
                }
                text_field.delegate = self;
            }];

    UIAlertAction *cancel_action = [UIAlertAction
            actionWithTitle:@"Cancel"
            style:UIAlertActionStyleDefault
            handler:^(UIAlertAction *action) {
            }];
    [alert addAction:cancel_action];

    UIAlertAction *ok_action = [UIAlertAction
            actionWithTitle:@"OK"
            style:UIAlertActionStyleDefault
            handler:^(UIAlertAction *action) {
                UITextField *text_field = [alert.textFields firstObject];
                [delegate_ onDone:self text:text_field.text];
            }];
    [alert addAction:ok_action];

    [controller presentViewController:alert animated:YES completion:nil];
    UITextField *text_field = [alert.textFields firstObject];
    [text_field becomeFirstResponder];
}

- (BOOL)textField:(UITextField *)textField
        shouldChangeCharactersInRange:(NSRange)range
        replacementString:(NSString *)string {
    if (cchMax_ == -1) {
        return YES;
    }
    if (textField.text.length >= cchMax_ && range.length == 0) {
        return NO;
    }
    return YES;
}
@end
