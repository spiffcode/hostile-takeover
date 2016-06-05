#include "../ht.h"
#include "game/sdl/sdlspritemgr.h"
#include "game/sdl/hosthelpers.h"
#include "game/sdl/sdlspritemgr.h"

namespace wi {

static SdlSpriteManager *s_psprm;
static Size s_siz;

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

    m_window = NULL;
    m_renderer = NULL;
    m_texture = NULL;

    m_gamePixels = NULL;
    m_gamePixels32 = NULL;
    memset(m_palette, 0, sizeof(m_palette));
    m_pitch32 = 0;
    m_pixelCount = 0;

    m_density = 0;
    m_fShouldRender = false;
}

Display::~Display()
{
	delete m_pbmBack;
	m_pbmBack = NULL;
	delete m_pbmFront;
	m_pbmFront = NULL;
	delete m_pbmClip;
	m_pbmClip = NULL;

    SDL_DestroyWindow(m_window);
    m_window = NULL;
    SDL_DestroyRenderer(m_renderer);
    m_renderer = NULL;
    SDL_DestroyTexture(m_texture);
    m_texture = NULL;

    free(m_gamePixels);
    m_gamePixels = NULL;
    free(m_gamePixels32);
    m_gamePixels32 = NULL;
}

bool Display::Init()
{
    dword videoflags;

    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        LOG() << "Couldn't initialize SDL: " << SDL_GetError();
        return false;
    }

	// Absolutely do not mess with SDL_FULLSCREEN if there is any chance the app
	// will crash or stop at a breakpoint. If it does you will be lost in full
	// screen mode! (ssh from another machine and kill the Xcode process)
    videoflags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN;
    #if defined(__IPHONEOS__) || defined(__ANDROID__)
    videoflags = videoflags | SDL_WINDOW_BORDERLESS;
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeRight LandscapeLeft");
    #endif

    // Android wants to process mouse and touch events separately
    #if defined(__ANDROID__)
    SDL_SetHint(SDL_HINT_ANDROID_SEPARATE_MOUSE_AND_TOUCH, "1");
    #endif

    // Get surface properties
    SurfaceProperties props;
    HostHelpers::GetSurfaceProperties(&props);
    m_cx = props.cxWidth;
    m_cy = props.cyHeight;
    m_density = props.density;
    
    // Create window
    if ((m_window = SDL_CreateWindow("Hostile Takeover", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_cx, m_cy, videoflags)) == NULL) {
        LOG() << "Couldn't create window: " << m_cx << "x" << m_cy << " \nError: " << SDL_GetError();
        return false;
    }

    // Create renderer
    m_renderer = SDL_CreateRenderer(m_window, 0, SDL_RENDERER_TARGETTEXTURE);
    this->SetShouldRender(true);

    // Keep the screen size around
    s_siz.cx = m_cx;
    s_siz.cy = m_cy;

    // Create texture
    m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, m_cx, m_cy);

    // Set pixel info
    m_pixelCount = m_cx * m_cy;
    m_gamePixels = (byte *)malloc(m_pixelCount);
    m_gamePixels32 = (dword *)malloc(m_pixelCount * 4);
    m_pitch32 = m_cx * sizeof(dword);

    // Keep the screen size around
    s_siz.cx = m_cx;
    s_siz.cy = m_cy;

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
#endif

#if 0
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

#if defined(__IPHONEOS__)
    HostHelpers::DisplayInitComplete();
#endif
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
    
    for (int i = 0; i < cEntries; i++) {
        m_palette[i] = ((Uint8)aclr[i].r << 16) | ((Uint8)aclr[i].g << 8) | ((Uint8)aclr[i].b << 0);
    }
    
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

    DibBitmap *pbmFront = CreateDibBitmap(m_gamePixels, pmode->cx, pmode->cy);
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
    // surface->pixels can change every time the surface is locked.
	// TODO(darrinm): problem for, e.g. scrolling optimizations?
    m_pbmFront->Init(m_gamePixels, m_cx, m_cy);
}

void Display::FrameComplete(int cfrmm, UpdateMap **apupd, Rect *arc,
        bool fScrolled)
{
    for (int i = 0; i < m_pixelCount; i++) {
        m_gamePixels32[i] = m_palette[m_gamePixels[i]];
    }

    RenderGameSurface();
}

void Display::RenderGameSurface() {
    if (!m_fShouldRender)
        return;

    // Update the texture
    SDL_UpdateTexture(m_texture, NULL, m_gamePixels32, m_pitch32);

    // Draw any sprites onto the texture
    SDL_SetRenderTarget(m_renderer, m_texture);
    s_psprm->DrawSprites(m_renderer, s_siz);
    SDL_SetRenderTarget(m_renderer, NULL);

    // Present the renderer to the screen
    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
    SDL_RenderPresent(m_renderer);
}

void Display::ResetScrollOffset()
{
#if 0 // TODO(darrinm): SDL-ify
    IPhone::ResetScrollOffset();
#endif
}

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

float Display::Density() {
    return m_density;
}

void Display::SetShouldRender(bool fsr) {
    m_fShouldRender = fsr;
}

} // namespace wi
