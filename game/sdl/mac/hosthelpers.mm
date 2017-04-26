#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#include <sys/stat.h>

#include "game/sdl/hosthelpers.h"
#include "game/sdl/mac/machttpservice.h"
#include "base/thread.h"
#include "game/sdl/mac/mac.h"
#include "game/sdl/mac/AppDelegate.h"

namespace wi {

char gszMainDataDir[PATH_MAX];          // data file (htdata832.pdb, htsfx.pdb) dir
char gszTempDir[PATH_MAX];              // temp file directory
char gszMissionPacksDir[PATH_MAX];      // downloaded mission packs
char gszMissionPackInfosDir[PATH_MAX];
char gszSaveGamesDir[PATH_MAX];         // saved games
char gszPrefsFilename[PATH_MAX];        // game prefs
char gszCompletesDir[PATH_MAX];         // for "mission completed" tracking
HttpService *gphttp;
Mac *mac;

bool HostHelpers::Init() {
    // Get directories off .app
    NSBundle *bundle = [NSBundle mainBundle];
    NSString *appDir = [bundle bundlePath];
    const char *pszAppDir = [appDir cStringUsingEncoding:
            [NSString defaultCStringEncoding]];
    sprintf(gszMainDataDir, "%s/Contents/Resources", pszAppDir);

    // This returns the user's home directory; Library, and Documents are in it
    NSString *applicationSupportDir = [NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) firstObject];
    const char *pszApplicationSupportDir = [applicationSupportDir cStringUsingEncoding:
            [NSString defaultCStringEncoding]];

    sprintf(gszTempDir, "%s/HostileTakeover/tmp", pszApplicationSupportDir);
    sprintf(gszMissionPacksDir, "%s/HostileTakeover/MissionPacks", pszApplicationSupportDir);
    sprintf(gszMissionPackInfosDir, "%s/HostileTakeover/MissionPackInfos", pszApplicationSupportDir);
    sprintf(gszSaveGamesDir, "%s/HostileTakeover/SaveGames", pszApplicationSupportDir);
    sprintf(gszCompletesDir, "%s/HostileTakeover/Completes", pszApplicationSupportDir);
    sprintf(gszPrefsFilename, "%s/HostileTakeover/prefs.json", pszApplicationSupportDir);

    [[NSFileManager defaultManager] createDirectoryAtPath:[NSString stringWithFormat:@"%@/HostileTakeover", applicationSupportDir] withIntermediateDirectories:YES attributes:nil error:nil];

    // Make the directories under Library
    mkdir(gszTempDir, 0755);
    mkdir(gszMissionPacksDir, 0755);
    mkdir(gszMissionPackInfosDir, 0755);
    mkdir(gszSaveGamesDir, 0755);
    mkdir(gszCompletesDir, 0755);

    // Mac specific http service
    extern HttpService *gphttp;
    gphttp = (HttpService *)new MacHttpService;
    
    mac = [[Mac alloc] init];

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
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:strURL]];
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
    
    cxScreen = 800;
    cyScreen = 600;
    
    pprops->cxWidth = cxScreen;
    pprops->cyHeight = cyScreen;
    pprops->cbxPitch = 1;
    pprops->cbyPitch = pprops->cxWidth;
    pprops->ffFormat = wi::kfDirect8;
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
    [mac initiateAsk:[NSString stringWithUTF8String:title] maxCharacters:max defaultString:[NSString stringWithUTF8String:def] keyboard:keyboard secure: secure ? YES : NO];
    base::Thread::current().Post(wi::kidmAskStringEvent, NULL);
}

void HostHelpers::GetAskString(char *psz, int cb)
{
    const char *cString = [[mac askString] cStringUsingEncoding:NSASCIIStringEncoding];
    wi::strncpyz(psz, cString, cb);
}

IChatController *HostHelpers::GetChatController()
{
    return [mac getChatController];
}

void HostHelpers::InitiateWebView(const char *title, const char *url) {
    HostHelpers::OpenUrl(url);
}

const char *HostHelpers::GetPlatformString() {
    NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
    NSString *versionString = [NSString stringWithFormat:@"MacOS %ld.%ld.%ld",
        version.majorVersion, version.minorVersion, version.patchVersion];

    return [versionString cStringUsingEncoding:NSASCIIStringEncoding];
}

void HostHelpers::GameThreadStart(void *pv) {
    Log("Starting game...");
    wi::GameMain((char *)""); 
}

int HostHelpers::main(int argc, char **argv)
{
    // Create the application
    NSArray *tl;
    NSApplication *application = [NSApplication sharedApplication];
    [[NSBundle mainBundle] loadNibNamed:@"MainMenu" owner:application topLevelObjects:&tl];

    // Setup application & delegate
    AppDelegate *applicationDelegate = [[AppDelegate alloc] init];
    [application setDelegate:applicationDelegate];
    [application run];
    
    return 0;
}
    
} // namespace wi
