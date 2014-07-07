#include "../ht.h"
#include "game/sdl/sdlspritemgr.h"
#include "hosthelpers.h"

namespace wi {

Display *HostCreateDisplay() {
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

Display::Display()
{
	m_cx = 0;
	m_cy = 0;
	memset(m_amodeInfo, 0, sizeof(m_amodeInfo));
	m_imode = -1;
	m_cmodes = 0;
	m_pbmBack = NULL;
	m_pbmFront = NULL;
	m_pbmClip = NULL;
}

Display::~Display()
{
	delete m_pbmBack;
	m_pbmBack = NULL;
	delete m_pbmFront;
	m_pbmFront = NULL;
	delete m_pbmClip;
	m_pbmClip = NULL;
}

// TODO(darrinm): horrible hack to see if it works (screen->map->dst = NULL)
// Alternatively:
// { SDL_SetSurfaceRLE(surface, 1); SDL_SetSurfaceRLE(surface, 0); }
#define SDL_InvalidateMap(surface) *(void **)surface->map = NULL;


bool Display::Init()
{
    Uint32 videoflags;
    int cxScreen, cyScreen;

    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        LOG() << "Couldn't initialize SDL: " << SDL_GetError();
        return false;
    }

	// Absolutely do not mess with SDL_FULLSCREEN if there is any chance the app
	// will crash or stop at a breakpoint. If it does you will be lost in full
	// screen mode! (ssh from another machine and kill the Xcode process)
    videoflags = SDL_SWSURFACE;

#ifdef __IPHONEOS__
    cxScreen = 320;
    cyScreen = 480;
#else
    cxScreen = 800;
    cyScreen = 600;
#endif

    /* Set 640x480 video mode */
    if ((screen =
         SDL_SetVideoMode(cxScreen, cyScreen, 8, videoflags)) == NULL) {
        LOG() << "Couldn't set " << cxScreen << "x" << cyScreen << " 8 video mode: " << SDL_GetError();
        return false;
    }

    SurfaceProperties props;
    props.cxWidth = cxScreen;
    props.cyHeight = cyScreen;
    props.cbxPitch = 1;
    props.cbyPitch = props.cxWidth;
    props.ffFormat = wi::kfDirect8;

    ModeInfo *pmode = m_amodeInfo;
    
#if 1
    pmode->nDepth = 8;
    pmode->cx = props.cxWidth;
    pmode->cy = props.cyHeight;
    pmode->cyGraffiti = 0;
    pmode->fNative = true;
    pmode->nDegreeOrientation = 0;
    pmode++;
#endif

#if 0
    pmode->nDepth = 8;
    pmode->cx = props.cyHeight;
    pmode->cy = props.cxWidth;
    pmode->cyGraffiti = 0;
    pmode->fNative = true;
    pmode->nDegreeOrientation = 90; // leftie controls
    pmode++;

    pmode->nDepth = 8;
    pmode->cx = props.cxWidth;
    pmode->cy = props.cyHeight;
    pmode->cyGraffiti = 0;
    pmode->fNative = true;
    pmode->nDegreeOrientation = 180; // bizzaro controls
    pmode++;
#endif
    
#if 0
    pmode->nDepth = 8;
    pmode->cx = props.cyHeight;
    pmode->cy = props.cxWidth;
    pmode->cyGraffiti = 0;
    pmode->fNative = true;
    pmode->nDegreeOrientation = 270; // rightie controls
    pmode++;
#endif

    m_cmodes = pmode - m_amodeInfo;

    HostOutputDebugString("Display::Init %d", pmode - m_amodeInfo);
    HostOutputDebugString("Display::Init %d modes", m_cmodes);
    return true;
}

void Display::SetPalette(Palette *ppal)
{
	SDL_Color aclr[256];
	int cEntries = BigWord(ppal->cEntries);
	byte *pb = (byte *)ppal->argb;
	SDL_Color *pclr = aclr;
	
	for (int i = 0; i < cEntries; i++) {
		pclr->r = *pb++;
		pclr->g = *pb++;
		pclr->b = *pb++;
		pclr++;
	}

	SDL_SetColors(screen, aclr, 0, cEntries);
	SDL_InvalidateMap(screen);
//    SDL_UpdateRect(screen, 0, 0, 0, 0);
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

bool Display::SetMode(int imode)
{
	// Allocate dib

	ModeInfo *pmode = &m_amodeInfo[imode];

	DibBitmap *pbmBack = CreateDibBitmap(NULL, pmode->cx, pmode->cy);
	if (pbmBack == NULL)
		return false;

	// TODO(darrinm): uh, locking, unlocking pixels?
	DibBitmap *pbmFront = CreateDibBitmap((byte *)screen->pixels, pmode->cx, pmode->cy);
	if (pbmFront == NULL) {
		delete pbmBack;
		return NULL;
	}
	delete m_pbmBack;
	delete m_pbmFront;
	m_pbmBack = pbmBack;
	m_pbmFront = pbmFront;
	m_imode = imode;

	return true;
}

void Display::DrawText(const char *psz, int x, int y, word wf)
{
}

void Display::DrawFrameInclusive(Rect *prc)
{
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
	DibBitmap *pbm = CreateDibBitmap(NULL, kcCopyBy4Procs * 4, kcCopyBy4Procs * 4);
	if (pbm == NULL)
		return NULL;
	m_pbmClip = pbm;
	return pbm;
}

void Display::GetHslAdjustments(short *pnHueOffset, short *pnSatMultiplier, short *pnLumOffset)
{
	*pnHueOffset = 0;
	*pnSatMultiplier = 0;
	*pnLumOffset = 0;
}

void Display::FrameStart()
{
	SDL_LockSurface(screen);
	
	// screen->pixels can change every time the surface is locked.
	// TODO(darrinm): problem for, e.g. scrolling optimizations?
	m_pbmFront->Init((byte *)screen->pixels, screen->w, screen->h);
}

void Display::FrameComplete(int cfrmm, UpdateMap **apupd, Rect *arc,
        bool fScrolled)
{
	SDL_UnlockSurface(screen);
    SDL_UpdateRect(screen, 0, 0, 0, 0);
}

void Display::ResetScrollOffset()
{
#if 0 // TODO(darrinm): SDL-ify
    IPhone::ResetScrollOffset();
#endif
}

static SdlSpriteManager *s_psprm;

SpriteManager *Display::GetSpriteManager()
{
	if (s_psprm == NULL)
		s_psprm = new SdlSpriteManager();
	return s_psprm;
}

void Display::SetFormMgrs(FormMgr *pfrmmSimUI, FormMgr *pfrmmInput)
{
#if 0 // TODO(darrinm): SDL-ify
    IPhone::SetFormMgrs(pfrmmSimUI, pfrmmInput);
#endif
}

} // namespace wi
