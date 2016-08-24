#include <sys/param.h>
#include <unistd.h>

#include "SDL.h"
#include "SDLMain.h"
#include "game/sdl/hosthelpers.h"

// SDL2 will work without creating any NSApplications or AppDelegates.
// However, if we don't load MainMenu.xib, WI wont have menu bar items.
// So main calls HostHelpers::main which creats a Cocoa app and AppDelegate.
// Once loaded, the app delegate is responsible for making the call to SDL_main()
// to launch WI. At this point, argurments aren't passed through to WI.

int main(int argc, char *argv[])
{
    wi::HostHelpers::main(argc, argv);
}

int SDL_main()
{
    return SDL_main(0, NULL);
}
