// Platform-specific entrypoints (e.g. mac/SDLMain.m) do early platform-specific
// setup and establish a uniform 'SDL' environment. Then they invoke SDL_main
// (see below) which does platform-independent SDL setup and calls the main
// game entrypoint wi::GameMain.
//
// Further platform-specific setup is done as the game calls Host* interfaces
// as it initializes.

#include "game/ht.h"
#include "game/sdl/sdleventserver.h"
#include "base/thread.h"

#if 0
/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void quit(int rc)
{
    SDL_Quit();
    exit(rc);
}
#endif

#ifdef __cplusplus
extern "C"
#endif
int SDL_main(int argc, char *argv[])
{
    // Set up the main thread as the SDL event thread.
    base::Thread::current().set_ss(new wi::SdlEventServer());

    // TODO(darrinm): pass args through
    wi::GameMain((char *)"");
    return 0;
}
