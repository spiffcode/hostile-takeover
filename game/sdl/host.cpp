#include "game/ht.h"
#include "game/sdl/hosthelpers.h"
#include "base/tick.h"
#include "base/log.h"
#include "base/thread.h"
#include "base/md5.h"
#include "game/httppackmanager.h"
#include "game/httppackinfomanager.h"
#include "game/httpservice.h"
#include "game/completemanager.h"
#include <sys/types.h>
#include <errno.h>
#include <SDL.h>

namespace wi {

SdlPackFileReader gpakr;
HttpPackManager *gppackm;
HttpPackInfoManager *gppim;
CompleteManager *gpcptm;

char *gpszUdid;

bool HostInit()
{
    HostHelpers::Init();

    //
    gppackm = new HttpPackManager(gphttp, HostHelpers::GetMissionPacksDir(), HostHelpers::GetTempDir());
    gppackm->InitFromInstalled();
    gppim = new HttpPackInfoManager(gphttp, HostHelpers::GetMissionPackInfosDir(), HostHelpers::GetTempDir());
    gpcptm = new CompleteManager(HostHelpers::GetCompletesDir());
    gpcptm->Init();

	// TODO(darrinm): get user id (gpszUdid)

    return true;
}

void HostExit()
{
    delete gppim;
    delete gppackm;

    HostHelpers::Cleanup();
}

const char *HostGetDeviceId() {
    // Hash it so query params aren't obnoxious                                 
    MD5_CTX md5;
    MD5Init(&md5);
    const char *udid = HostHelpers::GetUdid();
    MD5Update(&md5, (const byte *)udid, strlen(udid));
    byte hash[16];
    MD5Final(hash, &md5);
    return base::Format::ToHex(hash, 16);
}

void HostInitiateWebView(const char *title, const char *url) {
    return HostHelpers::InitiateWebView(title, url);
}

IChatController *HostGetChatController() {
    return HostHelpers::GetChatController();
}

void HostInitiateAsk(const char *title, int max, const char *def,
        int keyboard, bool secure) {
    return HostHelpers::InitiateAsk(title, max, def, keyboard, secure);
}

void HostGetAskString(char *psz, int cb) {
    return HostHelpers::GetAskString(psz, cb);
}

void HostOpenUrl(const char *pszUrl) {
    HostHelpers::OpenUrl(pszUrl);
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

bool ProcessSdlEvent(Event *pevt)
{
    memset(pevt, 0, sizeof(*pevt));
    SDL_Event event;
    switch (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT,
            SDL_LASTEVENT)) {
    case -1:
        // App is exiting, or there is an error
        pevt->eType = appStopEvent;
        break;

    case 0:
        return false;
    }

	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
        pevt->eType = penDownEvent;
        pevt->x = event.button.x;
        pevt->y = event.button.y;
		break;
		
	case SDL_MOUSEBUTTONUP:
        pevt->eType = penUpEvent;
		pevt->x = event.button.x;
		pevt->y = event.button.y;
		break;
		
	case SDL_MOUSEMOTION:
        pevt->eType = penMoveEvent;
		pevt->x = event.motion.x;
		pevt->y = event.motion.y;
		break;
		
	case SDL_KEYDOWN:
        pevt->eType = keyDownEvent;
        switch (event.key.keysym.sym) {
		case SDLK_UP:
			pevt->chr = chrPageUp;
			break;
			
		case SDLK_DOWN:
			pevt->chr = chrPageDown;
			break;
			
		case SDLK_LEFT:
			pevt->chr = chrLeft;
			break;
			
		case SDLK_RIGHT:
			pevt->chr = chrRight;
			break;
			
		case SDLK_BACKSPACE:
			pevt->chr = chrBackspace;
			break;
			
		case SDLK_DELETE:
			pevt->chr = chrDelete;
			break;

#if 0			
		case SDLK_F7:
			if (gpavir == NULL) {
				gpavir = new AviRecorder();
				Size siz;
				gpdisp->GetFrontDib()->GetSize(&siz);
				if (!gpavir->Start(siz.cx, siz.cy)) {
					delete gpavir;
					gpavir = NULL;
				}
			}
			break;

		case SDLK_F8:
			if (gpavir != NULL) {
				gpavir->Stop();
				delete gpavir;
				gpavir = NULL;
			}
			break;
#endif

		case SDLK_KP_MINUS:	// numpad
			{
				int i = 0;
				for (; i < ARRAYSIZE(gatGameSpeeds); i++)
					if (gatGameSpeeds[i] == gtGameSpeed)
						break;
				i--;
				if (i < 0)
					i = 0;
				ggame.SetGameSpeed(gatGameSpeeds[i]);
			}
			break;

		case SDLK_KP_PLUS:	// numpad
			{
				int i = 0;
				for (; i < ARRAYSIZE(gatGameSpeeds); i++)
					if (gatGameSpeeds[i] == gtGameSpeed)
						break;
				i++;
				if (i >= ARRAYSIZE(gatGameSpeeds))
					i = ARRAYSIZE(gatGameSpeeds) - 1;
				ggame.SetGameSpeed(gatGameSpeeds[i]);
			}
			break;

        default:
#ifdef DEBUG_HELPERS
			extern void DebugHelperKeyHandler(word vk);
			DebugHelperKeyHandler(pmsg->wParam);
#endif
            pevt->chr = event.key.keysym.sym;
            break;
        }
        break;

	case SDL_QUIT:
        pevt->eType = appStopEvent;
        break;

    default:
        return false;
    }    

#if 0 // TODO(scottlu)
    - Add SDL processing for touch events.
    - Generate penMoveEvent2, penDownEvent2, penUpEvent2
    - add pevt->ff |= kfEvtFinger
    - add SDL support for device turned off, or locked and return
      gameSuspendEvent. If no SDL support for this, post kidmAppKillFocus
      in native layer and translate into gameSuspendEvent in ProcessEvents.
      old code:

