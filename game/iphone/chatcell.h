#ifndef __CHATCELL_H__
#define __CHATCELL_H__

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <UIKit/UIView.h>
#import <UIKit/UIWindow.h>

@interface ChatCell : UITableViewCell {
    UILabel *nameLabel_;
    UILabel *chatLabel_;
}
- (void)setup:(NSString *)chat user:(NSString *)user size:(CGSize)size;
@end

#endif // __CHATCELL_H__
