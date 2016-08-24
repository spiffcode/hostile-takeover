#import <Cocoa/Cocoa.h>
#import "game/sdl/mac/AppDelegate.h"

#include "game/ht.h"
#include "game/sdl/sysmessages.h"
#include "base/thread.h"
#include "game/sdl/mac/SDLMain.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // MainMenu.xib is set via IB to not be visible on launch.
    // So SDL should be the only window open once the game starts
    
    // Start the game
    SDL_main();
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    SDL_Quit();
}

#if 0
// As OS X is a desktop where most users are adjusted to multi-window management,
// we probably don't need to suspend the game just because the window goes
// out of focus... The player probobably intends to let the game continute running
// as they do something else.

- (void)applicationDidBecomeActive:(NSNotification *)notification {
    // See host.cpp case SDL_APP_DIDENTERFOREGROUND
}

- (void)applicationWillResignActive:(NSNotification *)notification {
    // See host.cpp case SDL_APP_WILLENTERBACKGROUND
}
#endif

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

@end
