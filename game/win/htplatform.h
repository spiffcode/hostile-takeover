
#ifdef CHECK_OLD_ALLOCS

void my_delete(void *pv);
void *my_new(int cb);
void my_alloccheck();
void my_freeall();

inline int _CrtCheckMemory(void)
{
	my_alloccheck();
	return 1;
}
inline int _CrtDumpMemoryLeaks(void)
{
	my_freeall();
	return 0;
}

inline void __cdecl operator delete(void * _P)
        { my_delete(_P); }

inline void * __cdecl operator new(size_t s)
        { return my_new(s); }
#else
#ifdef DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#else
#include <malloc.h>
#endif
#endif

// htplatform.h

#include <stdlib.h>
#include <winsock.h>
//#include <windows.h>
#include <mmsystem.h>
#include <memory.h>
#include <stdarg.h>
#include <string.h>

#define MakeDword(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

// Can't just include <stdio.h> because that hoses users of PackFile.h
extern "C" {
_CRTIMP int __cdecl sprintf(char *, const char *, ...);
_CRTIMP int __cdecl vsprintf(char *, const char *, va_list);
}

#define kszWindowTitle "Hostile Takeover"
#define kszWindowClass "HostileTakeover"
#define kidiMain 1	// icon id

#define BigWord(x) ((((x)&0xFF)<<8) | (((x)&0xFF00)>>8))
#define BigDword(x) ((((x)&0xFF)<<24) | (((x)&0xFF00)<<8) | (((x)&0xFF0000)>>8) | (((x)&0xFF000000)>>24))

#define WM_GAMEEVENT (WM_USER+0)

#define chrPageUp 0xb
#define chrPageDown 0xc
#define vchrMenu 0x105
#define vchrFind 0x10a
#define vchrCalc 0x10b
#define vchrHard4 0x0207				// memo button
#define penDownEvent 1
#define penUpEvent 2
#define penMoveEvent 3
#define keyDownEvent 4
#define appStopEvent 22
#define chrLeft 0x1000
#define chrRight 0x1001
#define chrBackspace 0x0008
#define chrDelete 0x007f
#define chrUp chrPageUp
#define chrDown chrPageDown
#define chrDpad 0x0503
#define	keyBitPower			0x0001		// Power key
#define	keyBitPageUp		0x0002		// Page-up, aka Tungsten d-pad up
#define	keyBitPageDown		0x0004		// Page-down, aka Tungsten d-pad down
#define	keyBitHard1			0x0008		// App #1
#define	keyBitHard2			0x0010		// App #2
#define	keyBitHard3			0x0020		// App #3
#define	keyBitHard4			0x0040		// App #4
#define	keyBitCradle		0x0080		// Button on cradle
#define	keyBitAntenna		0x0100		// Antenna "key" <chg 3-31-98 RM>
#define	keyBitContrast		0x0200		// Contrast key

// Palm SG's bits

#define keyBitDpadLeft		0x01000000	// Tungsten d-pad left
#define keyBitDpadRight		0x02000000	// Tungsten d-pad right
#define keyBitDpadButton	0x04000000	// Tungsten d-pad center button

// Official PalmSource bits

#define keyBitRockerUp          0x00010000      // 5-way rocker
#define keyBitRockerDown        0x00020000
#define keyBitRockerLeft        0x00040000
#define keyBitRockerRight       0x00080000
#define keyBitRockerCenter      0x00100000

#define	keyBitsAll			0xFFFFFFFF	// all keys

// shutdown() is in winsock.h but these defines aren't

#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02

struct ModeInfo
{
	int cx;
	int cy;
	int nDepth;
	bool fNative;
	int cyGraffiti;
	int nScale;
	int nDegreeOrientation;
};
#define kcmodesMax 16

struct Palette;
class Rect;
class DibBitmap;
class UpdateMap;
class Display // disp
{
public:
	Display();
	~Display();

	bool Init();
	void SetPalette(Palette *ppal);
	int GetModeCount();
	void GetModeInfo(int imode, ModeInfo *pmode);
	int GetMode(ModeInfo *pmode);
	bool SetMode(int imode, int nScale);
	void DrawText(const char *psz, int x, int y, word wf);
	void DrawFrameInclusive(Rect *prc);
	void GetHslAdjustments(short *pnHueOffset, short *pnSatMultiplier, short *pnLumOffset);
	void GetWindowsPalette(RGBQUAD *pargbq);
	DibBitmap *GetBackDib();
	DibBitmap *GetFrontDib();
	DibBitmap *GetClippingDib();
	void FrameStart();
	void FrameComplete();
	void SetScale(int nScale);
	int GetScale() {
		return m_nScale;
	}

	static HMENU m_hmenuPopup;
	static HWND m_hwnd;

#ifndef DEBUG_HELPERS
private:
#endif
	HDC m_hdcMem;

private:
	void ResizeWindow(int nScale);

	int m_nScale;
	HBITMAP m_hbm;
	HBITMAP m_hbmSav;
	HDC m_hdc;
	byte *m_pbScreen;
	DibBitmap *m_pbmFront;
	DibBitmap *m_pbmBack;
	DibBitmap *m_pbmClip;
	ModeInfo m_amodeInfo[kcmodesMax];
	int m_cmodes;
	int m_imode;
};
extern Display *gpdisp;

#define kfDtClearLine 1

// Windows only, for creating avis

#include <vfw.h>
class TBitmap;
class AviRecorder // avir
{
public:
	AviRecorder();
	~AviRecorder();

	bool Start(int cx, int cy, char *pszFn = NULL);
	void AddFrame(DibBitmap *pbm);
	void AddAudio(byte *pb8Unsigned, dword cb);
	void Stop();

private:
	PAVIFILE m_pavif;
	PAVISTREAM m_pstmVideo;
	PAVISTREAM m_pstmAudio;
	long m_tStart;
	int m_nSample;
	DibBitmap *m_pbmFlip;
	TBitmap *m_ptbmPointer;
	bool m_fAudioReady;
};
extern AviRecorder *gpavir;

void ReporterInit();
void ReporterExit();
void ReporterOutputDebugString(char *pszFormat, va_list va);

#include "../mixer.h"

class SoundDevice;
void SetSoundServiceDevice(SoundDevice *psndd);
SoundDevice *CreateWin32SoundDevice();
void PrependDataDirectory(char *pszIn, char *pszOut);

#define DebugBreak() _asm { int 3 }

extern HINSTANCE ghInst;
extern int gxPenLast;
extern int gyPenLast;
