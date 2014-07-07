#include "..\ht.h"

#ifdef DEBUG_HELPERS

#include <windowsx.h>

#ifdef GDIPLUS
#include <gdiplus.h>
using namespace Gdiplus;
struct GdiplusStartupInput s_siGdiPlus;
ULONG_PTR s_pulGdiPlusToken;
#endif

const int kcySmallFont = 13;
const int kcyLabel = kcySmallFont - 2;
const int kcxDebugWindow = 320 + 250;
const int kcyDebugWindow = 480;
const int kcxLogWindow = 450;
const int kcyButton = 16;

struct Toggle {
	word vk;
	char *sz;
	bool f;
	HWND hwnd;
};

#define kidShowSelectedUnitsOnly 0
#define kidShowStateMachineLog 1
#define kidShowHealth 2
#define kidShowState 3
#define kidShowAction 4
#define kidShowMuntFlags 5
#define kidShowUnitGroup 6
#define kidShowFiringRange 7
#define kidShowOccupied 8
#define kidShowGobPointer 9
#define kidShowMuntAggressiveness 10
#define kidDebugSelectedStateMachine 11
#define kidSuspendUpdates 12
#define kidShowCoordinates 13
#define kidShowDst 14
#define kidShowUpdaters 15
#define kidStep 16
#define kidToggleInvincibility 17

static Toggle s_atgl[] = {
	{ 'T', "Only show selected units' info (t)", false, NULL },
	{ 'L', "Show StateMachine log (l)", false, NULL },
	{ 'H', "Show Unit health (h)", false, NULL },
	{ 'S', "Show Unit state (s)", true, NULL },
	{ 'A', "Show Mobile Unit actions (a)", true, NULL },
	{ 'F', "Show Mobile Unit flags (f)", false, NULL },
	{ 'G', "Show Unit group (g)", false, NULL },
	{ 'I', "Show Unit firing range (i)", false, NULL },
	{ 'O', "Show occupied tiles (o)", false, NULL },
	{ 'B', "Show Gobs' memory address (b)", false, NULL },
	{ 'E', "Show Unit aggressiveness (e)", true , NULL },
	{ 'M', "Debug selected StateMachine (m)", false, NULL },
	{ 'U', "Suspend Updates (u)", false, NULL },
	{ 'C', "Show Coordinates (c)", false, NULL },
	{ 'Q', "Show txDst, tyDst (q)", false, NULL },
	{ 'Z', "Show Updaters", false, NULL },
	{ VK_SPACE, "Single Step (spacebar)", false, (HWND)1 },
	{ 'V', "Toggle invincibility", false, (HWND)1 },
//	{ 'D', "Debug selected Unit (d)", false, NULL },
};

class DebugViewer {
public:
	DebugViewer(char *pszTitle, int x, int y, int cx, int cy);
	~DebugViewer();
	void ToggleVisibility();
	void ScrollTo(int dx, int dy);

	virtual void Update();
	virtual void Paint(HDC hdc);
	virtual void OnRightClick(int x, int y);

protected:
	int m_xView, m_yView;
	HWND m_hwnd;
};

class TriggerViewer : public DebugViewer {
public:
	TriggerViewer();
	virtual void Paint(HDC hdc);
	virtual void OnRightClick(int x, int y);
};

class UnitGroupViewer : public DebugViewer {
public:
	UnitGroupViewer();
	virtual void Paint(HDC hdc);
};

class DelayedMessageViewer : public DebugViewer {
public:
	DelayedMessageViewer();
	virtual void Update();
	virtual void Paint(HDC hdc);
};

class CommandQueueViewer : public DebugViewer {
public:
	CommandQueueViewer();
	virtual void Update();
	virtual void Paint(HDC hdc);
};

static TriggerViewer *s_ptgrv;
static UnitGroupViewer *s_pugv;
static DelayedMessageViewer *s_pdmv;
static CommandQueueViewer *s_pcmdqv;

HWND s_hwndDebug;
bool gfBack;
extern RECT g_rcOldWindowPos;
static int s_dyLabel;
HFONT s_hfntSmall;

char Condition::s_szDebugHelpers[300];
char QualifiedNumber::s_szDebugHelpers[200];
char TriggerAction::s_szDebugHelpers[200];
char UnitGroupAction::s_szDebugHelpers[200];

// Forward declarations

LRESULT CALLBACK DebugWndProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp);
void UpdateControlStates();
void DebugHelperCommandHandler(WPARAM wp, LPARAM lp);
void DrawLabel(HDC hdc, int x, int y, char *psz);

void InitLog();
void ExitLog();
void DrawLog(HDC hdc);

//

void InitDebugHelpers()
{
    WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = DebugWndProc;
    wc.hInstance = ghInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
    wc.lpszClassName = "HostileTakeoverDebug";
 	RegisterClass(&wc);

	POINT ptT = { 0, 0 };
	HMONITOR hmon = MonitorFromPoint(ptT, MONITOR_DEFAULTTOPRIMARY);

	MONITORINFOEX mi;
	mi.cbSize = sizeof(mi);
	GetMonitorInfo(hmon, &mi);

	// Adjust so the client area is the size we want

	dword dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN;

	RECT rc;
	SetRect(&rc, 0, 0, kcxDebugWindow, kcyDebugWindow);
	AdjustWindowRect(&rc, dwStyle, false);
	s_hwndDebug = CreateWindowEx(WS_EX_TOOLWINDOW, "HostileTakeoverDebug", "HT Debug",
			dwStyle, mi.rcWork.left, mi.rcWork.bottom - (rc.bottom - rc.top), // g_rcOldWindowPos.top + 320 + (60 * 2 /* Graffiti area */) + 30 /* title bar */,
			rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, ghInst, 0);

	// Create checkboxes/buttons

	int y = 0;
	for (int i = 0; i < ARRAYSIZE(s_atgl); i++) {
		s_atgl[i].hwnd = CreateWindow("BUTTON", s_atgl[i].sz, WS_VISIBLE | WS_CHILD | (s_atgl[i].hwnd == NULL ? BS_CHECKBOX : 0), 
				320, y, 250, kcyButton, s_hwndDebug, (HMENU)i, NULL, NULL);
		y += kcyButton;
	}

	// Update their state

	UpdateControlStates();

#ifdef GDIPLUS
   // Initialize GDI+

   GdiplusStartup(&s_pulGdiPlusToken, &s_siGdiPlus, NULL);
#endif

	HFONT hfntSystem = (HFONT)GetStockObject(SYSTEM_FONT);
	LOGFONT lf;
	::GetObject(hfntSystem, sizeof(lf), &lf);
	lf.lfWeight = FW_LIGHT;
	lf.lfHeight = 6;
	strcpy(lf.lfFaceName, "Helv");
	s_hfntSmall = ::CreateFontIndirect(&lf);

	InitLog();
	s_ptgrv = new TriggerViewer();
	s_pugv = new UnitGroupViewer();
	s_pdmv = new DelayedMessageViewer();
	s_pcmdqv = new CommandQueueViewer();
}

void ExitDebugHelpers()
{
	DeleteObject(s_hfntSmall);
	ExitLog();
	DestroyWindow(s_hwndDebug);
#ifdef GDIPLUS
	GdiplusShutdown(s_pulGdiPlusToken);
#endif
	delete s_ptgrv;
	delete s_pugv;
	delete s_pdmv;
	delete s_pcmdqv;
}

