#import "game/iphone/inputcontroller.h"

@implementation InputController

- (id)init:(NSString *)title default:(NSString *)default_string
        keyboardType:(UIKeyboardType)keyboard_type delegate:(id)delegate
        maxChars:(int)cchMax secure:(BOOL)secure {

    title_ = title;
    [title_ retain]; 
    default_string_ = default_string;
    [default_string_ retain];
    keyboard_type_ = keyboard_type;
    delegate_ = delegate;
    [delegate_ retain];
    cchMax_ = cchMax;
    secure_ = secure;
    alert_view_ = nil;

    [self loadView];

    return self;
}

- (void)dealloc {
    [title_ release];
    [default_string_ release];
    [alert_view_ release];
    [delegate_ release];
    [super dealloc];
} 

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)way
{
    return way == UIInterfaceOrientationLandscapeRight;
}

- (void)loadView {
	UIAlertView *view = [[UIAlertView alloc] initWithTitle:title_
            message:nil delegate:self cancelButtonTitle:@"Cancel"
            otherButtonTitles:@"OK", nil];
    alert_view_ = view;
    alert_view_.alertViewStyle = secure_ ? UIAlertViewStyleSecureTextInput : UIAlertViewStylePlainTextInput;
    [alert_view_ textFieldAtIndex:0].delegate = self;
    [alert_view_ show];
}

- (void)alertView:(UIAlertView *)actionSheet
        clickedButtonAtIndex:(NSInteger)buttonIndex {
	if (buttonIndex > 0) {
        [delegate_ onDone:self text:[alert_view_ textFieldAtIndex:0].text];
    }
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
