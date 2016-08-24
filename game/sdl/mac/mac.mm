#include "game/ht.h"
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include "base/log.h"
#include "base/thread.h"

#import "game/sdl/mac/mac.h"
#import "game/sdl/mac/inputcontroller.h"
#import "game/sdl/mac/chatviewcontroller.h"

@implementation Mac

- (id)init {
    m_pchat = NULL;
    
    inputController = [[InputController alloc] init];
    chatViewController = [[ChatViewController alloc] init];
    
    return self;
}

- (void)initiateAsk:(NSString *)title maxCharacters:(int)max defaultString:(NSString *)def keyboard:(int)keyboard secure:(BOOL)secure {
    [inputController showInputAlertWithTitle:title defaultText:def maxChars:max secure:secure];
}

- (NSString *)askString {
    return [inputController askString];
}

- (wi::IChatController *)getChatController {
    if (m_pchat == NULL) {
        m_pchat = new wi::ChatController(chatViewController);
    }
    return m_pchat;
}

@end
