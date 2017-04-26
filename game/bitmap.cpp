#include "ht.h"

#if defined(SDL)
#include <SDL.h>
#include <SDL_image.h>

#define SURFACE_FLAGS 0
#define SURFACE_DEPTH 32
#define SURFACE_PITCH(x) x*4 // row length in bytes: x*sizeof(byte)*sizeof(dword)
#define SHADOW_ALPHA_MOD 100
#endif // defined(SDL)

namespace wi {

#if defined(SDL)

DibBitmap::DibBitmap()
{
    m_surface = NULL;
    m_wf = 0;
    m_pszFn = NULL;
    m_renderer = NULL;
}

DibBitmap::~DibBitmap()
{
    SDL_DestroyRenderer(m_renderer);
    SDL_FreeSurface(m_surface);
    if (m_wf & kfDibFreeMem)
        free(m_surface->pixels);
    m_surface = NULL;
    if (m_pszFn != NULL)
        delete[] m_pszFn;
}

DibBitmap *LoadDibBitmap(char *pszFn)
{
    DibBitmap *pbm = new DibBitmap();
    Assert(pbm != NULL, "out of memory!");
	if (pbm == NULL)
		return NULL;
	if (!pbm->Init(pszFn)) {
		delete pbm;
		return NULL;
	}
	return pbm;
}

DibBitmap *CreateDibBitmap(dword *pb, int cx, int cy, bool alpha)
{
    DibBitmap *pbm = new DibBitmap();
	Assert(pbm != NULL, "out of memory!");
	if (pbm == NULL)
		return NULL;
	if (!pbm->Init(pb, cx, cy, alpha, false)) {
		delete pbm;
		return NULL;
	}
	return pbm;
}

DibBitmap *CreateBigDibBitmap(dword *pb, int cx, int cy, bool alpha)
{
    DibBitmap *pbm = new DibBitmap();
	Assert(pbm != NULL, "out of memory!");
	if (pbm == NULL)
		return NULL;
	if (!pbm->Init(pb, cx, cy, alpha, true)) {
		delete pbm;
		return NULL;
	}
	return pbm;
}

bool DibBitmap::Init(char *pszFn)
{
    // Read the file

    File *pfil = gpakr.fopen(pszFn, "rb");
    if (pfil == NULL)
        return false;
    byte *pb = new byte[pfil->cbTotal];

    if (gpakr.fread(pb, pfil->cbTotal, 1, pfil) != 1)
        return false;

    // Feed the bytes into an SDL surface

    SDL_RWops *prwio = SDL_RWFromMem(pb, pfil->cbTotal);
    m_surface = IMG_Load_RW(prwio, 0);

    // Cleanup

    SDL_FreeRW(prwio);
    delete[] pb;
    gpakr.fclose(pfil);

    // Keep the name around

    m_pszFn = AllocString(pszFn);

    return m_surface != NULL;
}

bool DibBitmap::Init(dword *pb, int cx, int cy, bool alpha, bool bigendian)
{
    Uint32 rmask, gmask, bmask, amask;
    if (bigendian) {
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
    } else {
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
    }
    if (!alpha)
        amask = 0;

    if (pb == NULL) {
        m_surface = SDL_CreateRGBSurface(SURFACE_FLAGS, cx, cy, SURFACE_DEPTH,
            rmask, gmask, bmask, amask);
    } else {
        m_surface = SDL_CreateRGBSurfaceFrom(pb, cx, cy, SURFACE_DEPTH, SURFACE_PITCH(cx),
            rmask, gmask, bmask, amask);

        // When using SDL_CreateRGBSurfaceFrom(), SDL does not manage the pixel data
        // so it will need to be freed manually

        m_wf |= kfDibFreeMem;
    }

#if 0
    // This can be used to set the alpha color (only works if amask is 0)
    // SDL_SetColorKey(m_surface, 1, SDL_MapRGB(m_surface->format, 255, 0, 255));
#endif

    return m_surface != NULL;
}

void DibBitmap::Blt(DibBitmap *pbmSrc, Rect *prcSrc, int xDst, int yDst)
{
    if (prcSrc == NULL) {
        SDL_Rect rcDst = { xDst, yDst, pbmSrc->GetSurface()->w, pbmSrc->GetSurface()->h };
        SDL_BlitSurface(pbmSrc->GetSurface(), NULL, m_surface, &rcDst);
    } else {
        SDL_Rect rcSrc = { prcSrc->left, prcSrc->top, prcSrc->Width(), prcSrc->Height() };
        SDL_Rect rcDst = { xDst, yDst, prcSrc->Width(), prcSrc->Height() };
        SDL_BlitSurface(pbmSrc->GetSurface(), &rcSrc, m_surface, &rcDst);
    }
}

void DibBitmap::BltTo(class DibBitmap *pbmDst, int xDst, int yDst, Rect *prcSrc)
{
    if (prcSrc == NULL) {
        SDL_Rect rcDst = { xDst, yDst, m_surface->w, m_surface->h };
        SDL_BlitSurface(m_surface, NULL, pbmDst->GetSurface(), &rcDst);
    } else {
        SDL_Rect rcSrc = { prcSrc->left, prcSrc->top, prcSrc->Width(), prcSrc->Height() };
        SDL_Rect rcDst = { xDst, yDst, prcSrc->Width(), prcSrc->Height() };
        SDL_BlitSurface(m_surface, &rcSrc, pbmDst->GetSurface(), &rcDst);
    }
}

void DibBitmap::BltTiles(DibBitmap *pbmSrc, UpdateMap *pupd, int yTopDst)
{
    bool fFirst = true;
    Rect rc;
    while (pupd->EnumUpdateRects(fFirst, NULL, &rc)) {
        fFirst = false;
        Blt(pbmSrc, &rc, rc.left, rc.top + yTopDst);
    }
}

void DibBitmap::GetSize(Size *psiz)
{
    psiz->cx = m_surface->w;
	psiz->cy = m_surface->h;
}

dword *DibBitmap::GetBits()
{
    return (dword *)m_surface->pixels;
}

int DibBitmap::GetPitch()
{
    return m_surface->pitch;
}

void DibBitmap::Fill(int x, int y, int cx, int cy, Color clr)
{
    SDL_Rect rc = { x, y, cx, cy };
    SDL_FillRect(m_surface, &rc, clr);
}

void DibBitmap::FillTo(class DibBitmap *pbmDst, int xDst, int yDst,
        int cxDst, int cyDst, int xOrigin, int yOrigin)
{
    Rect rcclp;

    // For every y point that is disivible by m_surface->h

    for (int y = 0; y < cyDst; y++) {
        if (!(y % m_surface->h)) {

            // For every x point that is disivible by m_surface->w

            for (int x = 0; x < cxDst; x++) {
                if (!(x % m_surface->w)) {

                    // Blt and clip appropriately

                    rcclp.Set(xDst + x, yDst + y, cxDst, cyDst);
                    pbmDst->Blt(this, NULL, xDst + x, yDst + y);
                }
            }
        }
    }
}

void DibBitmap::Clear(Color clr)
{
    Fill(0, 0, m_surface->w, m_surface->h, clr);
}

void DibBitmap::Shadow(int x, int y, int cx, int cy)
{
    DibBitmap *pbm = CreateDibBitmap(NULL, cx, cy);
    pbm->Clear(GetColor(kiclrBlack));

    SDL_SetSurfaceBlendMode(pbm->GetSurface(), SDL_BLENDMODE_BLEND);
    SDL_SetSurfaceAlphaMod(pbm->GetSurface(), SHADOW_ALPHA_MOD);

    pbm->BltTo(this, x, y);
}

void DibBitmap::DrawLine(short x1, short y1, short x2, short y2, Color clr)
{
    if (m_renderer == NULL)
        if ((m_renderer = SDL_CreateSoftwareRenderer(m_surface)) == NULL)
            return;

    byte r, g, b;
    SDL_GetRGB(clr, m_surface->format, &r, &g, &b);

    SDL_SetRenderDrawColor(m_renderer, r, g, b, 255);
    SDL_RenderDrawLine(m_renderer, x1, y1, x2, y2);
}

void DibBitmap::Scroll(Rect *prcSrc, int xDst, int yDst)
{
    // Some implementations do blts differently (such as rotated dibs).
    // That is why this method - essentially a blt from and to the
    // destination - exists.
    Blt(this, prcSrc, xDst, yDst);
}


DibBitmap *DibBitmap::Suballoc(int yTop, int cy)
{
    Assert(yTop < m_surface->h && yTop + cy <= m_surface->h);
	dword *pb = (dword *)m_surface->pixels + (long)m_surface->w * yTop;
    DibBitmap *pbm = CreateDibBitmap(pb, m_surface->w, cy);

    // We don't want it to free memory since that will be taken care
    // of by 'this'
    if (pbm != NULL)
        pbm->SetFlags(pbm->GetFlags() & ~kfDibFreeMem);

    return pbm;
}

dword DibBitmap::MapRGB(byte r, byte g, byte b)
{
    return SDL_MapRGB(m_surface->format, r, g, b);
}

#endif // defined(SDL)

} // namespace wi
