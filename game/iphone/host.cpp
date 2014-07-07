#include "game/ht.h"
#include "game/iphone/iphone.h"
#include "game/iphone/input.h"
#include "base/tick.h"
#include "base/log.h"
#include "base/thread.h"
#include "base/md5.h"
#include "game/httppackmanager.h"
#include "game/httppackinfomanager.h"
#include "game/iphone/iphonehttpservice.h"
#include "game/completemanager.h"
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

namespace wi {

IPhonePackFileReader gpakr;
HttpPackManager *gppackm;
HttpPackInfoManager *gppim;
HttpService *gphttp;
CompleteManager *gpcptm;

bool HostInit()
{
    gphttp = (HttpService *)new IPhoneHttpService;
    gppackm = new HttpPackManager(gphttp, IPhone::GetMissionPacksDir(),
            IPhone::GetTempDir());
    gppackm->InitFromInstalled();
    gppim = new HttpPackInfoManager(gphttp, IPhone::GetMissionPackInfosDir(),
            IPhone::GetTempDir());
    gpcptm = new CompleteManager(IPhone::GetCompletesDir());
    gpcptm->Init();

    return true;
}

void HostExit()
{
    delete gppim;
    delete gppackm;
    delete (IPhoneHttpService *)gphttp;
}

const char *HostGenerateDeviceId() {
    // Hash it so query params aren't obnoxious
    MD5_CTX md5;
    MD5Init(&md5);
    const char *pszUUID = IPhone::GetStaticUUID();
    MD5Update(&md5, (const byte *)pszUUID, strlen(pszUUID));
    byte hash[16];
    MD5Final(hash, &md5);
    return base::Format::ToHex(hash, 16);
}

void HostInitiateWebView(const char *title, const char *url) {
    return IPhone::InitiateWebView(title, url);
}

IChatController *HostGetChatController() {
    return IPhone::GetChatController();
}

void HostInitiateAsk(const char *title, int max, const char *def,
        int keyboard, bool secure) {
    return IPhone::InitiateAsk(title, max, def, keyboard, secure);
}

void HostGetAskString(char *psz, int cb) {
    return IPhone::GetAskString(psz, cb);
}

void HostOpenUrl(const char *pszUrl) {
    IPhone::OpenUrl(pszUrl);
}

void HostSuspendModalLoop(DibBitmap *pbm)
{
    // Wait for WI to become active again

    LOG() << "Entering SuspendModalLoop";

    base::Thread& thread = base::Thread::current();
    while (true) {
        base::Message msg;
        thread.Get(&msg);
        if (msg.id == kidmAppSetFocus) {
            break;
        }
        if (msg.id == kidmAppTerminate) {
            thread.Post(&msg);
            break;
        }
        thread.Dispatch(&msg);
    }

    LOG() << "Leaving SuspendModalLoop";
}

bool ProcessMessage(base::Message *pmsg, Event *pevt)
{
    switch (pmsg->id) {
    case kidmMouseDown:
        pevt->eType = penDownEvent;
        pevt->ff = kfEvtFinger;
        break;

    case kidmMouseDown2:
        pevt->eType = penDownEvent2;
        pevt->ff = kfEvtFinger;
        break;

    case kidmMouseUp:
        pevt->eType = penUpEvent;
        pevt->ff = kfEvtFinger;
        break;

    case kidmMouseUp2:
        pevt->eType = penUpEvent2;
        pevt->ff = kfEvtFinger;
        break;

    case kidmMouseMove:
        pevt->eType = penMoveEvent;
        pevt->ff = kfEvtFinger;
        break;

    case kidmMouseMove2:
        pevt->eType = penMoveEvent2;
        pevt->ff = kfEvtFinger;
        break;            
            
    case kidmAppTerminate:
        pevt->eType = appStopEvent;
        pevt->ff = 0;
        break;

    case kidmAppKillFocus:
        pevt->eType = gameSuspendEvent;
        pevt->ff = 0;
        break;

    case base::kidmNullEvent:
        pevt->eType = nullEvent;
        pevt->ff = 0;
        break;

    case base::kidmTransportEvent:
        pevt->eType = transportEvent;
        pevt->ff = 0;
        break;

    case kidmAskStringEvent:
        pevt->eType = askStringEvent;
        pevt->ff = 0;
        break;

    default:
        return false;
    }

    pevt->dw = 0;
    pevt->x = pmsg->x;
    pevt->y = pmsg->y;
    pevt->ms = pmsg->ms;
    if (pmsg->ff & kfMsgCoalesce) {
        pevt->ff |= kfEvtCoalesce;
    }
    if (pmsg->ff & kfMsgCancelMode) {
        pevt->dw = 1;
    }
    
    SurfaceProperties props;
    IPhone::GetSurfaceProperties(&props);
    int cx = props.cxWidth;
    int cy = props.cyHeight;

    if (gpdisp != NULL) {
        ModeInfo mode;
        gpdisp->GetMode(&mode);
        switch (mode.nDegreeOrientation) {
        case 0:
            // Screen rotated 90 degrees but coordinates unrotated
            pevt->x = pmsg->y;
            pevt->y = (cy - 1) - pmsg->x;
            break;

        case 90:
            pevt->x = (cy - 1) - pmsg->y;
            pevt->y = pmsg->x;
            break;

        case 180:
            pevt->x = (cx - 1) - pmsg->x;
            pevt->y = (cy - 1) - pmsg->y;
            break;

        case 270:
            pevt->x = pmsg->y;
            pevt->y = (cx - 1) - pmsg->x;
            break;
        }
    }

    return true;
}

bool HostGetEvent(Event *pevt, long ctWait)
{
    base::Thread& thread = base::Thread::current();
    while (true) {
        base::Message msg;
        if (!thread.Get(&msg, ctWait)) {
            return false;
        }

        if (msg.handler != NULL) {
            thread.Dispatch(&msg);
            continue;
        }

        if (msg.id == kidmBreakEvent) {
            return false;
        }

        if (ProcessMessage(&msg, pevt)) {
            return true;
        }
    }
}

void HostOutputDebugString(char *pszFormat, ...)
{
#ifdef DEBUG
    va_list va;
    va_start(va, pszFormat);
    IPhone::Log(pszFormat, va);
    va_end(va);
#endif
}

long HostGetTickCount()
{
    return base::GetTickCount();
}

long HostGetMillisecondCount()
{
    return base::GetMillisecondCount();
}
    
long HostRunSpeedTests(DibBitmap *pbmSrc)
{
	return 0;
}

dword HostGetCurrentKeyState(dword keyBit)
{
    return 0;
}

bool HostIsPenDown()
{
    return false;
}

void HostMessageBox(TCHAR *pszFormat, ...)
{
    va_list va;
    va_start(va, pszFormat);
    IPhone::MessageBox(pszFormat, va);
    va_end(va);
}

void HostGetUserName(char *pszBuff, int cb)
{
    strncpyz(pszBuff, "anonymous", cb);
}

bool HostGetOwnerName(char *pszBuff, int cb, bool fShowError)
{
    strncpyz(pszBuff, "Player", cb);
    return true;
}

// UNDONE: prefix directory?

FileHandle HostOpenFile(const char *pszFilename, word wf)
{
    char *pszMode;
    if (wf == kfOfRead)
        pszMode = "rb";
    else if (wf == kfOfWrite)
        pszMode = "wb";
    else if (wf == (kfOfRead | kfOfWrite))
        pszMode = "rb+";

    return (FileHandle)fopen((char *)pszFilename, pszMode);
}

void HostCloseFile(FileHandle hf)
{
    fclose((FILE *)hf);
}

dword HostWriteFile(FileHandle hf, void *pv, dword cb)
{
    return fwrite(pv, 1, cb, (FILE *)hf);
}

dword HostReadFile(FileHandle hf, void *pv, dword cb)
{
    return fread(pv, 1, cb, (FILE *)hf);
}

void HostSleep(dword ct)
{
    int cms = ct * 10;
    struct timeval tvWait;
    tvWait.tv_sec = cms / 1000;
    tvWait.tv_usec = (cms % 1000) * 1000;
    select(0, NULL, NULL, NULL, &tvWait);
}

void HostGetSilkRect(int irc, Rect *prc)
{
    return;
}

// Figure out what kind of sound device exists, and return a SoundDevice for it

SoundDevice *HostOpenSoundDevice()
{
    return CreateIPhoneSoundDevice();
}

SoundDevice::~SoundDevice()
{
}

// Used for sound buffer maintenance requirements

SoundDevice *gpsnddService;
void SetSoundServiceDevice(SoundDevice *psndd)
{
    gpsnddService = psndd;
}

bool HostSoundServiceProc()
{
    if (gpsnddService == NULL)
        return false;
    gpsnddService->ServiceProc();
    return true;
}

void HostGetCurrentDate(Date *pdate)
{
    time_t result = time(NULL);
    struct tm *ptm = localtime(&result);
    pdate->nYear = ptm->tm_year + 1900;
    pdate->nMonth = ptm->tm_mon + 1;
    pdate->nDay = ptm->tm_mday;
}

bool HostSavePreferences(void *pv, int cb)
{
    LOG() << IPhone::GetPrefsFilename();

    FILE *pf = fopen(IPhone::GetPrefsFilename(), "wb");
    if (pf == NULL) {
        LOG() << "error opening preferences! " << errno;
        return false;
    }
    if (fwrite(pv, cb, 1, pf) != 1) {
        LOG() << "error writing preferences! " << errno;
        fclose(pf);
        return false;
    }
    fclose(pf);
    return true;
}

int HostLoadPreferences(void *pv, int cb)
{
    FILE *pf = fopen(IPhone::GetPrefsFilename(), "rb");
    if (pf == NULL) {
        return -1;
    }

    // Read prefs

    int cbRead = (int)fread(pv, 1, cb, pf);
    fclose(pf);
    return cbRead;
}

const char *HostGetMainDataDir()
{
    return IPhone::GetMainDataDir();
}

void HostNotEnoughMemory(bool fStorage, dword cbFree, dword cbNeed)
{
	HostMessageBox(TEXT("Need %ld bytes of memory but only %ld bytes are free!"), cbNeed, cbFree);
}

bool HostEnumAddonFiles(Enum *penm, char *pszAddonDir, int cbDir,
        char *pszAddon, int cb)
{
    // PackManager is the way to do this now
    return false;
}

} // namespace wi