LRESULT CALLBACK DebugWndProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp)
{
	extern void DebugHelperCommandHandler(WPARAM wp, LPARAM lp);

	switch (wm) {
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
		}
		break;

	case WM_COMMAND:
		DebugHelperCommandHandler(wp, lp);
		break;

	case WM_CLOSE:
		ShowWindow(hwnd, false);
		return 1;

	case WM_ERASEBKGND:
		break;
	}
	return DefWindowProc(hwnd, wm, wp, lp);
}

void DebugHelperKeyHandler(word vk)
{
	for (int i = 0; i < ARRAYSIZE(s_atgl); i++) {
		if (vk == s_atgl[i].vk) {
			DebugHelperCommandHandler(i, 0);
			return;
		}
	}

	switch (vk) {
	case VK_TAB:
		{
			extern void show(bool fBackBuffer);
			extern void hide();
			if (IsWindowVisible(s_hwndDebug))
				hide();
			else
				show(GetKeyState(VK_CONTROL) < 0);
		}
		break;

	case VK_F1:
		s_ptgrv->ToggleVisibility();
		break;

	case VK_F2:
		s_pugv->ToggleVisibility();
		break;

	case VK_F3:
		s_pdmv->ToggleVisibility();
		break;

	case VK_F4:
		s_pcmdqv->ToggleVisibility();
		break;
	}
}

