#include "ht.h"

#if defined(SDL)
#include <SDL.h>
#include <SDL_image.h>
#endif

namespace wi {

#if defined(SDL)

DibBitmap::DibBitmap()
{
    m_texture = NULL;
    m_ppfmt = NULL;
    m_wf = 0;
    m_cx = 0;
    m_cy = 0;
}

DibBitmap::~DibBitmap()
{
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = NULL;
    }
    if (m_ppfmt && m_ppfmt->next) {
        SDL_FreeFormat(m_ppfmt);
        m_ppfmt = NULL;
    }
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

DibBitmap *CreateDibBitmap(dword *pb, int cx, int cy)
{
    DibBitmap *pbm = new DibBitmap();
	Assert(pbm != NULL, "out of memory!");
	if (pbm == NULL)
		return NULL;
	if (!pbm->Init(pb, cx, cy)) {
		delete pbm;
		return NULL;
	}
	return pbm;
}

bool DibBitmap::Init(char *pszFn)
{
    // Open the file

    File *pfil = gpakr.fopen(pszFn, "rb");
    if (!pfil)
        return false;
    byte *pb = new byte[pfil->cbTotal];
    if (!pb)
        return false;

    // Read the file

    if (gpakr.fread(pb, pfil->cbTotal, 1, pfil) != 1) {
        gpakr.fclose(pfil);
        return false;
    }
    gpakr.fclose(pfil);

    // Copy bytes into an SDL texture

    SDL_RWops *prwio = SDL_RWFromMem(pb, pfil->cbTotal);
    m_texture = IMG_LoadTexture_RW(gpdisp->Renderer(), prwio, 1);
    if (!m_texture)
        return false;

    // Bytes have been copied into the texture and can be deleted

    delete[] pb;

    // Query the texture and save prevalent information

    dword fmt;
    SDL_QueryTexture(m_texture, &fmt, NULL, &m_cx, &m_cy);
    m_ppfmt = SDL_AllocFormat(fmt);

    return true;
}

bool DibBitmap::Init(dword *pb, int cx, int cy)
{
    // Assume RGBA mask

    Uint32 rmask = 0, gmask = 0, bmask = 0, amask = 0;
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;

    // Create the texture

    SDL_Renderer *renderer = gpdisp->Renderer();
    if (pb == NULL) {
        m_texture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, cx, cy);
    } else {

        // If creating with pb, copy bytes into SDL_Surface then create SDL_Texture from surface

        SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
            pb, cx, cy, 32, cx * sizeof(dword), rmask, gmask, bmask, amask);
        m_texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        surface = NULL;
    }

    if (!m_texture)
        return false;

    // Query the texture and save prevalent information

    dword fmt;
    SDL_QueryTexture(m_texture, &fmt, NULL, &m_cx, &m_cy);
    m_ppfmt = SDL_AllocFormat(fmt);
    if (!m_ppfmt)
        return false;

    return true;
}

SDL_Rect rcSrc;
SDL_Rect rcDst;
void DibBitmap::Blt(DibBitmap *pbmSrc, Rect *prcSrc, int xDst, int yDst)
{
    SDL_Renderer *renderer = gpdisp->Renderer();
    if (SDL_SetRenderTarget(renderer, m_texture) != 0) {
        LOG() << "SDL Error: " << SDL_GetError();
        return;
    }

    if (prcSrc != NULL) {
        rcSrc.x = prcSrc->left;
        rcSrc.y = prcSrc->top;
        rcSrc.w = prcSrc->Width();
        rcSrc.h = prcSrc->Height();
    }

    rcDst.x = xDst;
    rcDst.y = yDst;
    rcDst.w = prcSrc ? prcSrc->Width() : pbmSrc->Width();
    rcDst.h = prcSrc ? prcSrc->Height() : pbmSrc->Height();

    SDL_RenderCopy(renderer, pbmSrc->Texture(), prcSrc != NULL ? &rcSrc : NULL, &rcDst);
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
    psiz->cx = m_cx;
	psiz->cy = m_cy;
}

