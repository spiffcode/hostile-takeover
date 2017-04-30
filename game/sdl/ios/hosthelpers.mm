#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include <sys/stat.h>

#include "game/sdl/hosthelpers.h"
#include "game/sdl/ios/iphonehttpservice.h"
#include "base/thread.h"
#include "game/sdl/ios/iphone.h"

namespace wi {

char gszMainDataDir[PATH_MAX];          // data file (htdata832.pdb, htsfx.pdb) dir
char gszTempDir[PATH_MAX];              // temp file directory
char gszMissionPacksDir[PATH_MAX];      // downloaded mission packs
char gszMissionPackInfosDir[PATH_MAX];
char gszSaveGamesDir[PATH_MAX];         // saved games
char gszPrefsFilename[PATH_MAX];        // game prefs
char gszCompletesDir[PATH_MAX];         // for "mission completed" tracking
HttpService *gphttp;
IPhone *iphone;


bool HostHelpers::Init() {
    // Get directories off .app
    NSBundle *bundle = [NSBundle mainBundle];
    NSString *appDir = [bundle bundlePath];
    const char *pszAppDir = [appDir cStringUsingEncoding:
            [NSString defaultCStringEncoding]];
    sprintf(gszMainDataDir, "%s", pszAppDir);

    // This returns the user's home directory; Library, and Documents are in it
    NSString *homeDir = NSHomeDirectory();
    const char *pszHomeDir = [homeDir cStringUsingEncoding:
            [NSString defaultCStringEncoding]];

    sprintf(gszTempDir, "%s", [NSTemporaryDirectory() cStringUsingEncoding:
        [NSString defaultCStringEncoding]]);
    sprintf(gszMissionPacksDir, "%s/Library/MissionPacks", pszHomeDir);
    sprintf(gszMissionPackInfosDir, "%s/Library/MissionPackInfos", pszHomeDir);
    sprintf(gszSaveGamesDir, "%s/Library/SaveGames", pszHomeDir);
    sprintf(gszCompletesDir, "%s/Library/Completes", pszHomeDir);
    sprintf(gszPrefsFilename, "%s/Library/prefs.json", pszHomeDir);

    // Make the directories under Library
    mkdir(gszMissionPacksDir, 0755);
    mkdir(gszMissionPackInfosDir, 0755);
    mkdir(gszSaveGamesDir, 0755);
    mkdir(gszCompletesDir, 0755);

    // iPhone specific http service
    extern HttpService *gphttp;
    gphttp = (HttpService *)new IPhoneHttpService();
    
    // iPhone controller
    iphone = [IPhone sharedAppDelegate];
    [[UIApplication sharedApplication] setIdleTimerDisabled:YES];
    [iphone forceDeviceIntoLandscape];

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
    NSString *strURL = [NSString stringWithCString:pszUrl
            encoding:[NSString defaultCStringEncoding]];
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:strURL]];
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
    int cxScreen, cyScreen;

    float density = [[UIScreen mainScreen] scale];
    
    cxScreen = [UIScreen mainScreen].bounds.size.width;
    cyScreen = [UIScreen mainScreen].bounds.size.height;

    // On iOS <= 7 the screen size isn't orientation dependant
    if ([iphone deviceOS] <= 7) {
        cxScreen = [UIScreen mainScreen].bounds.size.height;
        cyScreen = [UIScreen mainScreen].bounds.size.width;
    }
    
    pprops->cxWidth = cxScreen;
    pprops->cyHeight = cyScreen;
    pprops->cbxPitch = 1;
    pprops->cbyPitch = pprops->cxWidth;
    pprops->ffFormat = wi::kfDirect8;
    pprops->density = density;
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
    [iphone initiateAsk:[NSString stringWithUTF8String:title] maxCharacters:max defaultString:[NSString stringWithUTF8String:def] keyboard:keyboard secure: secure ? YES : NO];
}

void HostHelpers::GetAskString(char *psz, int cb)
{
    const char *cString = [[iphone askString] cStringUsingEncoding:NSASCIIStringEncoding];
    wi::strncpyz(psz, cString, cb);
}

IChatController *HostHelpers::GetChatController()
{
    return [iphone getChatController];
}

void HostHelpers::InitiateWebView(const char *title, const char *url) {
    [iphone initiateWebView:[NSString stringWithUTF8String:url] title:[NSString stringWithUTF8String:title]];
}

const char *HostHelpers::GetPlatformString() {
    return [[iphone getPlatformString] cStringUsingEncoding:NSASCIIStringEncoding];
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

void HostHelpers::DisplayInitComplete() {
    // iOS 7 and under start in portrait and then need to be forced into landscape once the
    // SDL window is created
    [iphone forceDeviceIntoLandscape];
}

} // namespace wi
