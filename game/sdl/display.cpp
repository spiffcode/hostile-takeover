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
    
    m_gameDisplay = NULL;
    m_gameSurfacePixels = NULL;
    m_gameSurface = NULL;
    m_pixelCount = 0;
}

Display::~Display()
{
	delete m_pbmBack;
	m_pbmBack = NULL;
	delete m_pbmFront;
	m_pbmFront = NULL;
	delete m_pbmClip;
	m_pbmClip = NULL;
    
    delete m_gameDisplay;
    m_gameDisplay = NULL;
    
    m_pixelCount = 0;
    m_gameSurfacePixels = NULL;
    SDL_FreeSurface(m_gameSurface);
    m_gameSurface = NULL;
    SDL_DestroyWindow(window);
    window = NULL;
    SDL_DestroyTexture(texture);
    texture = NULL;
}

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
    videoflags = SDL_SWSURFACE | SDL_WINDOW_ALLOW_HIGHDPI;

    SurfaceProperties props;
    HostHelpers::GetSurfaceProperties(&props);
    cxScreen = props.cxWidth;
    cyScreen = props.cyHeight;
    m_density = props.density;
    
    // Create window
    if ((window = SDL_CreateWindow("Hostile Takeover", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, cxScreen, cyScreen, videoflags)) == NULL) {
        LOG() << "Couldn't create window: " << cxScreen << "x" << cyScreen << " \nError: " << SDL_GetError();
        return false;
    }

    renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_TARGETTEXTURE);
    this->SetShouldRender(true);

    // Keep the screen size around
    s_siz.cx = cxScreen;
    s_siz.cy = cyScreen;

    m_gameSurface = SDL_CreateRGBSurface(videoflags, cxScreen, cyScreen, 32, 0, 0, 0, 0);
    m_pixelCount = cxScreen * cyScreen;
    m_gameDisplay = (byte *)malloc(m_pixelCount);
    m_gameSurfacePixels = (Uint32 *)m_gameSurface->pixels;

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
        m_32bppColors[i] = ((Uint8)aclr[i].r << 16) | ((Uint8)aclr[i].g << 8) | ((Uint8)aclr[i].b << 0);
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

    DibBitmap *pbmFront = CreateDibBitmap(m_gameDisplay, pmode->cx, pmode->cy);
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
    m_pbmFront->Init(m_gameDisplay, m_gameSurface->w, m_gameSurface->h);
}

void Display::FrameComplete(int cfrmm, UpdateMap **apupd, Rect *arc,
        bool fScrolled)
{
    for (int i = 0; i < m_pixelCount; i++) {
        m_gameSurfacePixels[i] = m_32bppColors[m_gameDisplay[i]];
    }

    RenderGameSurface();
}

void Display::RenderGameSurface() {
    if (!m_fshouldRender)
        return;

    // Create the texture
    texture = SDL_CreateTextureFromSurface(renderer, m_gameSurface);

    // Prepare the renderer
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    // Draw on any sprites
    s_psprm->DrawSprites(renderer, s_siz);

    // Present the renderer
    SDL_RenderPresent(renderer);

    // Free memory from the created texture
    SDL_DestroyTexture(texture);
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
    m_fshouldRender = fsr;
}

} // namespace wi
