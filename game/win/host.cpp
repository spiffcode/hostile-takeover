#include <stdio.h>
#include "..\ht.h"

extern RECT g_arcSilkscreen[];
static MSG s_msgMouseLast;
static dword s_tLastMouseMsg;
int gxPenLast;
int gyPenLast;

void GetSilkRect(RECT *prcIn, RECT *prcOut, bool fScale)
{
	*prcOut = *prcIn;
	if (gpdisp != NULL) {
		ModeInfo mode;
		gpdisp->GetMode(&mode);
		int nScale;
		if (fScale) {
			nScale = gpdisp->GetScale() * mode.cx / 160;
		} else {
			nScale = mode.cx / 160;
		}
		prcOut->left = prcIn->left * nScale;
		prcOut->top = prcIn->top * nScale;
		prcOut->right = prcIn->right * nScale;
		prcOut->bottom = prcIn->bottom * nScale;
	}
}

bool WaitForInput(dword msStart, dword cmsWaitTotal)
{
	dword cmsWait = cmsWaitTotal;
	if (cmsWaitTotal != INFINITE) {
		dword cmsElapsed = GetTickCount() - msStart;
		if (cmsElapsed > cmsWaitTotal)
			return false;
		cmsWait = cmsWaitTotal - cmsElapsed;
	}
	dword dw = MsgWaitForMultipleObjects(0, NULL, TRUE, cmsWait, QS_ALLEVENTS);
	return dw != WAIT_TIMEOUT;
}

