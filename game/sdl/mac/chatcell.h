#ifndef __CHATCELL_H__
#define __CHATCELL_H__

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>

@interface ChatCell : NSView
+ (id)cellWithRect:(NSRect)rect user:(NSString *)user chat:(NSString *)chat;
@end

#endif // __CHATCELL_H__
