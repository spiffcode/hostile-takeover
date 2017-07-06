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
	m_pbmClip = NULL;

    m_window = NULL;
    m_renderer = NULL;
    m_pbm = NULL;

    m_fShouldRender = false;
}

Display::~Display()
{
	delete m_pbmClip;
	m_pbmClip = NULL;

    delete m_pbm;
    m_pbm = NULL;

    SDL_DestroyWindow(m_window);
    m_window = NULL;
    SDL_DestroyRenderer(m_renderer);
    m_renderer = NULL;
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

    // Get surface properties

    SurfaceProperties props;
    HostHelpers::GetSurfaceProperties(&props);
    m_cx = props.cxWidth;
    m_cy = props.cyHeight;

    // Set appropriate GL attributes

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    // Create window
    if ((m_window = SDL_CreateWindow("Hostile Takeover", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_cx, m_cy, videoflags)) == NULL) {
        LOG() << "Couldn't create window: " << m_cx << "x" << m_cy << " \nError: " << SDL_GetError();
        return false;
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_ACCELERATED);
    this->SetShouldRender(true);

    // Keep the screen size around

    s_siz.cx = m_cx;
    s_siz.cy = m_cy;

    ModeInfo *pmode = m_amodeInfo;
    
#if 1
    pmode->nDepth = 24;
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

    DibBitmap *pbm = CreateDibBitmap(NULL, m_cx, m_cy);
	if (pbm == NULL) {
		return NULL;
	}
	delete m_pbm;
	m_pbm = pbm;
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
    return m_pbm;
}

DibBitmap *Display::GetFrontDib()
{
    return m_pbm;
}

DibBitmap *Display::GetClippingDib()
{
    return NULL;
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
}

void Display::FrameComplete(int cfrmm, UpdateMap **apupd, Rect *arc,
        bool fScrolled)
{
    if (!m_fShouldRender)
        return;

    // Draw any sprites onto the texture

    SDL_SetRenderTarget(m_renderer, m_pbm->Texture());
    s_psprm->DrawSprites(m_renderer, s_siz);

    // Present the renderer to the screen

    SDL_SetRenderTarget(m_renderer, NULL);
    SDL_RenderCopy(m_renderer, m_pbm->Texture(), NULL, NULL);
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

void Display::SetShouldRender(bool fsr) {
    m_fShouldRender = fsr;
}

SDL_Renderer *Display::Renderer() {
    return m_renderer;
}

} // namespace wi