    case kidmAppKillFocus:
        pevt->eType = gameSuspendEvent;
        pevt->ff = 0;
        break;
#endif

#if 0 // TODO(darrinm)
    if (gpdisp != NULL) {
        int x = pevt->x;
        int y = pevt->y;

        SurfaceProperties props;
        HostHelpers::GetSurfaceProperties(&props);
        int cx = props.cxWidth;
        int cy = props.cyHeight;

        ModeInfo mode;
        gpdisp->GetMode(&mode);
        switch (mode.nDegreeOrientation) {
        case 0:
            // Screen rotated 90 degrees but coordinates unrotated
            pevt->x = y;
            pevt->y = (cy - 1) - x;
            break;

        case 90:
            pevt->x = (cy - 1) - y;
            pevt->y = x;
            break;

        case 180:
            pevt->x = (cx - 1) - x;
            pevt->y = (cy - 1) - y;
            break;

        case 270:
            pevt->x = y;
            pevt->y = (cx - 1) - x;
            break;
        }
    }
#endif

    return true;
}

bool ProcessMessage(base::Message *pmsg, Event *pevt)
{
    memset(pevt, sizeof(*pevt), 0);
    switch (pmsg->id) {
    case kidmNullEvent:
        pevt->eType = nullEvent;
        break;

    case kidmTransportEvent:
        pevt->eType = transportEvent;
        break;

    case kidmAskStringEvent:
        pevt->eType = askStringEvent;
        break;
		
    default:
        return false;
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

        if (msg.id == kidmSdlEvent) {
            if (ProcessSdlEvent(pevt)) {
                return true;
            }
            continue;
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
    HostHelpers::Log(pszFormat, va);
    va_end(va);
#endif
}

long HostGetTickCount()
{
    return (long)base::GetTickCount();
}

long HostGetMillisecondCount()
{
    return (long)base::GetMillisecondCount();
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
    HostHelpers::MessageBox(pszFormat, va);
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
    const char *pszMode;
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
    return CreateSdlSoundDevice();
}

SoundDevice::~SoundDevice()
{
}

// Used for sound buffer maintenance requirements

bool HostSoundServiceProc()
{
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
    LOG() << HostHelpers::GetPrefsFilename();

    FILE *pf = fopen(HostHelpers::GetPrefsFilename(), "wb");
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
    FILE *pf = fopen(HostHelpers::GetPrefsFilename(), "rb");
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
    return HostHelpers::GetMainDataDir();
}

const char *HostGetSaveGamesDir()
{
    return HostHelpers::GetSaveGamesDir();
}

void HostNotEnoughMemory(bool fStorage, dword cbFree, dword cbNeed)
{
	HostMessageBox(TEXT((char *)"Need %ld bytes of memory but only %ld bytes are free!"), cbNeed, cbFree);
}

bool HostEnumAddonFiles(Enum *penm, char *pszAddonDir, int cbDir,
        char *pszAddon, int cb)
{
    // PackManager is the way to do this now
    return false;
}

} // namespace wi
