#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <UIKit/UIWindow.h>
#import <UIKit/UIApplication.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
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
    char *m_pszMissionPacksDir;
    char *m_pszMissionPackInfosDir;
    char *m_pszSaveGamesDir;
    char *m_pszPrefsFilename;
    char *m_pszMainDataDir;
    char *m_pszTempDir;
    char *m_pszCompletesDir;
    char *m_pszUUID;
}
@end

namespace wi {
IPhoneAppDelegate *g_appDelegate;
}

@implementation IPhoneAppDelegate

+ (void)initialize
{
}

- (void)allocPath:(char **)ppsz baseDir:(const char *)baseDir
        subDir:(const char *)subDir
{
    int cb = strlen(baseDir) + strlen(subDir) + 1;
    char *psz = (char *)malloc(cb);
    strcpy(psz, baseDir);
    strcat(psz, subDir);
    *ppsz = psz;
}

- (void)allocPaths
{
    // .app -> main app directory
    // .app/htdata832.pdb, htsfx.pdb -> main data files
    // Library/MissionPacks/ -> downloaded mission packs
    // Library/SaveGames/ -> saved games
    // Library/prefs.bin -> game prefs
    // tmp -> tmp directory

    // This returns the home directory; .app, Library, and Documents are in it
    NSString *homeDir = NSHomeDirectory();
    const char *pszHomeDir = [homeDir cStringUsingEncoding:
            [NSString defaultCStringEncoding]];

    [self allocPath:&m_pszMissionPacksDir baseDir:pszHomeDir
            subDir:"/Library/MissionPacks"];
    [self allocPath:&m_pszMissionPackInfosDir baseDir:pszHomeDir
            subDir:"/Library/MissionPackInfos"];
    [self allocPath:&m_pszSaveGamesDir baseDir:pszHomeDir
            subDir:"/Library/SaveGames"];
    [self allocPath:&m_pszPrefsFilename baseDir:pszHomeDir
            subDir:"/Library/prefs.bin"];
    [self allocPath:&m_pszTempDir baseDir:pszHomeDir subDir:"/tmp"];
    [self allocPath:&m_pszCompletesDir baseDir:pszHomeDir
            subDir:"/Library/Completes"];

    // Make the directories under Library
    mkdir(m_pszMissionPacksDir, 0755);
    mkdir(m_pszMissionPackInfosDir, 0755);
    mkdir(m_pszSaveGamesDir, 0755);
    mkdir(m_pszCompletesDir, 0755);

    // Get directories off .app
    NSBundle *bundle = [NSBundle mainBundle];
    NSString *appDir = [bundle bundlePath];
    const char *pszAppDir = [appDir cStringUsingEncoding:
            [NSString defaultCStringEncoding]];

    [self allocPath:&m_pszMainDataDir baseDir:pszAppDir
            subDir:""];

#if 0
    NSArray *pathsLibrary = NSSearchPathForDirectoriesInDomains(
            NSLibraryDirectory, NSUserDomainMask, YES);
    NSString *libraryDirectory = [pathsLibray objectAtIndex:0];
#endif
}

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
    // NOTE: This is fixed on iOS >= 7 by the implementation of prefersStatusBarHidden
    // in wiviewcontroller.mm.
    [application setStatusBarHidden:YES animated:NO];

    // Tell the application object to turn off the screen dimming idle
    // timer.
    application.idleTimerDisabled = YES; 
    
    CGRect frame = [[UIScreen mainScreen] bounds];
    m_window = [[UIWindow alloc] initWithFrame: frame];

    // Create the Wi view controller, remember the view for shortcut purposes
    m_vcwi = [[WiViewController alloc] initWithNibName:nil bundle:nil];
	m_window.rootViewController = m_vcwi;
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

    // Generate a UUID and keep around for later query
    CFUUIDRef uuid = CFUUIDCreate(NULL);
    NSString *strUUID = (NSString *)CFUUIDCreateString(NULL, uuid);
    CFRelease(uuid);
    const char *pszUUID = [strUUID cStringUsingEncoding:
            [NSString defaultCStringEncoding]];
    m_pszUUID = (char *)malloc(strlen(pszUUID) + 1);
    strcpy(m_pszUUID, pszUUID);
    CFRelease(strUUID);

    // Spin off a thread to run the game
    m_game_thread = new base::Thread();
    m_game_thread->Start(wi::IPhone::GameThreadStart, NULL);
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

