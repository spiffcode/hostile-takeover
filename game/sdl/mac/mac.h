#ifndef __MAC_H__
#define __MAC_H__

#import <Foundation/Foundation.h>

@class InputController;
@class ChatViewController;

@interface Mac : NSObject {
    wi::IChatController *m_pchat;
    InputController *inputController;
    ChatViewController *chatViewController;
}

- (void)initiateAsk:(NSString *)title maxCharacters:(int)max defaultString:(NSString *)def keyboard:(int)keyboard secure:(BOOL)secure;
- (NSString *)askString;
- (wi::IChatController *)getChatController;
@end

#endif // __MAC_H__