void ProcessKeyEvent(MSG *pmsg, Event *pevt)
{
	// Palm mixes "chars" with "vkeys", however with values that don't
	// overlap. The host will treat this the same.

	pevt->eType = keyDownEvent;
	switch (pmsg->wParam) {
	case VK_UP:
		pevt->chr = chrPageUp;
		return;

	case VK_DOWN:
		pevt->chr = chrPageDown;
		return;

	case VK_LEFT:
		pevt->chr = chrLeft;
		return;

	case VK_RIGHT:
		pevt->chr = chrRight;
		return;

	case VK_BACK:
		pevt->chr = chrBackspace;
		return;

	case VK_DELETE:
		pevt->chr = chrDelete;
		return;

	case VK_F7:
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

	case VK_F8:
		if (gpavir != NULL) {
			gpavir->Stop();
			delete gpavir;
			gpavir = NULL;
		}
		break;

	case VK_SUBTRACT:	// numpad
	case VK_OEM_MINUS:	// '-' key
		{
			for (int i = 0; i < ARRAYSIZE(gatGameSpeeds); i++)
				if (gatGameSpeeds[i] == gtGameSpeed)
					break;
			i--;
			if (i < 0)
				i = 0;
			ggame.SetGameSpeed(gatGameSpeeds[i]);
		}
		break;

	case VK_ADD:		// numpad
	case VK_OEM_PLUS:	// '=' key
		{
			for (int i = 0; i < ARRAYSIZE(gatGameSpeeds); i++)
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
		pevt->chr = pmsg->wParam;
		break;
	}
	if (!TranslateMessage(pmsg))
		return;

	// A WM_CHAR *may* have been posted to the queue - we don't know for
	// sure until we take a peek.

	MSG msg;
	if (PeekMessage(&msg, NULL, WM_CHAR, WM_CHAR, TRUE))
		pevt->chr = msg.wParam;
}

bool ProcessMouseEvent(MSG *pmsg, Event *pevt, bool fHover = false)
{
	static bool s_fAppButtonDown;
	POINT ptT;

#ifdef DEBUG_HELPERS
	// This allows the debug window to get events it needs
 
	extern HWND s_hwndDebug;
	if (pmsg->hwnd != Display::m_hwnd)
		if (pmsg->hwnd != s_hwndDebug || LOWORD(pmsg->lParam) >= 320)
			return false;
#endif

	// Scale the position appropriately.

	int nScale = gpdisp->GetScale();
	gxPenLast = (int)((float)(short)LOWORD(pmsg->lParam) / (float)nScale);
	gyPenLast = (int)((float)(short)HIWORD(pmsg->lParam) / (float)nScale);

	// Remember the last

	s_msgMouseLast = *pmsg;
	s_tLastMouseMsg = GetTickCount();

	// Setup

	RECT rcApp;
	GetSilkRect(&g_arcSilkscreen[2], &rcApp, true);
	bool fGraffitiArea = false;
	if (gpdisp != NULL) {
		ModeInfo mode;
		gpdisp->GetMode(&mode);
		fGraffitiArea = (mode.cyGraffiti != 0);
	}

	switch (pmsg->message) {
	case WM_LBUTTONDOWN:
		pevt->eType = penDownEvent;
		SetCapture(pmsg->hwnd);

		ptT.x = LOWORD(pmsg->lParam);
		ptT.y = HIWORD(pmsg->lParam);
		if (fGraffitiArea) {
			if (PtInRect(&rcApp, ptT))
				s_fAppButtonDown = true;
		}
		break;

	case WM_LBUTTONUP:
		pevt->eType = penUpEvent;
		ReleaseCapture();

		// Check for soft silk screen buttons.

		if (fGraffitiArea) {
			ptT.x = LOWORD(pmsg->lParam);
			ptT.y = HIWORD(pmsg->lParam);
			if (s_fAppButtonDown && PtInRect(&rcApp, ptT))
				PostMessage(pmsg->hwnd, WM_GAMEEVENT, appStopEvent, 0);
			s_fAppButtonDown = false;
		}
		break;

	case WM_MOUSEMOVE:
		if (GetAsyncKeyState(VK_LBUTTON) < 0) {
			pevt->eType = penMoveEvent;
			break;
		}
		if (fHover)
			break;
		return false;

#if 0
#ifdef HOSTILE_TAKEOVER
	case WM_RBUTTONDOWN:
		pevt->chr = chrPageDown;
		pevt->eType = keyDownEvent;
		break;
#endif
#else
	case WM_RBUTTONDOWN:
		return false;
#endif
	}

	pevt->x = gxPenLast;
	pevt->y = gyPenLast;
	pevt->dw = 0;
	return true;
}

bool ProcessEvent(MSG *pmsg, Event *pevt)
{
	// Is it something the host will not process?

	switch (pmsg->message) {
#ifdef HOSTILE_TAKEOVER
	case WM_RBUTTONDOWN:
#endif
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MOUSEMOVE:
		return ProcessMouseEvent(pmsg, pevt);

	case WM_KEYDOWN:
		ProcessKeyEvent(pmsg, pevt);
		return true;

	case WM_GAMEEVENT:
		pevt->eType = pmsg->wParam;
		pevt->dw = pmsg->lParam;
		return true;
	}

	return false;
}

#define kcmsMouseHover 1000

bool HostGetEvent(Event *pevt, long ctWait)
{
	GdiFlush();

	// See if we have a hover event

	if (s_msgMouseLast.message == WM_MOUSEMOVE && (GetTickCount() - s_tLastMouseMsg) >= kcmsMouseHover) {
		ProcessMouseEvent(&s_msgMouseLast, pevt, true);
		pevt->eType = penHoverEvent;
		s_msgMouseLast.message = 0;
		return true;
	}

	// Check for input from the queue first

	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, TRUE)) {
		if (ProcessEvent(&msg, pevt)) 
			return true;
		DispatchMessage(&msg);
	}

	// No input; wait for it

	if (ctWait == 0)
		return false;

	dword msStart = GetTickCount();
	dword cmsWaitTotal = INFINITE;
	if (ctWait != -1) {
		cmsWaitTotal = ctWait * 10;
	}

	while (true) {
		// false means a timeout occured

		if (!WaitForInput(msStart, cmsWaitTotal))
			return false;

		// We have input. See if it is returnable

		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, TRUE)) {
			if (ProcessEvent(&msg, pevt)) 
				return true;
			DispatchMessage(&msg);
		}
	}
}

void HostOutputDebugString(char *pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	ReporterOutputDebugString(pszFormat, va);
	va_end(va);
}

long HostGetTickCount()
{
	return (long)(GetTickCount() / 10);
}

long HostRunSpeedTests(DibBitmap *pbmSrc)
{
	return 0;
}