void DibBitmap::Fill(int x, int y, int cx, int cy, Color clr)
{
    SDL_Renderer *renderer = gpdisp->Renderer();
    SDL_SetRenderTarget(renderer, m_texture);

    SDL_Rect rc;
    rc.x = x;
    rc.y = y;
    rc.w = cx;
    rc.h = cy;

    SDL_SetRenderDrawColor(renderer, clr.r, clr.g, clr.b, 255);
    SDL_RenderFillRect(renderer, &rc);
}

void DibBitmap::Fill(int x, int y, int cx, int cy, dword clr)
{
    SDL_PixelFormat *pfmt = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
    if (pfmt) {
        byte r, g, b;
        SDL_GetRGB(clr, pfmt, &r, &g, &b);
        Color c = { r, g, b };
        Fill(x, y, cx, cy, c);
    }
}

void DibBitmap::FillTo(class DibBitmap *pbmDst, int xDst, int yDst,
        int cxDst, int cyDst, int xOrigin, int yOrigin)
{
    Rect rcclp;

    // For every y point that is disivible by cx

    for (int y = 0; y < cyDst; y++) {
        if (!(y % m_cy)) {

            // For every x point that is disivible by cy

            for (int x = 0; x < cxDst; x++) {
                if (!(x % m_cx)) {

                    // Blt and clip appropriately

                    rcclp.Set(xDst + x, yDst + y, cxDst, cyDst);
                    pbmDst->Blt(this, &rcclp, xDst + x, yDst + y);
                }
            }
        }
    }
}

void DibBitmap::Clear(Color clr)
{
    Fill(0, 0, m_cx, m_cy, clr);
}

void DibBitmap::Shadow(int x, int y, int cx, int cy)
{
    DibBitmap *pbm = CreateDibBitmap(NULL, cx, cy);
    pbm->Clear(GetColor(kiclrBlack));

    SDL_SetTextureBlendMode(pbm->Texture(), SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(pbm->Texture(), 100);

    Blt(pbm, NULL, x, y);

    SDL_SetTextureBlendMode(pbm->Texture(), SDL_BLENDMODE_NONE);
}

void DibBitmap::DrawLine(short x1, short y1, short x2, short y2, Color clr)
{
    SDL_Renderer *renderer = gpdisp->Renderer();
    SDL_SetRenderTarget(renderer, m_texture);

    SDL_SetRenderDrawColor(renderer, clr.r, clr.g, clr.b, 255);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

void DibBitmap::Scroll(Rect *prcSrc, int xDst, int yDst)
{
    // Some implementations do blts differently (such as rotated dibs).
    // That is why this method - essentially a blt from and to the
    // destination - exists.
    Blt(this, prcSrc, xDst, yDst);
}

SubBitmap *DibBitmap::Suballoc(int yTop, int cy)
{
    Assert(yTop < m_cy && yTop + cy <= m_cy);
    Rect rc;
    rc.Set(0, yTop, m_cx, yTop + cy);
    return Suballoc(rc);
}

SubBitmap *DibBitmap::Suballoc(Rect rc)
{
    Assert(rc.right <= m_cx && rc.top <= m_cy);

    SubBitmap *pbm = new SubBitmap(rc);
    if (!pbm) {
        delete pbm;
        return NULL;
    }

    pbm->m_texture = m_texture;
    pbm->m_ppfmt = m_ppfmt;
    pbm->m_cx = rc.Width();
    pbm->m_cy = rc.Height();

    return pbm;
}

#endif // defined(SDL)

// SubBitmap

void SubBitmap::Blt(DibBitmap *pbmSrc, Rect *prcSrc, int xDst, int yDst)
{
    DibBitmap::Blt(pbmSrc, prcSrc, xDst + m_rc.left, yDst + m_rc.top);
}

void SubBitmap::Fill(int x, int y, int cx, int cy, Color clr)
{
    DibBitmap::Fill(x + m_rc.left, y + m_rc.top, cx, cy, clr);
}

void SubBitmap::Clear(Color clr)
{
    DibBitmap::Fill(m_rc.left, m_rc.top, m_rc.Width(), m_rc.Height(), clr);
}

void SubBitmap::DrawLine(short x1, short y1, short x2, short y2, Color clr)
{
    DibBitmap::DrawLine(x1 + m_rc.left, y1 + m_rc.top, x2 + m_rc.left, y2 + m_rc.top, clr);
}

} // namespace wi
