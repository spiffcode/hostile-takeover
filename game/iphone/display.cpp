#include "../ht.h"

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

bool Display::Init()
{
    SurfaceProperties props;
    IPhone::GetSurfaceProperties(&props);

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
    IPhone::SetPalette(ppal);
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
	DibBitmap *pbmFront = IPhone::CreateFrontDib(pmode->cx, pmode->cy, pmode->nDegreeOrientation);
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
    IPhone::FrameStart();
}

void Display::FrameComplete(int cfrmm, UpdateMap **apupd, Rect *arc,
        bool fScrolled)
{
    IPhone::FrameComplete(cfrmm, apupd, arc, fScrolled);
}

void Display::ResetScrollOffset()
{
    IPhone::ResetScrollOffset();
}

SpriteManager *Display::GetSpriteManager()
{
    return IPhone::GetSpriteManager();
}

void Display::SetFormMgrs(FormMgr *pfrmmSimUI, FormMgr *pfrmmInput)
{
    IPhone::SetFormMgrs(pfrmmSimUI, pfrmmInput);
}

} // namespace wi