dword HostGetCurrentKeyState(dword keyBit)
{
	if (GetForegroundWindow() != Display::m_hwnd)
		return 0;

	struct KeyBitVKey {
		dword keyBit;
		short vk;
	} s_akk[] = {
//		{ keyBitPower, 0 },
#ifdef HOSTILE_TAKEOVER
		{ keyBitPageUp, VK_SHIFT },
		{ keyBitPageUp, VK_UP },
		{ keyBitPageDown, VK_DOWN },
#else
		{ keyBitPageUp, VK_PRIOR },
#endif
		{ keyBitPageDown, VK_NEXT },
		{ keyBitHard1, VK_INSERT },
		{ keyBitHard2, VK_HOME },
		{ keyBitHard3, VK_DELETE },
		{ keyBitHard4, VK_END },
//		{ keyBitCradle, 0 },
//		{ keyBitAntenna, 0 },
//		{ keyBitContrast, 0 },
		{ keyBitDpadLeft, VK_LEFT },
		{ keyBitDpadRight, VK_RIGHT },
		{ keyBitDpadButton, VK_RETURN },
	};

	Assert(keyBit != 0);

	dword keyBitRet = 0;
	for (int i = 0; i < sizeof(s_akk) / sizeof(KeyBitVKey); i++) {
		if ((keyBit & s_akk[i].keyBit) != 0) {
			if (GetAsyncKeyState(s_akk[i].vk) < 0)
				keyBitRet |= s_akk[i].keyBit;
		}
	}

	return keyBitRet;
}

bool HostIsPenDown()
{
	return GetAsyncKeyState(VK_LBUTTON) < 0;
}

void HostMessageBox(TCHAR *pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	TCHAR sz[512];
	vsprintf(sz, pszFormat, va);
	MessageBox(NULL, sz, TEXT("Message"), MB_OK);
	va_end(va);
}

void HostGetUserName(char *pszBuff, int cb)
{
	// Can fail if WSAStartup hasn't been called yet

	if (gethostname(pszBuff, cb - 1) != 0) {
		strncpyz(pszBuff, "bogus name", cb);
	}
}

bool HostGetOwnerName(char *pszBuff, int cb, bool fShowError)
{
	if (gethostname(pszBuff, cb - 1) != 0) {
		if (fShowError)
			HtMessageBox(kfMbClearDib, "Hostile Takeover", "This PC has an invalid computer name! Set the computer name and try again.");
		return false;
	}
	return true;
}

// Override packfile.h's overrides

#undef FILE
#undef fopen
#undef fclose
#undef fread
#undef fwrite

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
	dword cbWritten = fwrite(pv, 1, cb, (FILE *)hf);
#ifdef DEBUG
	fflush((FILE *)hf);
#endif
	return cbWritten;
}

dword HostReadFile(FileHandle hf, void *pv, dword cb)
{
	return fread(pv, 1, cb, (FILE *)hf);
}

void HostSleep(dword ct)
{
	Sleep(ct * 10);
}

void HostGetSilkRect(int irc, Rect *prc)
{
	RECT rcT;
	switch (irc) {
	case kircSilkGraffiti:
		rcT.left = g_arcSilkscreen[7].left;
		rcT.top = g_arcSilkscreen[7].top;
		rcT.right = g_arcSilkscreen[8].right;
		rcT.bottom = g_arcSilkscreen[7].bottom;
		break;

	case kircSilkApps:
		rcT = g_arcSilkscreen[2];
		break;

	case kircSilkMenu:
		rcT = g_arcSilkscreen[3];
		break;

	case kircSilkCalc:
		rcT = g_arcSilkscreen[4];
		break;

	case kircSilkFind:
		rcT = g_arcSilkscreen[5];
		break;
	}

	GetSilkRect(&rcT, &rcT, false);
	prc->left = rcT.left;
	prc->top = rcT.top;
	prc->right = rcT.right;
	prc->bottom = rcT.bottom;
}

// Figure out what kind of sound device exists, and return a SoundDevice for it

SoundDevice *HostOpenSoundDevice()
{
	return CreateWin32SoundDevice();
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
	SYSTEMTIME time;
	GetLocalTime(&time);
	pdate->nYear = time.wYear;
	pdate->nMonth = time.wMonth;
	pdate->nDay = time.wDay;
}

#define kszRegKey TEXT("Software\\Spiffcode\\Takeover")
#define kszRegValue TEXT("Preferences")

