// htplatform.h

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/time.h>
#include <stdio.h>
#include <SDL.h>
#include "game/sdl/sdlpackfile.h"
#include "game/sdl/sysmessages.h"

// To determine if running on simulator, the sdk sets TARGET_IPHONE_SIMULATOR
// to 0 or 1.
#if defined(IPHONE) || defined(__IPHONEOS__)
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR == 1
#define SIMULATOR
#endif
#endif

namespace wi {

#define MakeDword(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
#define BigWord(x) ((((x)&0xFF)<<8) | (((x)&0xFF00)>>8))
#define BigDword(x) ((((x)&0xFF)<<24) | (((x)&0xFF00)<<8) | (((x)&0xFF0000)>>8) | (((x)&0xFF000000)>>24))

typedef char TCHAR;
#define TEXT(str) str

#define _MAX_PATH PATH_MAX
#define MAX_PATH PATH_MAX

#define chrPageUp 0xb
#define chrPageDown 0xc
#define chrUp chrPageUp
#define chrDown chrPageDown
#define vchrMenu 0x105
#define vchrFind 0x10a
#define vchrCalc 0x10b
#define vchrBack 0x10c
#define penDownEvent 1
#define penUpEvent 2
#define penMoveEvent 3
#define keyDownEvent 4
#define penDownEvent2 5
#define penUpEvent2 6
#define penMoveEvent2 7
#define appStopEvent 22
#define chrBackspace 0x0008
#define chrDelete 0x007f
#define chrLeft 0x1000
#define chrRight 0x1001
#define chrUp chrPageUp
#define chrDown chrPageDown

#define    keyBitPower            0x0001        // Power key
#define    keyBitPageUp        0x0002        // Page-up
#define    keyBitPageDown        0x0004        // Page-down
#define    keyBitHard1            0x0008        // App #1
#define    keyBitHard2            0x0010        // App #2
#define    keyBitHard3            0x0020        // App #3
#define    keyBitHard4            0x0040        // App #4
#define    keyBitCradle        0x0080        // Button on cradle
#define    keyBitAntenna        0x0100        // Antenna "key" <chg 3-31-98 RM>
#define    keyBitContrast        0x0200        // Contrast key

// From PalmSG

#define keyBitDpadLeft        0x01000000    // d-pad left
#define keyBitDpadRight        0x02000000    // d-pad right
#define keyBitDpadButton    0x04000000    // d-pad center button

// From PalmSource

#define keyBitRockerUp          0x00010000      // 5-way rocker
#define keyBitRockerDown        0x00020000
#define keyBitRockerLeft        0x00040000
#define keyBitRockerRight       0x00080000
#define keyBitRockerCenter      0x00100000

#define    keyBitsAll            0xFFFFFFFF    // all keys

struct ModeInfo
{
    int cx;
    int cy;
    int cyGraffiti;
    int nDepth;
    bool fNative;
    int nDegreeOrientation;
};
#define kcmodesMax 16

class Rect;
class DibBitmap;
class UpdateMap;
class SpriteManager;
class FormMgr;

class Display // disp
{
public:
    Display();
    ~Display();

    bool Init();
    int GetModeCount();
    void GetModeInfo(int imode, ModeInfo *pmode);
    int GetMode(ModeInfo *pmode);
    bool SetMode(int imode);
    void DrawText(const char *psz, int x, int y, word wf);
    void DrawFrameInclusive(Rect *prc);
    DibBitmap *GetBackDib();
    DibBitmap *GetFrontDib();
    DibBitmap *GetClippingDib();
    void GetHslAdjustments(short *pnHueOffset, short *pnSatMultiplier, short *pnLumOffset);
    void FrameStart();
    void FrameComplete(int cfrmm, UpdateMap **apupd, Rect *arc, bool fScrolled);
    void ResetScrollOffset();
    SpriteManager *GetSpriteManager();
    void SetFormMgrs(FormMgr *pfrmmSim, FormMgr *pfrmmInput);
    void SetShouldRender(bool fsr);
    SDL_Renderer *Renderer();

private:
    int m_imode;
    ModeInfo m_amodeInfo[kcmodesMax];
    int m_cmodes;
    int m_cx;
    int m_cy;
    DibBitmap *m_pbm;
    DibBitmap *m_pbmClip;
    SDL_Window *m_window;
    SDL_Renderer *m_renderer;
    bool m_fShouldRender;
};

#define kfDtClearLine 1

extern Display *gpdisp;

class PlatformSprite {
public:
    virtual void Draw(void *pv, Size *psiz) = 0;
};

class SoundDevice;
SoundDevice *CreateSdlSoundDevice();
void PrependDataDirectory(char *pszIn, char *pszOut);

extern SdlPackFileReader gpakr;

} // namespace wi
