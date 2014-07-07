#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#include <sys/stat.h>

#if 0
#import <UIKit/UIKit.h>
#import <UIKit/UIWindow.h>
#import <UIKit/UIApplication.h>
#include <sys/types.h>

#include <dirent.h>
#include "iphone.h"
#include "input.h"
#include "base/log.h"
#include "base/thread.h"

#import "game/iphone/wiviewcontroller.h"
#import "game/iphone/chatviewcontroller.h"
#include "game/ht.h"

@interface IPhoneAppDelegate : NSObject <UIApplicationDelegate>
{
    UIWindow *m_window;
    WiViewController *m_vcwi;
    ChatViewController *m_vcchat;
    wi::IChatController *m_pchat;
    id m_view;
    base::Thread *m_game_thread;
    bool m_fExiting;
    char *m_pszUdid;
}
@end

- (void)initiateAsk:(const char *)title max:(int)max default:(const char *)def
        keyboard:(int)keyboard secure:(BOOL)secure
{
    return [m_vcwi initiateAsk:title max:max default:def keyboard:keyboard
            secure:secure];
}

- (void)getAskString:(char *)psz size:(int)cb
{
    return [m_vcwi getAskString:psz size:cb];
}

- (wi::IChatController *)getChatController
{
    if (m_pchat == NULL) {
        m_pchat = new wi::ChatController(m_vcwi, m_vcchat);
    }
    return m_pchat;
}

- (void)initiateWebView:(const char *)title withUrl:(const char *)url
{
    [m_vcwi initiateWebView:title withUrl:url];
}

#if 0
- (void)application:(UIApplication *)app
        willChangeStatusBarOrientation:(UIInterfaceOrientation)orientation
        duration:(NSTimeInterval)duration
{
    // This prevents the view from autorotating to portrait in the simulator
    if ((orientation == UIInterfaceOrientationPortrait) ||
            (orientation== UIInterfaceOrientationPortraitUpsideDown)) {
        [app setStatusBarOrientation:
                UIInterfaceOrientationLandscapeRight animated:NO];
    }
}
#endif

- (void)applicationDidFinishLaunching:(UIApplication *)application
{
    // Create the window and view
   
    wi::g_appDelegate = self;
    m_fExiting = false;

    // Set these here rather than in Info.plist, because devices with OS's
    // before iPhone OS 2.1 don't honor the Info.plist settings.
    // Hide the status bar. Unfortunately, the status bar area still
    // eat events. No known workaround currently. 
    [application setStatusBarHidden:YES animated:NO]; 

    // Tell the application object to turn off the screen dimming idle
    // timer.
    application.idleTimerDisabled = YES; 
    
    CGRect frame = [[UIScreen mainScreen] bounds];
    m_window = [[UIWindow alloc] initWithFrame: frame];

    // Create the Wi view controller, remember the view for shortcut purposes
    m_vcwi = [[WiViewController alloc] initWithNibName:nil bundle:nil];
    m_view = [[m_vcwi view] retain];

    m_vcchat = [[ChatViewController alloc] init:nil parent:m_view];
    m_pchat = NULL;
        
    // Show the window with table view
	[m_window addSubview:m_view];
    [m_window makeKeyAndVisible];

    // Must do this after makeKeyAndVisible, in order for it all to rotate
    [application setStatusBarOrientation:UIInterfaceOrientationLandscapeRight
            animated:NO];

    // Alloc the key directories
    [self allocPaths];

    // Grab the udid
    NSString *udid = [[UIDevice currentDevice] uniqueIdentifier];
    const char *pszUdid = [udid cStringUsingEncoding:
            [NSString defaultCStringEncoding]];
    m_pszUdid = (char *)malloc(strlen(pszUdid) + 1);
    strcpy(m_pszUdid, pszUdid);

    // Spin off a thread to run the game
    m_game_thread = new base::Thread();
    m_game_thread->Start(wi::HostHelpers::GameThreadStart, NULL);
    [m_vcwi setGameThread: m_game_thread];
}

- (wi::SpriteManager *)getSpriteManager
{
    return [m_view getSpriteManager];
}

- (void)setFormMgrs:(wi::FormMgr *)pfrmmSim input:(wi::FormMgr *)pfrmmInput
{
    [m_view setFormMgrs:pfrmmSim input:pfrmmInput];
}

- (const char *)getMissionPacksDir
{
    return m_pszMissionPacksDir;
}

- (const char *)getMissionPackInfosDir
{
    return m_pszMissionPackInfosDir;
}

- (const char *)getSaveGamesDir
{
    return m_pszSaveGamesDir;
}

- (const char *)getPrefsFilename
{
    return m_pszPrefsFilename;
}

- (const char *)getMainDataDir
{
    return m_pszMainDataDir;
}

- (const char *)getTempDir
{
    return m_pszTempDir;
}