- (const char *)staticUUID
{
    return m_pszUUID;
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

namespace wi {

void IPhone::OpenUrl(const char *pszUrl) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSString *s = [NSString stringWithCString:pszUrl
            encoding:[NSString defaultCStringEncoding]];
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:s]];
    [pool release];
}
    
void IPhone::Log(char *pszFormat, va_list va)
{
#if 0
    LOGX() << base::Log::vFormat(pszFormat, va);
#endif

    char sz[512];
    vsnprintf(sz, sizeof(sz), pszFormat, va);

#ifdef SIMULATOR
    printf("%s\n", sz);
#else
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSString *s = [NSString stringWithCString:sz
            encoding:[NSString defaultCStringEncoding]];
    NSLog(s);
    [pool release];
#endif
}

void IPhone::Log(char *pszFormat, ...)
{
    va_list va;
    va_start(va, pszFormat);
    Log(pszFormat, va);
    va_end(va);
}

void IPhone::MessageBox(char *pszFormat, va_list va)
{
    Log(pszFormat, va);
}

void IPhone::Break()
{
    Log("BREAK!!");
}

void IPhone::GetSurfaceProperties(SurfaceProperties *pprops)
{
    [g_appDelegate getSurfaceProperties: pprops];
}

void IPhone::FrameStart()
{
    [g_appDelegate frameStart];
}

void IPhone::FrameComplete(int cfrmm, UpdateMap **apupd, Rect *arc,
        bool fScrolled)
{
    [g_appDelegate frameComplete:cfrmm maps:apupd rects:arc scrolled:fScrolled];
}

void IPhone::ResetScrollOffset()
{
    [g_appDelegate resetScrollOffset];
}

SpriteManager *IPhone::GetSpriteManager()
{
    return [g_appDelegate getSpriteManager];
}

void IPhone::SetFormMgrs(FormMgr *pfrmmSim, FormMgr *pfrmmInput)
{
    return [g_appDelegate setFormMgrs:pfrmmSim input:pfrmmInput];
}

DibBitmap *IPhone::CreateFrontDib(int cx, int cy, int nDegreeOrientation)
{
    return [g_appDelegate createFrontDibWithOrientation:nDegreeOrientation width:cx height:cy];
}
    
void IPhone::SetPalette(Palette *ppal)
{
    [g_appDelegate setPalette:ppal];
}

const char *IPhone::GetMissionPacksDir()
{
    return [g_appDelegate getMissionPacksDir];
}

const char *IPhone::GetMissionPackInfosDir()
{
    return [g_appDelegate getMissionPackInfosDir];
}

const char *IPhone::GetSaveGamesDir()
{
    return [g_appDelegate getSaveGamesDir];
}

const char *IPhone::GetPrefsFilename()
{
    return [g_appDelegate getPrefsFilename];
}

const char *IPhone::GetMainDataDir()
{
    return [g_appDelegate getMainDataDir];
}

const char *IPhone::GetTempDir()
{
    return [g_appDelegate getTempDir];
}

const char *IPhone::GetCompletesDir()
{
    return [g_appDelegate getCompletesDir];
}

const char *IPhone::GetStaticUUID()
{
    return [g_appDelegate staticUUID];
}

bool IPhone::IsExiting()
{
    return [g_appDelegate isExiting];
}

void IPhone::InitiateAsk(const char *title, int max, const char *def,
        int keyboard, bool secure)
{
    BOOL s = secure ? YES : NO;
    return [g_appDelegate initiateAsk:title max:max default:def
            keyboard:keyboard secure:s];
}

void IPhone::GetAskString(char *psz, int cb)
{
    return [g_appDelegate getAskString:psz size:cb];
}

IChatController *IPhone::GetChatController()
{
    return [g_appDelegate getChatController];
}

void IPhone::InitiateWebView(const char *title, const char *url) {
    [g_appDelegate initiateWebView:title withUrl:url];
}

void IPhone::GameThreadStart(void *pv) {
    NSLog(@"Starting game...");
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    wi::GameMain(""); 
    [pool release];
}

int IPhone::main(int argc, char **argv)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    return UIApplicationMain(argc, argv, nil, @"IPhoneAppDelegate");
}
    
} // namespace wi

