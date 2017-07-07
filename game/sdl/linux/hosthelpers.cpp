#include "game/sdl/linux/linuxchatcontroller.h"
#include "game/sdl/hosthelpers.h"
#include "game/sdl/sdlhttpservice.h"
#include "base/thread.h"
#include <sys/utsname.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <net/if.h>
#include <limits.h>
#include <stdio.h>
#include <pwd.h>

namespace wi {

char gszLinuxGamesDir[PATH_MAX];        // /home/[user]/Games
char gszHomeDir[PATH_MAX];              // /home/[user]/Games/WarfareIncorporated

char gszMainDataDir[PATH_MAX];          // data files directory
char gszTempDir[PATH_MAX];              // temp file directory
char gszMissionPacksDir[PATH_MAX];      // downloaded mission packs
char gszMissionPackInfosDir[PATH_MAX];
char gszSaveGamesDir[PATH_MAX];         // saved games
char gszPrefsFilename[PATH_MAX];        // game prefs
char gszCompletesDir[PATH_MAX];         // for "mission completed" tracking
HttpService *gphttp;
IChatController *gpchat;

bool HostHelpers::Init() {
    // Get the user's home directory
    struct passwd *pw = getpwuid(getuid());
    const char *linuxHomeDir = pw->pw_dir;

    // Add a Games folder to it
    sprintf(gszLinuxGamesDir, "%s/Games", linuxHomeDir);

    // By HomeDir we are refering to the game's home dir and not the user's home dir
    sprintf(gszHomeDir, "%s/HostileTakeover", gszLinuxGamesDir);

    // MainDataDir is where htdata.pdb and htsfx.pdb are stored
    // Linux players should run install.sh to copy the database files this location
    sprintf(gszMainDataDir, "%s", gszHomeDir);

    sprintf(gszTempDir, "%s/tmp", gszHomeDir);
    sprintf(gszMissionPacksDir, "%s/MissionPacks", gszHomeDir);
    sprintf(gszMissionPackInfosDir, "%s/MissionPackInfos", gszHomeDir);
    sprintf(gszSaveGamesDir, "%s/SaveGames", gszHomeDir);
    sprintf(gszCompletesDir, "%s/Completes/", gszHomeDir);
    sprintf(gszPrefsFilename, "%s/prefs.json", gszHomeDir);

    // Make the directories
    mkdir(gszLinuxGamesDir, 0755);
    mkdir(gszHomeDir, 0755);
    mkdir(gszMissionPacksDir, 0755);
    mkdir(gszMissionPackInfosDir, 0755);
    mkdir(gszSaveGamesDir, 0755);
    mkdir(gszCompletesDir, 0755);
    mkdir(gszTempDir, 0755);

    extern HttpService *gphttp;
    gphttp = (HttpService *)new SdlHttpService();

    gpchat = NULL;

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
    Log("HostHelpers::OpenUrl not implemented yet");
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

bool HostHelpers::IsExiting()
{
#if 0
    return [g_appDelegate isExiting];
#else
    Log("HostHelpers::IsExitig not implemented yet");
    return false;
#endif
}

const char *HostHelpers::GetUdid()
{
// Use the Mac Address for the DID (doesn't work on Mac)
#ifndef DARWIN

    #define MAC_STRING_LENGTH 13
    char *ret = (char *)malloc(MAC_STRING_LENGTH);
    struct ifreq s;
    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

    strcpy(s.ifr_name, "eth0");
    if (fd >= 0 && ret && 0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
        int i;
        for (i = 0; i < 6; ++i)
        snprintf(ret+i*2,MAC_STRING_LENGTH-i*2,"%02x",(unsigned char) s.ifr_addr.sa_data[i]);
    } else {
        LOG() << "malloc/socket/ioctl failed";
    }

    return (const char *)ret;

#else
    static char gszUdid[20];

    char *pch = gszUdid;
	if (*pch == 0) {
		for (int i = 0; i < 19; i++)
			*pch++ = '0' + GetAsyncRandom() % 10;
		*pch = 0;
	}
    return gszUdid;
#endif
}

void HostHelpers::InitiateAsk(const char *title, int max, const char *def,
        int keyboard, bool secure)
{
    Log("HostHelpers::InitiateAsk not implemented yet");
}

void HostHelpers::GetAskString(char *psz, int cb)
{
    Log("HostHelpers::GetAskString not implemented yet");
}

IChatController *HostHelpers::GetChatController()
{
    if (gpchat == NULL) {
        gpchat = new wi::LinuxChatController();
    }
    return gpchat;
}

void HostHelpers::InitiateWebView(const char *title, const char *url) {
    Log("HostHelpers::InitiateWebView not implemented yet");
}

const char *HostHelpers::GetPlatformString() {
    static struct utsname systemInfo;
    uname(&systemInfo);
    return systemInfo.sysname;
}

} // namespace wi