void DebugHelperCommandHandler(WPARAM wp, LPARAM lp)
{
	extern bool gfSuspendUpdates;
	extern bool gfSingleStep;

	int id = LOWORD(wp);
	Toggle *ptgl = &s_atgl[id];
	ptgl->f = !ptgl->f;

	switch (id) {
	case kidSuspendUpdates:
		gfSuspendUpdates = ptgl->f;
		break;

	case kidStep:
		if (gfSuspendUpdates)
			gfSingleStep = true;
		break;

	case kidToggleInvincibility:
		for (Gob *pgob = ggobm.GetFirstGob(); pgob != NULL; pgob = ggobm.GetNextGob(pgob)) {
			if ((pgob->GetFlags() & (kfGobSelected | kfGobUnit)) != (kfGobSelected | kfGobUnit))
				continue;

			UnitGob *punt = (UnitGob *)pgob;
			punt->SetUnitFlags(punt->GetUnitFlags() ^ kfUnitInvulnerable);
		}
		break;

	case kidShowStateMachineLog:
		{
			RECT rc;
			GetWindowRect(s_hwndDebug, &rc);
			int cx = rc.right - rc.left;
			int cy = rc.bottom - rc.top;
			if (ptgl->f) {
				// Show
				SetWindowPos(s_hwndDebug, NULL, 0, 0, cx + kcxLogWindow, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			} else {
				// Hide
				SetWindowPos(s_hwndDebug, NULL, 0, 0, cx - kcxLogWindow, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}
		break;
	}

	UpdateControlStates();
}

void UpdateControlStates()
{
	for (int i = 0; i < ARRAYSIZE(s_atgl); i++)
		SendMessage(s_atgl[i].hwnd, BM_SETCHECK, (WPARAM)(s_atgl[i].f ? BST_CHECKED : BST_UNCHECKED), 0);
}

// Debug helpers are named in lowercase to facilite easy use

void show(bool fBack)
{
	extern void paint();
	gfBack = fBack;
	SetWindowText(s_hwndDebug, fBack ? "HT Debug - Back Buffer" : "HT Debug - Front Buffer");
	ShowWindow(s_hwndDebug, TRUE);
	paint();
}

void hide()
{
	ShowWindow(s_hwndDebug, FALSE);
}

void showtile(TCoord tx, TCoord ty)
{
	if (s_hwndDebug == NULL || !IsWindowVisible(s_hwndDebug))
		return;

	WCoord wxView, wyView;
	gsim.GetViewPos(&wxView, &wyView);
	short xView = PcFromUwc(wxView) & 0xfffe;
	short yView = PcFromUwc(wyView) & 0xfffe;
	int x = PcFromTc(tx) - xView;
	int y = PcFromTc(ty) - yView;

	HBRUSH hbr = CreateSolidBrush(RGB(255, 255, 100));
	RECT rcT;
	rcT.left = x;
	rcT.top = y;
	rcT.right = x + PcFromTc(1);
	rcT.bottom = y + PcFromTc(1);
	extern Display *gpdisp;

	HDC hdc = GetDC(s_hwndDebug);
	RECT rcT2;
	rcT2 = rcT;
	rcT2.right = rcT2.left + 1;
	FillRect(hdc, &rcT2, hbr);
	rcT2 = rcT;
	rcT2.left = rcT2.right - 1;
	FillRect(hdc, &rcT2, hbr);
	rcT2 = rcT;
	rcT2.bottom = rcT2.top + 1;
	FillRect(hdc, &rcT2, hbr);
	rcT2 = rcT;
	rcT2.top = rcT2.bottom - 1;
	FillRect(hdc, &rcT2, hbr);

	ReleaseDC(s_hwndDebug, hdc);

	DeleteObject(hbr);
}

// Side colors

#ifdef GDIPLUS
static Gdiplus::Color s_aclr[] = {
	Gdiplus::Color(0, 0, 0),
	Gdiplus::Color(40, 0, 116, 255),
	Gdiplus::Color(40, 255, 32, 0),
	Gdiplus::Color(40, 255, 228, 0),
	Gdiplus::Color(40, 104, 255, 255)
};
#else
static COLORREF s_acr[] = {
	RGB(0, 0, 0),
	RGB(0, 116, 255),
	RGB(255, 32, 0),
	RGB(255, 228, 0),
	RGB(104, 255, 255),
};
#endif

char *s_aszActions[] = {
	"None",
	"Guard",
	"GuardVicinity",
	"GuardArea",
	"Move",
	"Attack",
	"HuntEnemies",
};

void paint()
{
	extern Display *gpdisp;

	if (s_hwndDebug == NULL || !IsWindowVisible(s_hwndDebug))
		return;

	ModeInfo mode;
	gpdisp->GetMode(&mode);
	DibBitmap *pbm = gfBack ? gpdisp->GetBackDib() : gpdisp->GetFrontDib();
	Size sizDib;
	pbm->GetSize(&sizDib);

	// Create display surface with default palm palette

	struct BitmapInfo // bi
	{
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD bmiColors[256];
	};
	BitmapInfo bi;
	memset(&bi, 0, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = mode.cx;
	bi.bmiHeader.biHeight = -mode.cy;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 8;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biClrUsed = 256;
	gpdisp->GetWindowsPalette(bi.bmiColors);
	byte *pbT;
	HBITMAP hbm = CreateDIBSection(gpdisp->m_hdcMem, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (void **)&pbT, NULL, 0);
	if (hbm == NULL)
		return;
	memcpy(pbT, pbm->GetBits(), sizDib.cx * sizDib.cy);
	HBITMAP hbmSav = (HBITMAP)SelectObject(gpdisp->m_hdcMem, (HGDIOBJ)hbm);

	HDC hdc = gpdisp->m_hdcMem;
	HDC hdcScreen = GetDC(s_hwndDebug);
	Assert(hdcScreen != NULL);
	s_dyLabel = 0;

	if (gsim.GetLevel() != NULL) {
		// Set up a smaller font for displaying info

		HFONT hfntOld = (HFONT)SelectObject(hdc, s_hfntSmall);
		HFONT hfntScreenOld = (HFONT)SelectObject(hdcScreen, s_hfntSmall);
		SetBkMode(hdc, TRANSPARENT);
		SetBkColor(hdcScreen, GetSysColor(COLOR_APPWORKSPACE));
		SetTextColor(hdcScreen, RGB(255, 255, 255));

		//

		WCoord wxView, wyView;
		gsim.GetViewPos(&wxView, &wyView);
		short xView = PcFromUwc(wxView) & 0xfffe;
		short yView = PcFromUwc(wyView) & 0xfffe;
		wxView = WcFromUpc(xView);
		wyView = WcFromUpc(yView);

		::Size siz;
		ggame.GetPlayfieldSize(&siz);

		::Rect rcVisible;
		rcVisible.left = xView;
		rcVisible.top = yView;
		rcVisible.right = xView + siz.cx;
		rcVisible.bottom = yView + siz.cy;

		Gob *apgobVisible[512];
		int cpgobVisible = ggobm.FindGobs(&rcVisible, apgobVisible, ARRAYSIZE(apgobVisible), gsim.GetLevel()->GetFogMap()->GetMapPtr());

		if (s_atgl[kidShowOccupied].f) {
			HBRUSH hbrMunt = CreateSolidBrush(RGB(0, 0, 0));
			HBRUSH hbrStructure = CreateSolidBrush(RGB(0, 255, 255));

			Size sizT;
			ggame.GetPlayfieldSize(&sizT);
			int ctxView = sizT.cx / gcxTile + 1;
			int ctyView = sizT.cy / gcyTile + 1;
			TerrainMap *ptrmap = gsim.GetLevel()->GetTerrainMap();

			TCoord txView = TcFromWc(wxView);
			TCoord tyView = TcFromWc(wyView);
			for (TCoord ty = tyView; ty < tyView + ctyView; ty++) {
				for (TCoord tx = txView; tx < txView + ctxView; tx++) {
					int x = PcFromTc(tx) - xView;
					int y = PcFromTc(ty) - yView;
					if (ptrmap->TestFlags(tx, ty, 1, 1, kbfMobileUnit)) {
						RECT rcT;
						rcT.left = x;
						rcT.top = y;
						rcT.right = x + 2;
						rcT.bottom = y + 2;
						FillRect(hdc, &rcT, hbrMunt);
					}
					if (ptrmap->TestFlags(tx, ty, 1, 1, kbfStructure)) {
						RECT rcT;
						rcT.left = x + 4;
						rcT.top = y;
						rcT.right = x + 6;
						rcT.bottom = y + 2;
						FillRect(hdc, &rcT, hbrStructure);
					}
				}
			}

			DeleteObject(hbrStructure);
			DeleteObject(hbrMunt);
		}

		if (s_atgl[kidShowFiringRange].f) {
			for (int i = 0; i < cpgobVisible; i++) {
				Gob *pgob = apgobVisible[i];
				if ((pgob->GetFlags() & (kfGobUnit | kfGobActive)) != (kfGobUnit | kfGobActive))
					continue;

				UnitGob *punt = (UnitGob *)pgob;

				TCoord tcRange = punt->GetConsts()->tcFiringRange;
				if (tcRange == 0)
					continue;
				if (s_atgl[kidShowSelectedUnitsOnly].f && !(pgob->GetFlags() & kfGobSelected))
					continue;

#ifdef GDIPLUS
				Graphics gr(hdc);
				SolidBrush br(s_aclr[pgob->GetSide()]);
				gr.FillRectangle(&br, rc.left, rc.top, rc.Width(), rc.Height());
#else
				HBRUSH hbr = CreateSolidBrush(s_acr[pgob->GetSide()]);
				HBRUSH hbrOld = (HBRUSH)SelectObject(hdc, hbr);

				TPoint tpt;
				pgob->GetTilePosition(&tpt);
				TRect trc;
				trc.Set(tpt.tx - tcRange, tpt.ty - tcRange, tpt.tx + tcRange + 1, tpt.ty + tcRange + 1);
				
				for (TCoord ty = trc.top; ty < trc.bottom; ty++) {
					for (TCoord tx = trc.left; tx < trc.right; tx++) {
						TCoord dtx = abs(tx - tpt.tx);
						TCoord dty = abs(ty - tpt.ty);
						if (gmpDistFromDxy[dtx][dty] <= tcRange) {
							WRect wrc;
							wrc.left = WcFromTc(tx);
							wrc.right = wrc.left + kwcTile;
							wrc.top = WcFromTc(ty);
							wrc.bottom = wrc.top + kwcTile;
							wrc.Offset(-wxView, -wyView);
							::Rect rc;
							rc.FromWorldRect(&wrc);

							RECT rcT;

							if (gmpDistFromDxy[abs(tx - tpt.tx - 1)][dty] > tcRange) {
								rcT.left = rc.left;
								rcT.top = rc.top;
								rcT.right = rc.left + 2;
								rcT.bottom = rc.bottom;
								FillRect(hdc, &rcT, hbr);
							}
							if (gmpDistFromDxy[abs(tx - tpt.tx + 1)][dty] > tcRange) {
								rcT.left = rc.right - 2;
								rcT.top = rc.top;
								rcT.right = rc.right;
								rcT.bottom = rc.bottom;
								FillRect(hdc, &rcT, hbr);
							}
							if (gmpDistFromDxy[dtx][abs(ty - tpt.ty - 1)] > tcRange) {
								rcT.left = rc.left;
								rcT.top = rc.top;
								rcT.right = rc.right;
								rcT.bottom = rc.top + 2;
								FillRect(hdc, &rcT, hbr);
							}
							if (gmpDistFromDxy[dtx][abs(ty - tpt.ty + 1)] > tcRange) {
								rcT.left = rc.left;
								rcT.top = rc.bottom - 2;
								rcT.right = rc.right;
								rcT.bottom = rc.bottom;
								FillRect(hdc, &rcT, hbr);
							}
						}
					}
				}

				SelectObject(hdc, hbrOld);
				DeleteObject(hbr);
#endif
			}
		}

		if (s_atgl[kidShowHealth].f) {
			for (int i = 0; i < cpgobVisible; i++) {
				Gob *pgob = apgobVisible[i];

				if (!(pgob->GetFlags() & kfGobUnit))
					continue;
				if (s_atgl[kidShowSelectedUnitsOnly].f && !(pgob->GetFlags() & kfGobSelected))
					continue;

				WRect wrc;
				pgob->GetUIBounds(&wrc);
				int x = PcFromWc(wrc.left - wxView);
				int y = PcFromWc(wrc.top - wyView);
				UnitGob *punt = (UnitGob *)pgob;

				fix fxHealth = punt->GetHealth();
				fix fxHealthMax = punt->GetConsts()->GetArmorStrength();
				char szT[30];
				sprintf(szT, "%d/%d", fxtoi(fxHealth), fxtoi(fxHealthMax));
				DrawLabel(hdc, x, y, szT);
			}
			s_dyLabel += kcyLabel;
		}

		if (s_atgl[kidShowState].f) {
			for (int i = 0; i < cpgobVisible; i++) {
				extern char *gaszStateNames[];
				Gob *pgob = apgobVisible[i];

				if (!(pgob->GetFlags() & kfGobUnit))
					continue;
				if (s_atgl[kidShowSelectedUnitsOnly].f && !(pgob->GetFlags() & kfGobSelected))
					continue;

				WRect wrc;
				pgob->GetUIBounds(&wrc);
				int x = PcFromWc(wrc.left - wxView);
				int y = PcFromWc(wrc.top - wyView);

				UnitGob *punt = (UnitGob *)pgob;
				DrawLabel(hdc, x, y, gaszStateNames[punt->m_st]);
			}
			s_dyLabel += kcyLabel;
		}

		if (s_atgl[kidShowAction].f) {
			for (int i = 0; i < cpgobVisible; i++) {
				Gob *pgob = apgobVisible[i];

				if (!(pgob->GetFlags() & kfGobMobileUnit))
					continue;
				if (s_atgl[kidShowSelectedUnitsOnly].f && !(pgob->GetFlags() & kfGobSelected))
					continue;

				WRect wrc;
				pgob->GetUIBounds(&wrc);
				int x = PcFromWc(wrc.left - wxView);
				int y = PcFromWc(wrc.top - wyView);

				MobileUnitGob *pmunt = (MobileUnitGob *)pgob;
				DrawLabel(hdc, x, y, s_aszActions[pmunt->m_mua]);
			}
			s_dyLabel += kcyLabel;
		}

		if (s_atgl[kidShowMuntFlags].f) {
			for (int i = 0; i < cpgobVisible; i++) {
				Gob *pgob = apgobVisible[i];

				if (!(pgob->GetFlags() & kfGobMobileUnit))
					continue;
				if (s_atgl[kidShowSelectedUnitsOnly].f && !(pgob->GetFlags() & kfGobSelected))
					continue;

				MobileUnitGob *pmunt = (MobileUnitGob *)pgob;
				word wfMunt = pmunt->GetMobileUnitFlags();
				char szT[40];
				szT[0] = 0;
				if (wfMunt & kfMuntChaseEnemies)
					strcat(szT, "c,");
				if (wfMunt & kfMuntReturnFire)
					strcat(szT, "rf,");
				if (wfMunt & kfMuntRunAwayWhenHit)
					strcat(szT, "ra,");
				if (wfMunt & kfMuntMoveWait)
					strcat(szT, "mw,");
				if (wfMunt & kfMuntMoveWaitingNearby)
					strcat(szT, "mwn,");
				if (wfMunt & kfMuntPathPending)
					strcat(szT, "pp,");
				if (wfMunt & kfMuntCommandPending)
					strcat(szT, "cp,");
				if (wfMunt & kfMuntAttackEnemiesWhenMoving)
					strcat(szT, "am,");
				if (wfMunt & kfMuntAttackEnemiesWhenGuarding)
					strcat(szT, "ag,");
				if (wfMunt & kfMuntStuck)
					strcat(szT, "stk,");
				if (wfMunt & kfMuntAtReplicatorInput)
					strcat(szT, "rep,");

				// Trim trialing ','

				int cch = strlen(szT);
				if (cch > 0) {
					cch--;
					szT[cch] = 0;
				}

				if (cch > 0) {
					WRect wrc;
					pgob->GetUIBounds(&wrc);
					int x = PcFromWc(wrc.left - wxView);
					int y = PcFromWc(wrc.top - wyView);

					DrawLabel(hdc, x, y, szT);
				}
			}
			s_dyLabel += kcyLabel;
		}

		if (s_atgl[kidShowMuntAggressiveness].f) {
			for (int i = 0; i < cpgobVisible; i++) {
				Gob *pgob = apgobVisible[i];

				if (!(pgob->GetFlags() & kfGobMobileUnit))
					continue;
				if (s_atgl[kidShowSelectedUnitsOnly].f && !(pgob->GetFlags() & kfGobSelected))
					continue;

				MobileUnitGob *pmunt = (MobileUnitGob *)pgob;
				word wfMunt = pmunt->GetMobileUnitFlags() & kfMuntAggressivenessBits;
				char *pszT;
				switch (pmunt->GetMobileUnitFlags() & kfMuntAggressivenessBits) {
				case (kfMuntRunAwayWhenHit):
					pszT = "Coward";
					break;

				case (kfMuntReturnFire | kfMuntStayPut):
					pszT = "SelfDefense";
					break;

				case (kfMuntReturnFire | kfMuntAttackEnemiesWhenGuarding | kfMuntAttackEnemiesWhenMoving):
					pszT = "Defender";
					break;

				case (kfMuntReturnFire | kfMuntAttackEnemiesWhenGuarding | kfMuntAttackEnemiesWhenMoving | kfMuntChaseEnemies):
					pszT = "Pitbull";
					break;

				case (kfMuntReturnFire | kfMuntAttackEnemiesWhenGuarding):
					pszT = "Player";
					break;

				default:
					pszT = "?unknown?";
					break;
				}

				WRect wrc;
				pgob->GetUIBounds(&wrc);
				int x = PcFromWc(wrc.left - wxView);
				int y = PcFromWc(wrc.top - wyView);

				DrawLabel(hdc, x, y, pszT);
			}
			s_dyLabel += kcyLabel;
		}

		if (s_atgl[kidShowUnitGroup].f) {
			for (int i = 0; i < cpgobVisible; i++) {
				Gob *pgob = apgobVisible[i];

				if (!(pgob->GetFlags() & kfGobMobileUnit))
					continue;
				if (s_atgl[kidShowSelectedUnitsOnly].f && !(pgob->GetFlags() & kfGobSelected))
					continue;

				WRect wrc;
				pgob->GetUIBounds(&wrc);
				int x = PcFromWc(wrc.left - wxView);
				int y = PcFromWc(wrc.top - wyView);

				// See if this unit is a member of any of the unit groups

				UnitGroup *pug = gsim.GetLevel()->GetUnitGroupMgr()->GetUnitGroup(pgob->GetId());
				if (pug != NULL) {
					char *pszName = pug->GetName();
					if (pszName != NULL)
						DrawLabel(hdc, x, y, pszName);
				}
			}
			s_dyLabel += kcyLabel;
		}

		if (s_atgl[kidShowGobPointer].f) {
			for (int i = 0; i < cpgobVisible; i++) {
				Gob *pgob = apgobVisible[i];

				if (s_atgl[kidShowSelectedUnitsOnly].f && !(pgob->GetFlags() & kfGobSelected))
					continue;

				WRect wrc;
				pgob->GetUIBounds(&wrc);
				int x = PcFromWc(wrc.left - wxView);
				int y = PcFromWc(wrc.top - wyView);

				char szT[30];
				sprintf(szT, "%lx", pgob);
				DrawLabel(hdc, x, y, szT);
			}
			s_dyLabel += kcyLabel;
		}

		if (s_atgl[kidShowCoordinates].f) {
			for (int i = 0; i < cpgobVisible; i++) {
				Gob *pgob = apgobVisible[i];

				if (s_atgl[kidShowSelectedUnitsOnly].f && !(pgob->GetFlags() & kfGobSelected))
					continue;

				WPoint wpt;
				pgob->GetPosition(&wpt);
				char szT[30];
				sprintf(szT, "$%04lx,$%04lx", wpt.wx, wpt.wy);

				WRect wrc;
				pgob->GetUIBounds(&wrc);
				int x = PcFromWc(wrc.left - wxView);
				int y = PcFromWc(wrc.top - wyView);
				DrawLabel(hdc, x, y, szT);
			}
			s_dyLabel += kcyLabel;
		}

		if (s_atgl[kidShowDst].f) {
			for (int i = 0; i < cpgobVisible; i++) {
				Gob *pgob = apgobVisible[i];

				if (s_atgl[kidShowSelectedUnitsOnly].f && !(pgob->GetFlags() & kfGobSelected))
					continue;

				if (!(pgob->GetFlags() & kfGobMobileUnit))
					continue;
				MobileUnitGob *pmunt = (MobileUnitGob *)pgob;

				char szT[30];
				sprintf(szT, "$%02lx,$%02lx", pmunt->m_txDst, pmunt->m_tyDst);

				WRect wrc;
				pgob->GetUIBounds(&wrc);
				int x = PcFromWc(wrc.left - wxView);
				int y = PcFromWc(wrc.top - wyView);
				DrawLabel(hdc, x, y, szT);
			}
			s_dyLabel += kcyLabel;
		}

		if (s_atgl[kidShowUpdaters].f) {
			for (int i = 0; i < cpgobVisible; i++) {
				Gob *pgob = apgobVisible[i];

				if (s_atgl[kidShowSelectedUnitsOnly].f && !(pgob->GetFlags() & kfGobSelected))
					continue;

				if (!(pgob->GetFlags() & kfGobStateMachine))
					continue;

				if (pgob->m_unvl.m_cDecrements != 0)
					continue;

				Rect rcBounds;
				pgob->GetClippingBounds(&rcBounds);
				rcBounds.Offset(-PcFromUwc(wxView), -PcFromUwc(wyView));
				RECT rc;
				rc.left = rcBounds.left;
				rc.top = rcBounds.top;
				rc.right = rcBounds.right;
				rc.bottom = rcBounds.bottom;
				HBRUSH hbrWhite = (HBRUSH)GetStockObject(WHITE_BRUSH);

				RECT rcT;
				rcT = rc;
				rcT.bottom = rcT.top + 2;
				FillRect(hdc, &rcT, hbrWhite);

				rcT = rc;
				rcT.right = rc.left + 2;
				FillRect(hdc, &rcT, hbrWhite);

				rcT = rc;
				rcT.left = rcT.right - 2;
				FillRect(hdc, &rcT, hbrWhite);

				rcT = rc;
				rcT.top = rcT.bottom - 2;
				FillRect(hdc, &rcT, hbrWhite);
			}
		}

#if 0
		// UNDONE: not acted on anywhere yet

		bool fDebugUnit = s_atgl[kidDebugSelectedUnit].f;
		for (Gob *pgob = ggobm.GetFirstGob(); pgob != NULL; pgob = ggobm.GetNextGob(pgob)) {
			dword ff = pgob->GetFlags();
			if (fDebugUnit && (ff & kfGobSelected))
				ff |= kfGobDebug;
			else
				ff &= ~kfGobDebug;
			pgob->SetFlags(ff);
		}
#endif

		// Causes the StateMachine to break on each received message, excluding kmidReserved* (e.g., Update)

		bool fDebugSM = s_atgl[kidDebugSelectedStateMachine].f;
		for (Gob *pgob = ggobm.GetFirstGob(); pgob != NULL; pgob = ggobm.GetNextGob(pgob)) {
			if (fDebugSM && (pgob->GetFlags() & kfGobSelected))
				pgob->EnableDebug(true);
			else
				pgob->EnableDebug(false);
		}

#define Print(sz) TextOut(hdcScreen, x, y += kcySmallFont, sz, strlen(sz))

		int x = 322;
		int y = kcyButton * ARRAYSIZE(s_atgl);

		char szT[200];
		sprintf(szT, "Update: %ld, Time: %ld sec, Gobs: %d            ", 
				gsim.GetUpdateCount(), gsim.GetTickCount() / 100, ggobm.GetGobCount());
		Print(szT);
		Print("");

		Player *pplr = NULL;	
		while (true) {
			pplr = gplrm.GetNextPlayer(pplr);
			if (pplr == NULL)
				break;
			sprintf(szT, "Side %d", pplr->GetSide());
			Print(szT);
			sprintf(szT, "   credits/capacity: %d/%d            ", pplr->GetCredits(), pplr->GetCapacity());
			Print(szT);
			sprintf(szT, "   power demand/supply: %d/%d            ", pplr->GetPowerDemand(), pplr->GetPowerSupply());
			Print(szT);
		}

		SelectObject(hdc, hfntOld);
		SelectObject(hdcScreen, hfntScreenOld);

		{
			void UpdateLog();
			UpdateLog();
		}
	}

	// Draw to screen, default scale

	int cxMap = mode.cx * mode.nScale;
	int cyMap = mode.cy * mode.nScale;
	StretchBlt(hdcScreen, 0, 0, cxMap, cyMap, gpdisp->m_hdcMem, 0, 0, mode.cx, mode.cy, SRCCOPY);

	HBITMAP hbmDelete = (HBITMAP)SelectObject(gpdisp->m_hdcMem, (HGDIOBJ)hbmSav);
	DeleteObject((HGDIOBJ)hbmDelete);

	ReleaseDC(s_hwndDebug, hdcScreen);

}

void DrawLabel(HDC hdc, int x, int y, char *psz)
{
	int cch = strlen(psz);
	SetTextColor(hdc, RGB(0, 0, 0));
	TextOut(hdc, x + 1, y + 1 + s_dyLabel, psz, cch);
	SetTextColor(hdc, RGB(255, 255, 255));
	TextOut(hdc, x, y + s_dyLabel, psz, cch);
}

//
// Log
//

const int kcleMax = 5000;

const word kfLeString = 0x0001;
const word kfLeMessage = 0x0002;
const word kfLeStateChange = 0x0004;

struct LogEntry { // le
	LogEntry *pleNext;
	LogEntry *plePrev;
	word wf;
	long cUpdate;
	StateMachineId smid;
	Message msg;
	State stOld;
	State stNew;
	char sz[200];
};

// Newest entries go to the head of the list

static LogEntry *s_pleHead, *s_pleTail;
static int s_cle;

void AddLogEntry(LogEntry *ple);

//
extern char *gaszMessageNames[];
extern char *gaszStateNames[];

void Log(StateMachine *psm, Message *pmsg)
{
	LogEntry *ple = new LogEntry;
	memset(ple, 0, sizeof(LogEntry));
	ple->wf = kfLeMessage;
	ple->cUpdate = gsim.GetUpdateCount();
	ple->smid = gsmm.GetId(psm);
	ple->msg = *pmsg;

	char szT[100];
	if (pmsg->smidSender == ksmidNull)
		strcpy(szT, "null");
	else if (psm == gsmm.GetStateMachine(pmsg->smidSender))
		strcpy(szT, "self");
	else {
		StateMachine *psm = gsmm.GetStateMachine(pmsg->smidSender);
		char *pszName = psm == NULL ? "GONE" : psm->GetName();
		sprintf(szT, "%s.%lx.%d", pszName, gsmm.GetStateMachine(pmsg->smidSender), pmsg->smidSender);
	}
	char szT2[100];
	if (pmsg->smidSender == ksmidNull)
		szT2[0] = 0;
	else
		sprintf(szT2, " (from %s)", szT);
	sprintf(ple->sz, "M u: %ld, %s.%lx.%d recv %s/%s%s", 
			ple->cUpdate, psm->GetName(), psm, ple->smid, gaszStateNames[psm->m_st], 
			gaszMessageNames[pmsg->mid], szT2);
	AddLogEntry(ple);
}

void Log(StateMachine *psm, State stOld, State stNew)
{
	LogEntry *ple = new LogEntry;
	memset(ple, 0, sizeof(LogEntry));
	ple->wf = kfLeStateChange;
	ple->cUpdate = gsim.GetUpdateCount();
	ple->smid = gsmm.GetId(psm);
	ple->stOld = stOld;
	ple->stNew = stNew;

	sprintf(ple->sz, "S u: %ld, %s.%lx.%d set to %s (prev %s)", 
			ple->cUpdate, psm->GetName(), psm, ple->smid,
			gaszStateNames[stNew], gaszStateNames[stOld]);
	AddLogEntry(ple);
}

void Log(char *psz)
{
	LogEntry *ple = new LogEntry;
	memset(ple, 0, sizeof(LogEntry));
	ple->wf = kfLeString;

	AddLogEntry(ple);
}

void AddLogEntry(LogEntry *ple)
{
	ple->pleNext = s_pleHead;
	if (s_pleHead != NULL)
		s_pleHead->plePrev = ple;
	else
		s_pleTail = ple;
	s_pleHead = ple;

	if (s_cle >= kcleMax) {
		LogEntry *pleDel = s_pleTail;
		s_pleTail->plePrev->pleNext = NULL;
		s_pleTail = s_pleTail->plePrev;
		delete pleDel;
	} else {
		s_cle++;
	}
}

void InitLog()
{
	s_pleHead = NULL;
}

void ClearLog()
{
	LogEntry *ple = s_pleHead;
	while (ple != NULL) {
		LogEntry *pleNext = ple->pleNext;
		delete ple;
		ple = pleNext;
	}
	s_pleHead = NULL;
	s_pleTail = NULL;
	s_cle = 0;
}

// For immediate window

void OutputLog(int c)
{
	for (LogEntry *ple = s_pleHead; ple != NULL; ple = ple->pleNext) {
		if (c-- <= 0)
			break;
		OutputDebugString(ple->sz);
		OutputDebugString("\n");
	}
}

void ExitLog()
{
	ClearLog();
}

void UpdateLog()
{
	if (!s_atgl[kidShowStateMachineLog].f)
		return;

	HDC hdc = GetDC(s_hwndDebug);
	HFONT hfntOld = (HFONT)SelectObject(hdc, s_hfntSmall);

	DrawLog(hdc);

	SelectObject(hdc, hfntOld);
	ReleaseDC(s_hwndDebug, hdc);
}

void DrawLog(HDC hdc)
{
	int x = kcxDebugWindow + 2;
	int y = kcyDebugWindow - kcySmallFont;

	for (LogEntry *ple = s_pleHead; ple != NULL && y >= 0; ple = ple->pleNext) {

		// Filter unselected Gobs if we're in that mode

		if (s_atgl[kidShowSelectedUnitsOnly].f) {
			Gob *pgob = ggobm.GetGob(ple->smid);
			if (pgob == NULL || !(pgob->GetFlags() & kfGobSelected))
				continue;
		}

		RECT rc;
		rc.left = x;
		rc.top = y;
		rc.right = x + kcxLogWindow;
		rc.bottom = y + kcySmallFont;
		ExtTextOut(hdc, x, y, ETO_OPAQUE, &rc, ple->sz, strlen(ple->sz), NULL);
		y -= kcySmallFont;
	}

	// Clear any unused space above the displayed log entries

	if (y > 0) {
		RECT rc;
		rc.left = kcxDebugWindow;
		rc.top = 0;
		rc.right = rc.left + kcxLogWindow;
		rc.bottom = y + kcySmallFont;

		HBRUSH hbr = CreateSolidBrush(GetBkColor(hdc));
		FillRect(hdc, &rc, hbr);
		DeleteObject(hbr);
	}
}

//
// DebugViewer implementation
//

DebugViewer::DebugViewer(char *pszTitle, int x, int y, int cx, int cy)
{
	LRESULT CALLBACK DebugViewerWndProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp);
	static char s_szClass[] = "HostileTakeoverDebugViewer";
	static bool s_fRegistered = false;

	if (!s_fRegistered) {
		WNDCLASS wc;
		memset(&wc, 0, sizeof(wc));
		wc.lpfnWndProc = DebugViewerWndProc;
		wc.hInstance = ghInst;
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
		wc.lpszClassName = s_szClass;
 		RegisterClass(&wc);
		s_fRegistered = true;
	}

	POINT ptT = { 0, 0 };
	HMONITOR hmon = MonitorFromPoint(ptT, MONITOR_DEFAULTTOPRIMARY);

	MONITORINFOEX mi;
	mi.cbSize = sizeof(mi);
	GetMonitorInfo(hmon, &mi);

	// Right-align for negative x's

	x = x < 0 ? mi.rcWork.right + x : mi.rcWork.left + x;

	dword dwStyle = WS_OVERLAPPED | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	m_hwnd = CreateWindowEx(0, s_szClass, pszTitle,
			dwStyle, x, y, cx, cy, NULL, NULL, ghInst, 0);
	SetWindowLong(m_hwnd, GWL_USERDATA, (LONG)this);

	m_xView = m_yView = 0;
}

DebugViewer::~DebugViewer()
{
	DestroyWindow(m_hwnd);
}

void DebugViewer::OnRightClick(int x, int y)
{
}

LRESULT CALLBACK DebugViewerWndProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp)
{
	static bool s_fMouseDown;
	static int s_xMouseDown;
	static int s_yMouseDown;

	DebugViewer *pdbgv = (DebugViewer *)GetWindowLong(hwnd, GWL_USERDATA);

//	extern void DebugHelperCommandHandler(WPARAM wp, LPARAM lp);

	switch (wm) {
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			pdbgv->Paint(hdc);
			EndPaint(hwnd, &ps);
		}
		break;

//	case WM_COMMAND:
//		DebugHelperCommandHandler(wp, lp);
//		break;

	case WM_LBUTTONDOWN:
		s_fMouseDown = true;
		s_xMouseDown = GET_X_LPARAM(lp);
		s_yMouseDown = GET_Y_LPARAM(lp);
		SetCapture(hwnd);
		break;

	case WM_MOUSEMOVE:
		if (s_fMouseDown) {
			int dx = s_xMouseDown - GET_X_LPARAM(lp);
			int dy = s_yMouseDown - GET_Y_LPARAM(lp);
			pdbgv->ScrollTo(dx, dy);
			s_xMouseDown = GET_X_LPARAM(lp);
			s_yMouseDown = GET_Y_LPARAM(lp);
		}
		break;

	case WM_LBUTTONUP:
		s_fMouseDown = false;
		ReleaseCapture();
		break;

	case WM_RBUTTONDOWN:
		pdbgv->OnRightClick(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
		break;

	case WM_CLOSE:
		ShowWindow(hwnd, false);
		return 1;

	case WM_ERASEBKGND:
		if (gsim.GetLevel() != NULL)
			return 1;
		break;
	}
	return DefWindowProc(hwnd, wm, wp, lp);
}

void DebugViewer::ToggleVisibility()
{
	ShowWindow(m_hwnd, !IsWindowVisible(m_hwnd));
}

void DebugViewer::Update()
{
	InvalidateRect(m_hwnd, NULL, false);
}

void DebugViewer::ScrollTo(int dx, int dy)
{
	m_xView += dx;
	if (m_xView < 0)
		m_xView = 0;
	m_yView += dy;
	if (m_yView < 0)
		m_yView = 0;
	Update();
}

void DebugViewer::Paint(HDC hdc)
{
}

//
// Trigger Viewer implementation
//

const int kcxTriggerWindow = 650;
const int kcyTriggerWindow = 700;

TriggerViewer::TriggerViewer() : 
		DebugViewer("HT Triggers", 330, 0, kcxTriggerWindow, kcyTriggerWindow)
{
}

void UpdateTriggerViewer()
{
	s_ptgrv->Update();
}

void TriggerViewer::Paint(HDC hdc)
{
	Level *plvl = gsim.GetLevel();
	if (plvl == NULL)
		return;

	HFONT hfntOld = (HFONT)SelectObject(hdc, s_hfntSmall);
	HBRUSH hbr = (HBRUSH)GetStockObject(GRAY_BRUSH);

	int x = -m_xView;
	int y = -m_yView;
	RECT rc;

	// Display per-side triggers in the per-side specified order 

	TriggerMgr *ptgrm = plvl->GetTriggerMgr();
	for (Side side = ksideNeutral; side < kcSides; side++) {
		byte *pntgr = &ptgrm->m_mpSide2nTrigger[side][0];
		for (; *pntgr != 0xff; pntgr++) {
			Trigger *ptgr = &ptgrm->m_atgr[*pntgr];
			TriggerAction *pactn = ptgr->m_apactnLast[side];

			for (Condition *pcdn = ptgr->m_pcdn; pcdn != NULL; pcdn = pcdn->m_pcdnNext) {
				char szT[500];
				sprintf(szT, "%s", pcdn->ToString());
				Assert(strlen(szT) < sizeof(szT));

				COLORREF clr;
				// Armed? (ready to be triggered)

				if (ptgr->m_afArmed[side]) {

					// Yes. Condition satisfied?

					if (pcdn->SafeIsTrue(side))
						clr = RGB(164, 0, 0); // yes
					else
						clr = RGB(0, 0, 0);
				} else {

					// Disarmed (already triggered)
					clr = RGB(255, 0, 0);
				}

				// Action in progress

				if (pactn != NULL)
					clr = RGB(0, 255, 0);

				SetTextColor(hdc, clr);

				rc.left = x;
				rc.top = y;
				rc.right = kcxTriggerWindow;
				rc.bottom = y + kcySmallFont;
				ExtTextOut(hdc, x + 2, y, ETO_OPAQUE, &rc, szT, strlen(szT), NULL);

				y += kcySmallFont;
			}

			if (pactn != NULL) {
				char szT[200];
				sprintf(szT, "> %s", pactn->ToString());
				Assert(strlen(szT) < sizeof(szT));
				SetTextColor(hdc, RGB(0, 164, 0));
				
				rc.top = y;
				rc.bottom = y + kcySmallFont;
				ExtTextOut(hdc, x + 2, y, ETO_OPAQUE, &rc, szT, strlen(szT), NULL);
				y += kcySmallFont;
			}

			rc.left = 0;
			rc.top = y;
			rc.right = kcxTriggerWindow;
			rc.bottom = y + 2;
			FillRect(hdc, &rc, hbr);

			y += 2;
		}
	}

	if (y < kcyTriggerWindow) {
		rc.bottom = kcyTriggerWindow;
		FillRect(hdc, &rc, hbr);
	}

	SelectObject(hdc, hfntOld);
}

void TriggerViewer::OnRightClick(int xClick, int yClick)
{
	Level *plvl = gsim.GetLevel();
	if (plvl == NULL)
		return;

	int x = -m_xView;
	int y = -m_yView;

	// Display per-side triggers in the per-side specified order 

	TriggerMgr *ptgrm = plvl->GetTriggerMgr();
	for (Side side = ksideNeutral; side < kcSides; side++) {
		byte *pntgr = &ptgrm->m_mpSide2nTrigger[side][0];
		for (; *pntgr != 0xff; pntgr++) {
			Trigger *ptgr = &ptgrm->m_atgr[*pntgr];
			TriggerAction *pactn = ptgr->m_apactnLast[side];

			for (Condition *pcdn = ptgr->m_pcdn; pcdn != NULL; pcdn = pcdn->m_pcdnNext)
				y += kcySmallFont;

			if (pactn != NULL)
				y += kcySmallFont;

			if (yClick <= y) {
				ptgr->Execute(side, true);
				return;
			}

			y += 2;
		}
	}
}

//
// Unit Group Viewer implementation
//

const int kcxUnitGroupWindow = 300;
const int kcyUnitGroupWindow = 900;

UnitGroupViewer::UnitGroupViewer() : 
		DebugViewer("HT Unit Groups", -kcxUnitGroupWindow, 0, kcxUnitGroupWindow, kcyUnitGroupWindow)
{
}

void UpdateUnitGroupViewer()
{
	s_pugv->Update();
}

void UnitGroupViewer::Paint(HDC hdc)
{
	Level *plvl = gsim.GetLevel();
	if (plvl == NULL)
		return;

	HFONT hfntOld = (HFONT)SelectObject(hdc, s_hfntSmall);
	HBRUSH hbr = (HBRUSH)GetStockObject(GRAY_BRUSH);

	int x = -m_xView;
	int y = -m_yView;
	RECT rc;
	rc.left = x;
	rc.right = kcxUnitGroupWindow;

	UnitGroupMgr *pugm = plvl->GetUnitGroupMgr();
	int cug = pugm->m_cug;
	for (int i = 0; i < cug; i++) {
		char szT[500];
		UnitGroup *pug = &pugm->m_aug[i];

		COLORREF clr;

		word wf = pug->GetFlags();
		if (wf & kfUgActive) {
			if (wf & kfUgNeedsUnit)
				clr = RGB(0, 164, 0);
			else
				clr = RGB(0, 255, 0);
		} else {
			if (wf & kfUgActivatedBefore)
				clr = RGB(164, 0, 0);
			else
				clr = RGB(0, 0, 0);
		}

		SetTextColor(hdc, clr);

		char szT2[500];
		szT2[0] = 0;

		if (wf & kfUgRandomGroup) {
			if (szT2[0] != 0)
				strcat(szT2, ", ");
			strcat(szT2, "random");
		}

		if (wf & kfUgCreateAtLevelLoad) {
			if (szT2[0] != 0)
				strcat(szT2, ", ");
			strcat(szT2, "level load");
		}

		if (wf & kfUgReplaceGroup) {
			if (szT2[0] != 0)
				strcat(szT2, ", ");
			strcat(szT2, "replace");
		}

		if (wf & kfUgSpawn) {
			if (szT2[0] != 0)
				strcat(szT2, ", ");
			strcat(szT2, "spawn");
		}

		if (wf & kfUgLoopForever) {
			if (szT2[0] != 0)
				strcat(szT2, ", ");
			strcat(szT2, "loop");
		}

		if (!(wf & kfUgNotRecentlyActivated)) {
			if (szT2[0] != 0)
				strcat(szT2, ", ");
			strcat(szT2, "RA");
		}

		sprintf(szT, "%s  (%s)", pug->GetName(), szT2);

		rc.top = y;
		rc.bottom = y + kcySmallFont;
		ExtTextOut(hdc, x + 2, y, ETO_OPAQUE, &rc, szT, strlen(szT), NULL);
		y += kcySmallFont;

		if (pug->GetFlags() & kfUgActive) {
			// Show all the group's members

			int cule = pug->GetUnitCount();
			UnitListEntry *ple = pug->GetUnitList();
			for (int iule = 0; iule < cule; iule++, ple++) {
				SetTextColor(hdc, clr);

				rc.left = x;
				rc.top = y;
				rc.right = kcxUnitGroupWindow;
				rc.bottom = y + kcySmallFont;
				if (!(ple->bf & kfUleBuilt)) {
					sprintf(szT, "%s (building...)", gapuntc[ple->ut]->szName);
				} else {
					MobileUnitGob *pmunt = (MobileUnitGob *)ggobm.GetGob(ple->gid);
					char *pszT;
					if (pmunt == NULL) {
						SetTextColor(hdc, RGB(255, 0, 0));
						pszT = "dead";
					} else {
						if (pmunt->GetFlags() & kfGobMobileUnit)
							pszT = s_aszActions[pmunt->m_mua];
						else
							pszT = "no action";
					}
					sprintf(szT, "%s (%s)", gapuntc[ple->ut]->szName, pszT);
				}
				ExtTextOut(hdc, x + 10, y, ETO_OPAQUE, &rc, szT, strlen(szT), NULL);
				y += kcySmallFont;
			}

			// Show the currently executing action (if any)

			if (pug->m_pactnLast != NULL) {
				SetTextColor(hdc, RGB(0, 164, 0));
				rc.top = y;
				rc.bottom = y + kcySmallFont;
				sprintf(szT, "> %s", pug->m_pactnLast->ToString());
				ExtTextOut(hdc, x + 2, y, ETO_OPAQUE, &rc, szT, strlen(szT), NULL);
				y += kcySmallFont;
			}
		}

		rc.top = y;
		rc.bottom = y + 2;
		FillRect(hdc, &rc, hbr);

		y += 2;
	}

	if (y < kcyUnitGroupWindow) {
		rc.bottom = kcyUnitGroupWindow;
		FillRect(hdc, &rc, hbr);
	}

	SelectObject(hdc, hfntOld);
}

//
// DelayedMessage Viewer implementation
//

const int kcxDelayedMessageWindow = 300;
const int kcyDelayedMessageWindow = 900;

DelayedMessageViewer::DelayedMessageViewer() : 
		DebugViewer("HT Delayed Messages", -kcxDelayedMessageWindow, 0, kcxDelayedMessageWindow, kcyDelayedMessageWindow)
{
}

void UpdateDelayedMessageViewer()
{
	s_pdmv->Update();
}

void DelayedMessageViewer::Update()
{
	int cdmsg = 0;
	for (DelayedMessage *pdmsg = gsmm.m_pdmsgHead; pdmsg != NULL; pdmsg = pdmsg->pdmsgNext, cdmsg++);

	char szT[50];
	sprintf(szT, "HT Delayed Messages [%d]", cdmsg);
	SetWindowText(m_hwnd, szT);
	DebugViewer::Update();
}

void DelayedMessageViewer::Paint(HDC hdc)
{
	Level *plvl = gsim.GetLevel();
	if (plvl == NULL)
		return;

	HFONT hfntOld = (HFONT)SelectObject(hdc, s_hfntSmall);
	HBRUSH hbr = (HBRUSH)GetStockObject(GRAY_BRUSH);

	int x = -m_xView;
	int y = -m_yView;
	RECT rc;
	GetClientRect(m_hwnd, &rc);
	rc.left = x;

	for (DelayedMessage *pdmsg = gsmm.m_pdmsgHead; pdmsg != NULL; pdmsg = pdmsg->pdmsgNext) {
		StateMachine *psmSender = gsmm.GetStateMachine(pdmsg->msg.smidSender);
		StateMachine *psmReceiver = gsmm.GetStateMachine(pdmsg->msg.smidReceiver);
		char *pszSender = psmSender != NULL ? psmSender->GetName() : "<none>";
		char *pszReceiver = psmReceiver != NULL ? psmReceiver->GetName() : "<none>";
		
		char szT[500];
		sprintf(szT, "%s delayed [%d] from %s.%d to %s.%d", gaszMessageNames[pdmsg->msg.mid], 
				pdmsg->msg.tDelivery - gsim.GetTickCount(), 
				pszSender, pdmsg->msg.smidSender, pszReceiver, pdmsg->msg.smidReceiver);

		rc.top = y;
		rc.bottom = y + kcySmallFont;
		ExtTextOut(hdc, x + 2, y, ETO_OPAQUE, &rc, szT, strlen(szT), NULL);
		y += kcySmallFont;
	}

	if (y < kcyDelayedMessageWindow) {
		rc.bottom = kcyDelayedMessageWindow;
		FillRect(hdc, &rc, hbr);
	}

	SelectObject(hdc, hfntOld);
}

//
// CommandQueue Viewer implementation
//

const int kcxCommandQueueWindow = 300;
const int kcyCommandQueueWindow = 900;

CommandQueueViewer::CommandQueueViewer() : 
		DebugViewer("HT Command Queue", -kcxCommandQueueWindow, 0, kcxCommandQueueWindow, kcyCommandQueueWindow)
{
}

void UpdateCommandQueueViewer()
{
	s_pcmdqv->Update();
}

void CommandQueueViewer::Update()
{
	char szT[50];
	sprintf(szT, "HT Command Queue [%d]", gcmdq.GetCount());
	SetWindowText(m_hwnd, szT);
	DebugViewer::Update();
}

void CommandQueueViewer::Paint(HDC hdc)
{
	Level *plvl = gsim.GetLevel();
	if (plvl == NULL)
		return;

	HFONT hfntOld = (HFONT)SelectObject(hdc, s_hfntSmall);
	HBRUSH hbr = (HBRUSH)GetStockObject(GRAY_BRUSH);

	int x = -m_xView;
	int y = -m_yView;
	RECT rc;
	GetClientRect(m_hwnd, &rc);
	rc.left = x;

	Message *pmsg = gcmdq.GetFirst();
	int cmsg = gcmdq.GetCount();
	for (int i = 0; i < cmsg; i++, pmsg++) {
		StateMachine *psmSender = gsmm.GetStateMachine(pmsg->smidSender);
		StateMachine *psmReceiver = gsmm.GetStateMachine(pmsg->smidReceiver);
		char *pszSender = psmSender != NULL ? psmSender->GetName() : "<none>";
		char *pszReceiver = psmReceiver != NULL ? psmReceiver->GetName() : "<none>";
		
		char szT[500];
		sprintf(szT, "%s from %s.%d to %s.%d", gaszMessageNames[pmsg->mid], 
				pszSender, pmsg->smidSender, pszReceiver, pmsg->smidReceiver);

		rc.top = y;
		rc.bottom = y + kcySmallFont;
		ExtTextOut(hdc, x + 2, y, ETO_OPAQUE, &rc, szT, strlen(szT), NULL);
		y += kcySmallFont;
	}

	if (y < kcyCommandQueueWindow) {
		rc.bottom = kcyCommandQueueWindow;
		FillRect(hdc, &rc, hbr);
	}

	SelectObject(hdc, hfntOld);
}

// Helpers

static char s_szUnitMask[500];

char *PszFromUnitMask(UnitMask um)
{
	char *psz = s_szUnitMask;
	*psz = 0;
	if ((um & kumInfantry) == kumInfantry) {
		strcat(psz, "any infantry,");
		um &= ~kumInfantry;
	}

	if ((um & kumVehicles) == kumVehicles) {
		strcat(psz, "any vehicle,");
		um &= ~kumVehicles;
	}

	if ((um & kumStructures) == kumStructures) {
		strcat(psz, "any structure,");
		um &= ~kumStructures;
	}

	for (int i = 0; i < kutMax; i++) {
		if (um & (1UL << i)) {
			strcat(psz, gapuntc[i]->szName);
			strcat(psz, ",");
		}
	}

	// Trim trialing ','

	int cch = strlen(psz);
	if (cch > 0)
		psz[cch - 1] = 0;

	return psz;
}

static char s_szCaSideMask[500];

char *PszFromCaSideMask(word wfCaSideMask)
{
	char *psz = s_szCaSideMask;
	*psz = 0;
	if (wfCaSideMask & (1 << knCaSideAllSides))
		strcat(psz, "all sides,");
	if (wfCaSideMask & (1 << knCaSideAllies))
		strcat(psz, "allies,");
	if (wfCaSideMask & (1 << knCaSideEnemies))
		strcat(psz, "enemies,");
	if (wfCaSideMask & (1 << knCaSideCurrentSide))
		strcat(psz, "current side,");
	if (wfCaSideMask & (1 << knCaSideSide1))
		strcat(psz, "side 1,");
	if (wfCaSideMask & (1 << knCaSideSide2))
		strcat(psz, "side 2,");
	if (wfCaSideMask & (1 << knCaSideSide3))
		strcat(psz, "side 3,");
	if (wfCaSideMask & (1 << knCaSideSide4))
		strcat(psz, "side 4,");

	// Trim trialing ','

	int cch = strlen(psz);
	if (cch > 0)
		psz[cch - 1] = 0;

	return psz;
}

#endif // def DEBUG_HELPERS
