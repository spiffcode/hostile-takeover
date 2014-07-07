#include "..\ht.h"

#define kidmScale1 100
#define kidmScale2 101
#define kidmScale3 102
#define kidmScale4 103
#define kidmScaleDefault 104

HWND Display::m_hwnd;
HMENU Display::m_hmenuPopup;
RECT g_rcOldWindowPos;

RECT g_arcSilkscreen[] = {
	{ 27 , 206 , (27+18) , (206+14)  },		// abc area
	{ 115 , 206 , (115+18) , (206+14)  },	// 123 area
	{ 0 , 164 , (0+27) , (164+28)  },		// launch button

	// WARNING: host.cpp code assumes the rectangle for the menu button 
	// is at index 3 (from 0) in this list
	{ 0 , 192 , (0+27) , (192+28)  },		// menu button 

	// WARNING: host.cpp code assumes the rectangle for the calc button 
	// is at index 4 (from 0) in this list
	{ 133 , 164 , (133+27) , (164+28)  },	// calculator button

	// WARNING: host.cpp code assumes the rectangle for the find button 
	// is at index 5 (from 0) in this list
	{ 133 , 192 , (133+27) , (192+28)  },	// find button

	// WARNING: HostGetSilkRect() assumes all these indexes
	{ 0, 0, 160 , 160  },				// screen
	{ 27 , 164 , (27+62) , (164+56)  },		// alpha rect
	{ 89 , 164 , (89+44) , (164+56)  },		// number rect
};

Display *HostCreateDisplay()
{
	// Make sure the Hostile Takeover window isn't underlaying the taskbar if
	// it happens to be along the left side of the desktop.

	if (g_rcOldWindowPos.left == 0 && g_rcOldWindowPos.top == 0) {
		POINT ptT = { 0, 0 };
		HMONITOR hmon = MonitorFromPoint(ptT, MONITOR_DEFAULTTOPRIMARY);

		MONITORINFOEX mi;
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(hmon, &mi);
		g_rcOldWindowPos.left = mi.rcWork.left;
		g_rcOldWindowPos.top = mi.rcWork.top;
	}

	// Create a display

	Display *pdisp = new Display();
	if (pdisp == NULL)
		return NULL;
	if (!pdisp->Init()) {
		delete pdisp;
		return NULL;
	}
	return pdisp;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT wm, WPARAM wp,	LPARAM lp)
{
	switch (wm) {
	case WM_CLOSE:
		PostMessage(NULL, WM_GAMEEVENT, appStopEvent, knAppExit);
		return 0;

	case WM_PAINT:
		if (gpmfrmm != NULL)
			gpmfrmm->InvalidateRect(NULL);
		PostMessage(NULL, WM_GAMEEVENT, gamePaintEvent, 0);
		break;

	case WM_RBUTTONDOWN:
		{
			POINT pt;
			pt.x = LOWORD(lp);
			pt.y = HIWORD(lp);
			MapWindowPoints(Display::m_hwnd, NULL, &pt, 1);
			int idm = TrackPopupMenu(Display::m_hmenuPopup,
					TPM_CENTERALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON,
					pt.x, pt.y, 0, Display::m_hwnd, NULL);
			switch (idm) {
			case kidmScale1:
				gpdisp->SetScale(1);
				break;

			case kidmScale2:
				gpdisp->SetScale(2);
				break;

			case kidmScale3:
				gpdisp->SetScale(3);
				break;

			case kidmScale4:
				gpdisp->SetScale(4);
				break;

			case kidmScaleDefault:
				gpdisp->SetScale(-1);
				break;
			}
		}
		break;

	case WM_ERASEBKGND:
		if (gpdisp != NULL) {
            ModeInfo mode;
			gpdisp->GetMode(&mode);
			if (mode.cyGraffiti != 0) {
				DefWindowProc(hwnd, wm, wp, lp);
				HDC hdc = (HDC)wp;
				HBRUSH hbrT = (HBRUSH)GetStockObject(WHITE_BRUSH);
				ModeInfo mode;
				gpdisp->GetMode(&mode);
				int nScale = gpdisp->GetScale() * mode.cx / 160;
				RECT *prcT = g_arcSilkscreen;
				for (int i = 0; i < sizeof(g_arcSilkscreen) / sizeof(RECT); i++, prcT++) {
					RECT rcT = *prcT;
					rcT.left = rcT.left * nScale;
					rcT.top = rcT.top * nScale;
					rcT.right = rcT.right * nScale;
					rcT.bottom = rcT.bottom * nScale;
					FrameRect(hdc, &rcT, hbrT);
				}
				return 1;
			}
		}
		break;
	}
	return DefWindowProc(hwnd, wm, wp, lp);
}

