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
base::Thread *gpgt;

SurfaceProperties gprops;
SDL_FingerID gtouches[2];
wi::Point gaptLast[2];
bool gfWasBackgrounded;
bool gfSimWasPaused;

char *gpszUdid;

bool HostInit()
{
    HostHelpers::Init();
    HostHelpers::GetSurfaceProperties(&gprops);

    gppackm = new HttpPackManager(gphttp, HostHelpers::GetMissionPacksDir(), HostHelpers::GetTempDir());
    gppackm->InitFromInstalled();
    gppim = new HttpPackInfoManager(gphttp, HostHelpers::GetMissionPackInfosDir(), HostHelpers::GetTempDir());
    gpcptm = new CompleteManager(HostHelpers::GetCompletesDir());
    gpcptm->Init();

    // These initilize to 0 but are expected to be -1
    // since 0 is a valid touch id on some devices
    gtouches[0] = -1;
    gtouches[1] = -1;

	// TODO(darrinm): get user id (gpszUdid)

    return true;
}

void HostExit()
{
    delete gppim;
    delete gppackm;

    HostHelpers::Cleanup();
}

const char *HostGenerateDeviceId() {
    // Hash it so query params aren't obnoxious                                 
    MD5_CTX md5;
    MD5Init(&md5);
    const char *udid = HostHelpers::GetUdid();
    MD5Update(&md5, (const byte *)udid, strlen(udid));
    byte hash[16];
    MD5Final(hash, &md5);
    return base::Format::ToHex(hash, 16);
}