- (const char *)getCompletesDir
{
    return m_pszCompletesDir;
}

- (const char *)getUdid
{
    return m_pszUdid;
}

- (bool)isExiting
{
    return m_fExiting;
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // WI is now active (app launch, device turned on / unlocked)

    m_game_thread->Post(kidmAppSetFocus, NULL);
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // WI is not active (app terminating, device turned off / locked)
    // If already exiting, don't do anything special.

    if (m_fExiting) {
        return;
    }

    m_game_thread->Post(kidmAppKillFocus, NULL);
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // This method is called when the user presses the device exit button.
    // Ask the game thread to exit.

    m_fExiting = true;
    m_game_thread->Post(kidmAppTerminate, NULL);

    // This will tell the thread to exit, and block until it does
    delete m_game_thread;
    m_game_thread = NULL;
}

- (void)getSurfaceProperties:(wi::SurfaceProperties *)pprops
{
    [m_view getSurfaceProperties:pprops];
}

- (void)frameStart
{
    [m_view frameStart];
}

- (void)frameComplete:(int)cfrmm maps:(wi::UpdateMap **)apupd
        rects:(wi::Rect *)arc scrolled:(bool)fScrolled
{
    [m_view frameComplete:cfrmm maps:apupd rects:arc scrolled:fScrolled];
}

- (void)resetScrollOffset
{
    [m_view resetScrollOffset];
}

- (wi::DibBitmap *)createFrontDibWithOrientation:(int)nDegreeOrientation
        width:(int)cx height:(int)cy
{
    return [m_view createFrontDibWithOrientation:nDegreeOrientation
            width:cx height:cy];
}

- (void)setPalette:(wi::Palette *)ppal
{
    [m_view setPalette:ppal];
}
@end

//
// C++ wrapper class around obj-c IPhoneApp class
//
#endif

#include "game/sdl/hosthelpers.h"
#include "game/sdl/mac/machttpservice.h"

namespace wi {

char gszMainDataDir[PATH_MAX];          // data file (htdata832.pdb, htsfx.pdb) dir
char gszTempDir[PATH_MAX];              // temp file directory
char gszMissionPacksDir[PATH_MAX];      // downloaded mission packs
char gszMissionPackInfosDir[PATH_MAX];
char gszSaveGamesDir[PATH_MAX];         // saved games
char gszPrefsFilename[PATH_MAX];        // game prefs
char gszCompletesDir[PATH_MAX];         // for "mission completed" tracking
HttpService *gphttp;

bool HostHelpers::Init() {
    // Get directories off .app
    NSBundle *bundle = [NSBundle mainBundle];
    NSString *appDir = [bundle bundlePath];
    const char *pszAppDir = [appDir cStringUsingEncoding:
            [NSString defaultCStringEncoding]];
    sprintf(gszMainDataDir, "%s/Contents/Resources", pszAppDir);

    // This returns the user's home directory; Library, and Documents are in it
    NSString *homeDir = NSHomeDirectory();
    const char *pszHomeDir = [homeDir cStringUsingEncoding:
            [NSString defaultCStringEncoding]];

    strcpy(gszTempDir, "/tmp");
    sprintf(gszMissionPacksDir, "%s/Library/MissionPacks", pszHomeDir);
    sprintf(gszMissionPackInfosDir, "%s/Library/MissionPackInfos", pszHomeDir);
    sprintf(gszSaveGamesDir, "%s/Library/SaveGames", pszHomeDir);
    sprintf(gszCompletesDir, "%s/Library/Completes", pszHomeDir);
    sprintf(gszPrefsFilename, "%s/Library/prefs.bin", pszHomeDir);

    // Make the directories under Library
    mkdir(gszMissionPacksDir, 0755);
    mkdir(gszMissionPackInfosDir, 0755);
    mkdir(gszSaveGamesDir, 0755);
    mkdir(gszCompletesDir, 0755);

    // Mac specific http service
    extern HttpService *gphttp;
    gphttp = (HttpService *)new MacHttpService;

    return true;
}

void HostHelpers::Cleanup() {
    delete gphttp;
}

const char *HostHelpers::GetMainDataDir() {
    return gszMainDataDir;
}

const char *HostHelpers::GetTempDir() {
	return gszTempDir;
}

const char *HostHelpers::GetMissionPacksDir() {
	return gszMissionPacksDir;
}

const char *HostHelpers::GetMissionPackInfosDir() {
	return gszMissionPackInfosDir;
}

const char *HostHelpers::GetSaveGamesDir() {
	return gszSaveGamesDir;
}

const char *HostHelpers::GetCompletesDir() {
	return gszCompletesDir;
}

const char *HostHelpers::GetPrefsFilename() {
	return gszPrefsFilename;
}

void HostHelpers::OpenUrl(const char *pszUrl) {
#if 0
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSString *s = [NSString stringWithCString:pszUrl
            encoding:[NSString defaultCStringEncoding]];
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:s]];
    [pool release];