Display::Display()
{
	m_hwnd = NULL;
	m_hbm = NULL;
	m_hbmSav = NULL;
	m_hdc = NULL;
	m_hdcMem = NULL;
	m_imode = -1;
	m_cmodes = 0;
	m_pbmClip = NULL;
	m_pbmFront = NULL;
	m_pbmBack = NULL;
	m_nScale = 1;
	m_hmenuPopup = NULL;
}

Display::~Display()
{
	if (m_hdc != NULL) {
		ReleaseDC(m_hwnd, m_hdc);
		m_hdc = NULL;
	}
	if (m_hwnd != NULL) {
		GetWindowRect(m_hwnd, &g_rcOldWindowPos);
		DestroyWindow(m_hwnd);
		m_hwnd = NULL;
	}
	if (m_hbm != NULL) {
		if (m_hdcMem != NULL)
			SelectObject(m_hdcMem, (HGDIOBJ)m_hbmSav);
		DeleteObject(m_hbm);
		m_hbm = NULL;
	}
	if (m_hdcMem != NULL) {
		DeleteDC(m_hdcMem);
		m_hdcMem = NULL;
	}
	if (m_hmenuPopup != NULL) {
		DestroyMenu(m_hmenuPopup);
		m_hmenuPopup = NULL;
	}
	delete m_pbmBack;
	m_pbmBack = NULL;
	delete m_pbmFront;
	m_pbmFront = NULL;
	delete m_pbmClip;
	m_pbmClip = NULL;
}

bool Display::Init()
{
    WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = ghInst;
    wc.hIcon = LoadIcon(ghInst, MAKEINTRESOURCE(kidiMain));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
    wc.lpszClassName = kszWindowClass;
 	RegisterClass(&wc);

	// Adjust so the client area is the size we want

	dword dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN;
	RECT rc;
	SetRect(&rc, 0, 0, 320, 320 + (60 * 2 /* Graffiti area */));
	AdjustWindowRect(&rc, dwStyle, false);

	m_hwnd = CreateWindowEx(0, kszWindowClass, kszWindowTitle, 
			dwStyle, g_rcOldWindowPos.left, g_rcOldWindowPos.top, // CW_USEDEFAULT, CW_USEDEFAULT,
			rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, ghInst, 0);
	if (m_hwnd == NULL)
		return false;
	m_hdc = GetDC(m_hwnd);
	if (m_hdc == NULL)
		return false;
	m_hdcMem = CreateCompatibleDC(m_hdc);
	if (m_hdcMem == NULL)
		return false;

	// Create popup menu

	m_hmenuPopup = CreatePopupMenu();
	AppendMenu(m_hmenuPopup, MF_STRING, kidmScale1, "1x scale");
	AppendMenu(m_hmenuPopup, MF_STRING, kidmScale2, "2x scale");
	AppendMenu(m_hmenuPopup, MF_STRING, kidmScale3, "3x scale");
	AppendMenu(m_hmenuPopup, MF_STRING, kidmScale4, "4x scale");
	AppendMenu(m_hmenuPopup, MF_SEPARATOR, 0, NULL);
	AppendMenu(m_hmenuPopup, MF_STRING, kidmScaleDefault, "Default scale");

	// Set up mode list

	ModeInfo *pmode = m_amodeInfo;

	pmode->cx = 160;
	pmode->cy = 160;
	pmode->nDepth = 4;
	pmode->fNative = false;
	pmode->cyGraffiti = 60;
	pmode->nScale = 2;
	pmode->nDegreeOrientation = 0;
	pmode++;

	pmode->cx = 160;
	pmode->cy = 240;
	pmode->nDepth = 4;
	pmode->fNative = false;
	pmode->cyGraffiti = 0;
	pmode->nScale = 2;
	pmode->nDegreeOrientation = 0;
	pmode++;

	pmode->cx = 160;
	pmode->cy = 160;
	pmode->nDepth = 8;
	pmode->fNative = false;
	pmode->cyGraffiti = 60;
	pmode->nScale = 2;
	pmode->nDegreeOrientation = 0;
	pmode++;

	pmode->cx = 160;
	pmode->cy = 240;
	pmode->nDepth = 8;
	pmode->fNative = false;
	pmode->cyGraffiti = 0;
	pmode->nScale = 2;
	pmode->nDegreeOrientation = 0;
	pmode++;

	pmode->cx = 240;
	pmode->cy = 320;
	pmode->nDepth = 8;
	pmode->fNative = false;
	pmode->cyGraffiti = 0;
	pmode->nScale = 1;
	pmode->nDegreeOrientation = 0;
	pmode++;

	pmode->cx = 320;
	pmode->cy = 320;
	pmode->nDepth = 4;
	pmode->fNative = false;
	pmode->cyGraffiti = 120;
	pmode->nScale = 1;
	pmode->nDegreeOrientation = 0;
	pmode++;

	pmode->cx = 320;
	pmode->cy = 480;
	pmode->nDepth = 4;
	pmode->fNative = false;
	pmode->cyGraffiti = 0;
	pmode->nScale = 1;
	pmode->nDegreeOrientation = 0;
	pmode++;

	pmode->cx = 320;
	pmode->cy = 320;
	pmode->nDepth = 8;
	pmode->fNative = false;
	pmode->cyGraffiti = 120;
	pmode->nScale = 1;
	pmode->nDegreeOrientation = 0;
	pmode++;

	pmode->cx = 320;
	pmode->cy = 480;
	pmode->nDepth = 8;
	pmode->fNative = false;
	pmode->cyGraffiti = 0;
	pmode->nScale = 1;
	pmode->nDegreeOrientation = 0;
	pmode++;

	pmode->cx = 480;
	pmode->cy = 320;
	pmode->nDepth = 8;
	pmode->fNative = false;
	pmode->cyGraffiti = 0;
	pmode->nScale = 1;
	pmode->nDegreeOrientation = 0;
	pmode++;

	pmode->cx = 320;
	pmode->cy = 240;
	pmode->nDepth = 8;
	pmode->fNative = false;
	pmode->cyGraffiti = 0;
	pmode->nScale = 1;
	pmode->nDegreeOrientation = 0;
	pmode++;

	pmode->cx = 640;
	pmode->cy = 480;
	pmode->nDepth = 8;
	pmode->fNative = false;
	pmode->cyGraffiti = 0;
	pmode->nScale = 1;
	pmode->nDegreeOrientation = 0;
	pmode++;

	m_cmodes = pmode - m_amodeInfo;

	return true;
}