bool HostSavePreferences(void *pv, int cb)
{
	// Open key / create if it doesn't exist

	HKEY hkey;
	DWORD dwDisposition;
	LRESULT lr = RegCreateKeyEx(HKEY_CURRENT_USER, kszRegKey, 0, "", 0, KEY_WRITE, NULL, &hkey, &dwDisposition);
	if (lr != ERROR_SUCCESS)
		return false;

	// Set value

	lr = RegSetValueEx(hkey, kszRegValue, 0, REG_BINARY, (BYTE *)pv, cb);
	if (lr != ERROR_SUCCESS) {
		RegCloseKey(hkey);
		return false;
	}

	// Done

	RegCloseKey(hkey);
	return true;
}

int HostLoadPreferences(void *pv, int cb)
{
	// Key exist?

	HKEY hkey;
	LRESULT lr = RegOpenKeyEx(HKEY_CURRENT_USER, kszRegKey, 0, KEY_READ, &hkey);
	if (lr != ERROR_SUCCESS)
		return -1;

	// Read the data

	dword cbT = (dword)cb;
	lr = RegQueryValueEx(hkey, kszRegValue, NULL, NULL, (BYTE *)pv, &cbT);
	if (lr != ERROR_SUCCESS) {
		byte *pb = new byte[cbT];
		lr = RegQueryValueEx(hkey, kszRegValue, NULL, NULL, (BYTE *)pb, &cbT);
		if (lr == ERROR_SUCCESS) {
			cbT = min(cb, (int)cbT);
			memcpy(pv, pb, cbT);
			delete pb;
		} else {
			RegCloseKey(hkey);
			return -1;
		}
	}

	// Close and return size read

	RegCloseKey(hkey);
	return (int)cbT;
}

char *HostGetDataDirectory()
{
	// The data directory is the where the executable is executing from
	// On Windows, use the current directory
	// On CE, use the directory the executable is in (mostly because storage cards aren't
	// the "current directory" when an app is executed from the CE launcher.

	char szWorkingDir[_MAX_PATH];
	GetCurrentDirectory(sizeof(szWorkingDir) - 1, szWorkingDir);

	// Return it

	static char s_szDataDir[_MAX_PATH];
	strcpy(s_szDataDir, szWorkingDir);
	return s_szDataDir;
}

void HostSuspendModalLoop(DibBitmap *pbm)
{
}

void HostNotEnoughMemory(bool fStorage, dword cbFree, dword cbNeed)
{
	HostMessageBox(TEXT("Need %ld bytes of memory but only %ld bytes are free!"), cbNeed, cbFree);
}

bool HostEnumAddonFiles(Enum *penm, char *pszAddon, int cb)
{
	WIN32_FIND_DATA find;

	if (penm->m_wUser == (word)kEnmFirst) {
		penm->m_wUser = 0;

		char szFileSpec[_MAX_PATH];
		PrependDataDirectory("*.pdb", szFileSpec);

		TCHAR szT[_MAX_PATH];
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, szFileSpec, -1, szT, ARRAYSIZE(szT) - 1);
#else
		strcpy(szT, szFileSpec);
#endif

		penm->m_pvNext = (void *)FindFirstFile(szT, &find);
		if (penm->m_pvNext == NULL)
			return false;
	} else {
		if (FindNextFile((HANDLE)penm->m_pvNext, &find) == 0) {
			FindClose((HANDLE)penm->m_pvNext);
			return false;
		}
	}

	while (true) {
		// Get file

		char szFilename[_MAX_PATH];
#ifdef UNICODE
		WideCharToMultiByte(CP_ACP, 0, find.cFileName, -1, szFilename, ARRAYSIZE(szFilename) - 1, NULL, NULL);
#else
		strcpy(szFilename, find.cFileName);
#endif

		char szPath[_MAX_PATH];
		PrependDataDirectory(szFilename, szPath);

		// See if it is an extension file

		FILE *pf = fopen(szPath, "rb");
		if (pf != NULL) {
			char szType[5];
			szType[4] = 0;
			fseek(pf, 0x3c, SEEK_SET);
			fread(szType, 4, 1, pf);
			fclose(pf);
			if (strcmp(szType, kszTypeAddon) == 0) {
				strncpyz(pszAddon, szFilename, cb);
				return true;
			}
		}

		if (FindNextFile((HANDLE)penm->m_pvNext, &find) == 0) {
			FindClose((HANDLE)penm->m_pvNext);
			return false;
		}
	}
}