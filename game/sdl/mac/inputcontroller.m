// Custom NSFormatter to limit amount of chacracters that can be typed in the input UI
@interface TextfieldFormatter : NSFormatter {
  int maxLength;
}

- (void)setMaximumLength:(int)len;
- (int)maximumLength;
@end

@implementation TextfieldFormatter

+ (id)formatterWithMaxLength:(int)len {
    TextfieldFormatter *textfieldFormatter = [[TextfieldFormatter alloc] init];
    [textfieldFormatter setMaximumLength:len];
    
    return textfieldFormatter;
}

- (void)setMaximumLength:(int)len {
  maxLength = len;
}

- (int)maximumLength {
  return maxLength;
}

- (BOOL)isPartialStringValid:(NSString **)partialStringPtr proposedSelectedRange:(NSRangePointer)proposedSelRangePtr originalString:(NSString *)origString originalSelectedRange:(NSRange)origSelRange errorDescription:(NSString **)error {
    
    if ([*partialStringPtr length] > maxLength) {
        return NO;
    } else {
        return YES;
    }
}

- (NSString *)stringForObjectValue:(id)object {
  return (NSString *)object;
}

- (BOOL)getObjectValue:(id *)object forString:(NSString *)string errorDescription:(NSString **)error {
  *object = string;
  return YES;
}

@end
// End TextFieldFormatter

// Begin InputController
#import "game/sdl/mac/inputcontroller.h"

@implementation InputController
- (id)init {
    title_ = nil;
    default_string_ = nil;
    secure_ = NO;
    alert_view_ = nil;
    cchMax_ = 32;
    ask_ = nil;
    
    return self;
}

- (void)showInputAlertWithTitle:(NSString *)title defaultText:(NSString *)default_string maxChars:(int)cchMax secure:(BOOL)secure {

    title_ = title;
    default_string_ = default_string;
    cchMax_ = cchMax;
    secure_ = secure;
    
    // Make an appropriately styled textfield
    NSTextField *textfield;
    if (secure) {
        textfield = [[NSSecureTextField alloc] initWithFrame:NSRectFromCGRect(CGRectMake(0, 0, 300, 25))];
    } else {
        textfield = [[NSTextField alloc] initWithFrame:NSRectFromCGRect(CGRectMake(0, 0, 300, 25))];
    }
    textfield.stringValue = default_string_;
    textfield.formatter = [TextfieldFormatter formatterWithMaxLength:cchMax_];
    
    
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:title_];
    [alert setAccessoryView:textfield];
    [alert addButtonWithTitle:@"Okay"];
    [alert addButtonWithTitle:@"Cancel"];
    if ([alert runModal] == NSAlertFirstButtonReturn) {
        ask_ = [[textfield stringValue] copy];
    } else {
        ask_ = default_string_;
    }
}

- (NSString *)askString {
    return ask_;
}
@end