bool Display::SetMode(int imode, int nScale)
{
	ModeInfo *pmode = &m_amodeInfo[imode];

	// Create dib bitmap

	DibBitmap *pbmBack = CreateDibBitmap(NULL, pmode->cx, pmode->cy);
	if (pbmBack == NULL)
		return false;

	// Create display surface with default palm palette

	struct BitmapInfo // bi
	{
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD bmiColors[256];
	};
	BitmapInfo bi;
	memset(&bi, 0, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = pmode->cx;
	bi.bmiHeader.biHeight = -pmode->cy;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 8;
	bi.bmiHeader.biCompression = BI_RGB;
	switch (pmode->nDepth) {
	case 4:
		bi.bmiHeader.biClrUsed = 16;
		break;

	case 8:
		bi.bmiHeader.biClrUsed = 256;
		break;
	}

	byte *pbScreen;
	HBITMAP hbm = CreateDIBSection(m_hdcMem, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (void **)&pbScreen, NULL, 0);
	if (hbm == NULL) {
		delete pbmBack;
		return false;
	}

	// Success

	if (m_hbm != NULL) {
		SelectObject(m_hdcMem, (HGDIOBJ)m_hbmSav);
		DeleteObject(m_hbm);
		m_hbm = NULL;
		delete m_pbmBack;
		m_pbmBack = NULL;
		delete m_pbmFront;
		m_pbmFront = NULL;
	}

	DibBitmap *pbmFront = CreateDibBitmap(pbScreen, pmode->cx, pmode->cy);
	if (pbmFront == NULL) {
		delete pbmBack;
		return false;
	}

	m_hbm = hbm;
	m_hbmSav = (HBITMAP)SelectObject(m_hdcMem, (HGDIOBJ)m_hbm);
	m_imode = imode;
	m_pbmBack = pbmBack;
	m_pbmFront = pbmFront;

	// Scale the window

	if (nScale <= 0)
		nScale = m_amodeInfo[imode].nScale;
	SetScale(nScale);
	return true;
}

void Display::SetScale(int nScale)
{
	if (nScale <= 0)
		nScale = m_amodeInfo[m_imode].nScale;
	m_nScale = nScale;
	ResizeWindow(nScale);
}

void Display::ResizeWindow(int nScale)
{
	// Size window to the requested size

	ModeInfo *pmode = &m_amodeInfo[m_imode];
	int cx = pmode->cx * nScale;
	int cy = (pmode->cy + pmode->cyGraffiti) * nScale;
	dword dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN;
	RECT rc;
	SetRect(&rc, 0, 0, cx, cy);
	AdjustWindowRect(&rc, dwStyle, false);

	// Hack: If we're running in multiplayer test mode, position this window according to side represented

	int x = g_rcOldWindowPos.left;
	int y = g_rcOldWindowPos.top;
#ifdef MP_STRESS
	if (gnMPPos != 0)
		x += (rc.right - rc.left) * gnMPPos;
#endif

	SetWindowPos(m_hwnd, NULL, x, y, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_SHOWWINDOW);
	InvalidateRect(m_hwnd, NULL, TRUE);
}

int Display::GetMode(ModeInfo *pmode)
{
	if (pmode != NULL) {
		if (m_imode == -1) {
			memset(pmode, 0, sizeof(*pmode));
		} else {
			*pmode = m_amodeInfo[m_imode];
		}
	}
	return m_imode;
}

int Display::GetModeCount()
{
	return m_cmodes;
}

void Display::GetModeInfo(int imode, ModeInfo *pmode)
{
	memset(pmode, 0, sizeof(*pmode));
	if (imode >= 0 && imode < m_cmodes)
		*pmode = m_amodeInfo[imode];
}

DibBitmap *Display::GetBackDib()
{
	return m_pbmBack;
}

DibBitmap *Display::GetFrontDib()
{
	return m_pbmFront;
}

DibBitmap *Display::GetClippingDib()
{
	if (m_pbmClip != NULL)
		return m_pbmClip;
	DibBitmap *pbm = CreateDibBitmap(NULL, kcCopyBy4Procs * 4, kcCopyBy4Procs * 4);
	if (pbm == NULL)
		return NULL;
	m_pbmClip = pbm;
	return pbm;
}

void Display::FrameStart()
{
}

void Display::FrameComplete()
{
	// Update screen
	
	int cx = m_amodeInfo[m_imode].cx;
	int cy = m_amodeInfo[m_imode].cy;
	StretchBlt(m_hdc, 0, 0, cx * m_nScale, cy * m_nScale, m_hdcMem, 0, 0, cx, cy, SRCCOPY);

#ifdef DEBUG_HELPERS
	extern void paint();
	paint();
#endif
}

void Display::SetPalette(Palette *ppal)
{
	RGBQUAD *prgbq = new RGBQUAD[BigWord(ppal->cEntries)];
	for (int n = 0; n < BigWord(ppal->cEntries); n++) {
		prgbq[n].rgbRed = ppal->argb[n][0];
		prgbq[n].rgbGreen = ppal->argb[n][1];
		prgbq[n].rgbBlue = ppal->argb[n][2];
		prgbq[n].rgbReserved = 0;
	}
	SetDIBColorTable(m_hdcMem, 0, BigWord(ppal->cEntries), prgbq);
	delete prgbq;
}

void Display::GetWindowsPalette(RGBQUAD *pargbq)
{
	GetDIBColorTable(m_hdcMem, 0, 256, pargbq);
}

void Display::DrawText(const char *psz, int x, int y, word wf)
{
	if (y == -1)
		y = m_amodeInfo[m_imode].cy * m_nScale - 16;

	RECT rc;
	rc.left = 0;
	rc.top = y;
	if (wf & kfDtClearLine) {
		rc.right = m_amodeInfo[m_imode].cx * m_nScale;
		rc.bottom = rc.top + 16;
		HBRUSH hbrT = CreateSolidBrush(RGB(0, 0, 0));
		FillRect(m_hdc, &rc, hbrT);
		DeleteObject(hbrT);
	}
	SetTextColor(m_hdc, RGB(255, 255, 255));
	SetBkColor(m_hdc, RGB(0, 0, 0));
	TextOut(m_hdc, rc.left, rc.top, psz, strlen(psz));
	GdiFlush();
}

void Display::DrawFrameInclusive(Rect *prc)
{
	RECT rcT;
	SetRect(&rcT, prc->left * m_nScale, prc->top * m_nScale, prc->right * m_nScale, prc->bottom * m_nScale);
	HBRUSH hbr = CreateSolidBrush(RGB(255, 255, 0));
	FrameRect(m_hdc, &rcT, hbr);
	DeleteObject((HGDIOBJ)hbr);
	GdiFlush();
}

void Display::GetHslAdjustments(short *pnHueOffset, short *pnSatMultiplier, short *pnLumOffset)
{
	*pnHueOffset = 0;
	*pnSatMultiplier = 0;
	*pnLumOffset = 0;
}