const char *HostGetPlatformString() {
    return HostHelpers::GetPlatformString();
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

//
// Post motion events into the message queue in order
// to handle event coalescing for kidmMoveEvent / kidmMoveEvent2.
//
void PostSdlMotionEvent(const SDL_Event &event)
{
    // Post these into the message queue with coalescing
    // The second id is a special exception. Coalesce sequential
    // events, but allow this exception. This allows coalescing
    // to occur across an intermixed stream of finger 1 and finger 2
    // move events.
    for (int i = 0; i < 2; i++) {
        if (gtouches[i] == event.tfinger.fingerId) {
            base::Message msg;
            msg.id = (i == 0) ? kidmFingerMoveEvent : kidmFingerMoveEvent2;
            msg.x = (int)(event.tfinger.x * gprops.cxWidth);
            msg.y = (int)(event.tfinger.y * gprops.cyHeight);
            msg.ff = 0;
            msg.ms = HostGetMillisecondCount();

            gaptLast[i].x = (int)msg.x;
            gaptLast[i].y = (int)msg.y;

            HostGetGameThread().Post(&msg, i == 0 ? kidmFingerMoveEvent2 : kidmFingerMoveEvent);
            break;
        }
    }
}

void CheckTouchTracking() {
    // This is just in case - if we lose an up, this detects it and resets
    // things.

    int cTouchesTracking = 0;
    for (int i = 0; i < 2; i++) {
        if (gtouches[i] != -1) {
            cTouchesTracking++;
        }
    }
    int cTouchesFound = 0;

    // Check each touch device

    int ntd = SDL_GetNumTouchDevices();
    for (int i = 0; i < ntd; i++) {
        SDL_TouchID device = SDL_GetTouchDevice(i);

        for (int n = 0; n < SDL_GetNumTouchFingers(device); n++) {
            SDL_FingerID fingerId = SDL_GetTouchFinger(device, n)->id;
            if (fingerId == gtouches[0] || fingerId == gtouches[1]) {
                cTouchesFound++;
            }
        }
    }

    // If it's whacked, post an up and set the slot to -1.

    if (cTouchesFound < cTouchesTracking) {
        for (int i = 0; i < 2; i++) {
            if (gtouches[i] != -1) {
                gtouches[i] = -1;
                base::Message msg;
                msg.id = (i == 0) ? kidmFingerUpEvent : kidmFingerUpEvent2;
                msg.x = gaptLast[i].x;
                msg.y = gaptLast[i].y;
                msg.ff = 0;
                msg.ms = wi::HostGetMillisecondCount();
                HostGetGameThread().Post(&msg);
            }
        }
    }
}

bool ProcessSdlEvent(base::Message *pmsg, Event *pevt)
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

#if defined(__IPHONEOS__) || defined(__ANDROID__)
    case SDL_FINGERDOWN: {
        for (int i = 0; i < 2; i++) {
            if (gtouches[i] == -1) {
                gtouches[i] = event.tfinger.fingerId;

                pevt->eType = (i == 0) ? penDownEvent : penDownEvent2;
                pevt->x = (int)(event.tfinger.x * gprops.cxWidth);
                pevt->y = (int)(event.tfinger.y * gprops.cyHeight);
                pevt->ff = kfEvtFinger;
                pevt->ms = HostGetMillisecondCount();

                gaptLast[i].x = (int)pevt->x;
                gaptLast[i].y = (int)pevt->y;
                break;
            }
        }

        // Sometimes we won't get an up. Handle this case here.
        CheckTouchTracking();

        break;
        }
        
    case SDL_FINGERUP: {
        for (int i = 0; i < 2; i++) {
            if (gtouches[i] == event.tfinger.fingerId) {
                gtouches[i] = -1;

                pevt->eType = (i == 0) ? penUpEvent : penUpEvent2;
                pevt->x = (int)(event.tfinger.x * gprops.cxWidth);
                pevt->y = (int)(event.tfinger.y * gprops.cyHeight);
                pevt->ff = kfEvtFinger;
                pevt->ms = HostGetMillisecondCount();

                gaptLast[i].x = (int)pevt->x;
                gaptLast[i].y = (int)pevt->y;

                break;
            }
        }

        break;
        }
        
    case SDL_FINGERMOTION: {
        // Eat the event if it's not one of the two fingers
        if (event.tfinger.fingerId != gtouches[0] && event.tfinger.fingerId != gtouches[1]) {
            break;
        }

        // Coalesce all sequential finger motion events.
        PostSdlMotionEvent(event);
        while (true) {
            // Is the next event a motion event?
            SDL_Event eventT;
            int n = SDL_PeepEvents(&eventT, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
            if (event.type != SDL_FINGERMOTION)
                break;
            
            // Eat the motion event
            n = SDL_PeepEvents(&eventT, 1, SDL_GETEVENT, SDL_FINGERMOTION, SDL_FINGERMOTION);
            if (n != 1) {
                break;
            }
            PostSdlMotionEvent(eventT);
        }

        }
        break;
#endif

#if defined(__MACOSX__) || defined(__LINUX__)
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
#endif
		
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

    case SDL_KEYUP:
        // pevt->eType = keyUpEvent;
        switch (event.key.keysym.sym) {

        case SDLK_AC_BACK:
            pevt->eType = keyDownEvent;
            pevt->chr = vchrBack;
            break;

        default:
            pevt->chr = event.key.keysym.sym;
            break;
        }
        break;

    case SDL_APP_DIDENTERFOREGROUND:
        // Allow the display to render
        gpdisp->SetShouldRender(true);

        // Unpause simulation
        if (!gfSimWasPaused) {
            gsim.Pause(false);
            gfSimWasPaused = false;
        }

        // The client was disconected in SDL_APP_DIDENTERBACKGROUND.
        // Notify the callbacks about this to present the user with a message.
        if (gfWasBackgrounded && gptra != NULL) {
            if (gptra->GetGameCallback() != NULL) {
                gptra->GetGameCallback()->OnGameDisconnect();
            }

            if (gptra->GetCallback() != NULL) {
                gptra->GetCallback()->OnConnectionClose();
            }

            // This should already be closed from SDL_APP_DIDENTERBACKGROUND
            gptra->Close();
        }

        // SDL may have released its graphics context if the app was previously
        // backgrounded. This leaves the screen black when the user returns.
        // Hack: Redraw
        gpmfrmm->DrawFrame(true);
        break;

    case SDL_APP_DIDENTERBACKGROUND:
        gfWasBackgrounded = true;

        // Pause simulation
        gfSimWasPaused = gsim.IsPaused();
        gsim.Pause(true);

        // Close the connection to the server. If the user returns to the app,
        // SDL_APP_DIDENTERFOREGROUND will notify gptra's callbacks.
        if (gptra != NULL) {
            gptra->Close();
        }

        // Stop display rendering; SDL may release its graphics context when
        // backgrounded, so we don't want to try to render to a non-existant context.
        gpdisp->SetShouldRender(false);
        break;

    case SDL_APP_WILLENTERFOREGROUND:
        break;

    case SDL_APP_WILLENTERBACKGROUND:
        break;

    case SDL_APP_TERMINATING:
        // NOTE: The app seems to terminate before the code in this
        // case can be executed (iOS and Android).
        break;

	case SDL_QUIT:
        pevt->eType = appStopEvent;
        break;

    case SDL_WINDOWEVENT:
        gpmfrmm->DrawFrame(true);
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

    pevt->ms = pmsg->ms;
    if (pevt->ms == 0)
        pevt->ms = HostGetMillisecondCount();

    return true;
}

bool ProcessMessage(base::Message *pmsg, Event *pevt)
{
    memset(pevt, sizeof(*pevt), 0);
    switch (pmsg->id) {
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

    case kidmFingerMoveEvent:
        pevt->eType = penMoveEvent;
        pevt->ff = kfEvtFinger;
        break;

    case kidmFingerMoveEvent2:
        pevt->eType = penMoveEvent2;
        pevt->ff = kfEvtFinger;
        break;

    case kidmFingerUpEvent:
        pevt->eType = penUpEvent;
        pevt->ff = kfEvtFinger;
        break;

    case kidmFingerUpEvent2:
        pevt->eType = penUpEvent2;
        pevt->ff = kfEvtFinger;
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
            if (ProcessSdlEvent(&msg, pevt)) {
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
    struct KeyBitVKey {
        dword keyBit;
        short vk;
    } s_akk[] = {
        {keyBitPageUp, SDL_SCANCODE_UP},
        {keyBitPageDown, SDL_SCANCODE_DOWN},
        {keyBitDpadLeft, SDL_SCANCODE_LEFT},
        {keyBitDpadRight, SDL_SCANCODE_RIGHT},
        {keyBitDpadButton, SDL_SCANCODE_RETURN},
    };
	Assert(keyBit != 0);

	dword keyBitRet = 0;
    const Uint8 *state = SDL_GetKeyboardState(NULL);
	for (int i = 0; i < sizeof(s_akk) / sizeof(KeyBitVKey); i++)
		if ((keyBit & s_akk[i].keyBit) != 0)
			if (state[s_akk[i].vk])
				keyBitRet |= s_akk[i].keyBit;

    return keyBitRet;
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

    return (FileHandle)SDL_RWFromFile(pszFilename, pszMode);
}

void HostCloseFile(FileHandle hf)
{
    SDL_RWclose((SDL_RWops *)hf);
}

dword HostWriteFile(void *pv, dword c, dword cb, FileHandle hf)
{
    // SDL_RWwrite() returns the number of objects written,
    // which will be less than cb on error

    return (dword)SDL_RWwrite((SDL_RWops *)hf, pv, c, cb);
}

dword HostReadFile(void *pv, dword c, dword cb, FileHandle hf)
{
    // SDLRWread() returns the number of objects read,
    // or 0 at error or end of file

    return (dword)SDL_RWread((SDL_RWops *)hf, pv, c, cb);
}

dword HostSeekFile(FileHandle hf, int off, int nOrigin)
{
    // SDL_RWseek() returns the final offset in the data
    // stream after the seek or -1 on error

    return (dword)SDL_RWseek((SDL_RWops *)hf, off, nOrigin);
}

dword HostTellFile(FileHandle hf)
{
    // SDL_RWtell() returns the current offset in the stream,
    // or -1 if the information can not be determined

    return (dword)SDL_RWtell((SDL_RWops *)hf);
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

const char *HostGetMainDataDir()
{
    return HostHelpers::GetMainDataDir();
}

const char *HostGetSaveGamesDir()
{
    return HostHelpers::GetSaveGamesDir();
}

const char *HostGetPrefsFilename()
{
    return HostHelpers::GetPrefsFilename();
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

void HostSetGameThread(base::Thread *thread) {
    gpgt = thread;
}

base::Thread& HostGetGameThread() {
    return *gpgt;
}

base::Thread *HostGetGameThreadPointer() {
    return gpgt;
}

void HostAppStop() {
    // Only use this function for when you can't post an appStopEvent

    ggame.SaveReinitializeGame();
    gevm.SetAppStopping();
    ggame.AskResignGame();
}

} // namespace wi
