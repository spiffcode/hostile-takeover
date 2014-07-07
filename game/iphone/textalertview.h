/*
 *	Text Alert View
 *
 *  File: TextAlertView.h
 *	Abstract: UIAlertView extension with UITextField (Interface Declaration).
 *
 */

#ifndef __TEXTALERTVIEW_H__
#define __TEXTALERTVIEW_H__
 
#import <UIKit/UIKit.h>
 
#define kUITextFieldHeight 25.0
#define kUITextFieldXPadding 12.0
#define kUITextFieldYPadding 10.0
#define kUIAlertOffset 100.0
 
@interface TextAlertView : UIAlertView {
	UITextField *textField_;
	BOOL layoutDone_;
}
@property (nonatomic, retain) UITextField *textField_;

- (id)initSpecial:(NSString *)title default:(NSString *)def
        keyboardType:(UIKeyboardType)keyboardType secure:(BOOL)secure
        delegate:(id)delegate cancelButtonTitle:(NSString *)cancelButtonTitle
        otherButtonTitles:(NSString *)otherButtonTitles, ...;
@end

#endif // __TEXTALERTVIEW_H__