#else
    Log("HostHelpers::OpenUrl not implemented yet");
#endif
}
    
void HostHelpers::Log(const char *pszFormat, va_list va)
{
#if 0
    LOGX() << base::Log::vFormat(pszFormat, va);
#endif

    char sz[512];
    vsnprintf(sz, sizeof(sz), pszFormat, va);

    printf("%s\n", sz);
}

void HostHelpers::Log(const char *pszFormat, ...)
{
    va_list va;
    va_start(va, pszFormat);
    Log(pszFormat, va);
    va_end(va);
}

void HostHelpers::MessageBox(const char *pszFormat, va_list va)
{
    Log(pszFormat, va);
}

void HostHelpers::Break()
{
    Log("BREAK!!");
}

void HostHelpers::GetSurfaceProperties(SurfaceProperties *pprops)
{
#if 0
    [g_appDelegate getSurfaceProperties: pprops];
#else
    Log("HostHelpers::GetSurfaceProperties not implemented yet");
#endif
}

void HostHelpers::FrameStart()
{
#if 0
    [g_appDelegate frameStart];
#else
    Log("HostHelpers::FrameStart not implemented yet");
#endif
}

void HostHelpers::FrameComplete(int cfrmm, UpdateMap **apupd, Rect *arc,
        bool fScrolled)
{
#if 0
    [g_appDelegate frameComplete:cfrmm maps:apupd rects:arc scrolled:fScrolled];
#else
    Log("HostHelpers::FrameComplete not implemented yet");
#endif
}

void HostHelpers::ResetScrollOffset()
{
#if 0
    [g_appDelegate resetScrollOffset];
#else
    Log("HostHelpers::ResetScrollOffset not implemented yet");
#endif
}

void HostHelpers::SetFormMgrs(FormMgr *pfrmmSim, FormMgr *pfrmmInput)
{
#if 0
    return [g_appDelegate setFormMgrs:pfrmmSim input:pfrmmInput];
#else
    Log("HostHelpers::SetFormMgrs not implemented yet");
#endif
}

DibBitmap *HostHelpers::CreateFrontDib(int cx, int cy, int nDegreeOrientation)
{
#if 0
    return [g_appDelegate createFrontDibWithOrientation:nDegreeOrientation width:cx height:cy];
#else
    Log("HostHelpers::CreateFrontDib not implemented yet");
    return NULL;
#endif
}
    
void HostHelpers::SetPalette(Palette *ppal)
{
#if 0
    [g_appDelegate setPalette:ppal];
#else
    Log("HostHelpers::SetPalette not implemented yet");
#endif
}

static char gszUdid[20];

const char *HostHelpers::GetUdid()
{
#if 0
    return [g_appDelegate getUdid];
#else
	// TODO(darrinm): talk to Scott about Udid requirements
	char *pch = gszUdid;
	if (*pch == 0) {
		for (int i = 0; i < 19; i++)
			*pch++ = '0' + GetAsyncRandom() % 10;
		*pch = 0;
	}
    return gszUdid;
#endif
}

bool HostHelpers::IsExiting()
{
#if 0
    return [g_appDelegate isExiting];
#else
    Log("HostHelpers::IsExitig not implemented yet");
    return false;
#endif
}

void HostHelpers::InitiateAsk(const char *title, int max, const char *def,
        int keyboard, bool secure)
{
#if 0
    BOOL s = secure ? YES : NO;
    return [g_appDelegate initiateAsk:title max:max default:def
            keyboard:keyboard secure:s];
#else
    Log("HostHelpers::InitiateAsk not implemented yet");
#endif
}

void HostHelpers::GetAskString(char *psz, int cb)
{
#if 0
    return [g_appDelegate getAskString:psz size:cb];
#else
    Log("HostHelpers::GetAskString not implemented yet");
#endif
}

IChatController *HostHelpers::GetChatController()
{
#if 0
    return [g_appDelegate getChatController];
#else
    Log("HostHelpers::GetChatController not implemented yet");
    return NULL;
#endif
}

void HostHelpers::InitiateWebView(const char *title, const char *url) {
#if 0
    [g_appDelegate initiateWebView:title withUrl:url];
#else
    Log("HostHelpers::InitiateWebView not implemented yet");
#endif
}

void HostHelpers::GameThreadStart(void *pv) {
    Log("Starting game...");
    wi::GameMain((char *)""); 
}

int HostHelpers::main(int argc, char **argv)
{
    // TODO(darrinm): should SDL call this?
#if 0
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    return UIApplicationMain(argc, argv, nil, @"IPhoneAppDelegate");
#else
    Log("HostHelpers::main not implemented yet");
#endif
    return 0;
}
    
} // namespace wi
