/*
 * Text Alert View
 *
 * File: TextAlertView.m
 * Abstract: UIAlertView extension with UITextField (Implementation).
 *
 * Pulled off the 'tubes. It's a hack because it subclasses the built in
 * AlertView and makes all sorts of assumptions.
 */
 
#import "game/iphone/textalertview.h"
 
@implementation TextAlertView
 
@synthesize textField_;
 
/*
 * Initialize view with maximum of two buttons
 */
- (id)initSpecial:(NSString *)title default:(NSString *)def
        keyboardType:(UIKeyboardType)keyboard secure:(BOOL)secure
        delegate:(id)delegate cancelButtonTitle:(NSString *)cancelButtonTitle
        otherButtonTitles:(NSString *)otherButtonTitles, ... {

	self = [super initWithTitle:title message:@"hideme" delegate:delegate
            cancelButtonTitle:cancelButtonTitle
			otherButtonTitles:otherButtonTitles, nil];

	if (self) {
		// Create and add UITextField to UIAlertView
		UITextField *myTextField = [[[UITextField alloc] initWithFrame:CGRectZero] retain];
		myTextField.autocorrectionType = UITextAutocorrectionTypeNo;
        myTextField.autocapitalizationType = UITextAutocapitalizationTypeNone;
        myTextField.keyboardAppearance = UIKeyboardAppearanceAlert;
        myTextField.keyboardType = keyboard;
		//myTextField.alpha = 0.75;
		myTextField.alpha = 1.0;
		myTextField.borderStyle = UITextBorderStyleRoundedRect;
		myTextField.delegate = delegate;
        myTextField.text = def;
        myTextField.secureTextEntry = secure;
		[self setTextField_:myTextField];
		// insert UITextField before first button
		for (UIView *view in self.subviews) {
			if (![view isKindOfClass:[UILabel class]]) {
				[self insertSubview:myTextField aboveSubview:view];
                break;
            }
		}

        // hide the message label - so only the title is visible
		for (UIView *view in self.subviews) {
			if ([view isKindOfClass:[UILabel class]]) {
                UILabel *label = (UILabel *)view;
                if (label.text == @"hideme") {
                    label.hidden = YES;
                    break;
                }
            }
		}

		layoutDone_ = NO;
 
		// add a transform to move the UIAlertView above the keyboard
		CGAffineTransform myTransform = CGAffineTransformMakeTranslation(0.0, kUIAlertOffset);
		[self setTransform:myTransform];
	}
	return self;
}
 
/*
 * Show alert view and make keyboard visible
 */
- (void) show {
	[super show];
	[[self textField_] becomeFirstResponder];
}
 
/*
 * Determine maximum y-coordinate of UILabel objects. This method assumes
 * that only following objects are contained in subview list:
 * - UILabel
 * - UITextField
 * - UIThreePartButton (Private Class)
 */
- (CGFloat) maxLabelYCoordinate {
	// Determine maximum y-coordinate of labels
	CGFloat maxY = 0;
	for (UIView *view in self.subviews) {
		if ([view isKindOfClass:[UILabel class]]) {
            if (view.hidden == YES) {
                continue;
            }
			CGRect viewFrame = [view frame];
			CGFloat lowerY = viewFrame.origin.y + viewFrame.size.height;
			if (lowerY > maxY) {
				maxY = lowerY;
            }
		}
	}
	return maxY;
}
 
/*
 *	Override layoutSubviews to correctly handle the UITextField
 */
- (void)layoutSubviews {
	[super layoutSubviews];
	CGRect frame = [self frame];
	CGFloat alertWidth = frame.size.width;
 
	// Perform layout of subviews just once
	if (!layoutDone_) {
		CGFloat labelMaxY = [self maxLabelYCoordinate];
 
        // Size the textField below labels
        CGRect textFieldFrame;
		for (UIView *view in self.subviews) {
		    if ([view isKindOfClass:[UITextField class]]) {
				textFieldFrame = CGRectMake(
                        kUITextFieldXPadding, 
                        labelMaxY + kUITextFieldYPadding, 
                        alertWidth - 2.0*kUITextFieldXPadding, 
                        kUITextFieldHeight);
				[view setFrame:textFieldFrame];
                break;
            }
        }

        // Keyboard is 180 high according to iPhone HIG. Center view within
        // the upper 180.

        frame.origin.y += 15;
        [self setFrame:frame];
		layoutDone_ = YES;
	} else {
        // reduce the x placement and width of the UITextField based on
        // UIAlertView width

		for (UIView *view in self.subviews) {
		    if ([view isKindOfClass:[UITextField class]]) {
				CGRect viewFrame = [view frame];
				viewFrame.origin.x = kUITextFieldXPadding;
				viewFrame.size.width = alertWidth - 2.0*kUITextFieldXPadding;
				[view setFrame:viewFrame];
		    }
		}
	}
}
@end
