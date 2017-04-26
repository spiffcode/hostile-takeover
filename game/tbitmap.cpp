#include "ht.h"
#include <libgen.h>
#include <SDL.h>

namespace wi {

TBitmap::TBitmap()
{
    m_pszFn = NULL;
    m_x = 0;
    m_y = 0;
    m_cx = 0;
    m_cy = 0;
    m_anSideMap = NULL;
}

TBitmap::~TBitmap()
{
    if (m_anSideMap != NULL)
        delete[] m_anSideMap;
    m_anSideMap = NULL;
    if (m_pszFn != NULL)
        delete[] m_pszFn;
    m_pszFn = NULL;
}

TBitmap *CreateTBitmap(char *pszName) {
    return gptam->CreateTBitmap(pszName);
}

bool TBitmap::Init(char *pszFn, int x, int y, int cx, int cy, int cxOrig, int cyOrig,
    int ccLeft, int ccTop, int *anSideMap)
{
    m_pszFn = AllocString(pszFn);
    m_x = x;
    m_y = y;
    m_cx = cx;
    m_cy = cy;
    m_cxOrig = cxOrig;
    m_cyOrig = cyOrig;
    m_ccLeft = ccLeft;
    m_ccTop = ccTop;
    m_anSideMap = anSideMap;

    return true;
}

void TBitmap::BltTo(class DibBitmap *pbmDst, int xDst, int yDst, Rect *prcSrc)
{
    gptam->BltTo(this, pbmDst, xDst, yDst, (Side)1, prcSrc);
}

void TBitmap::BltTo(class DibBitmap *pbmDst, int xDst, int yDst, Side side, Rect *prcSrc)
{
    // Flash unit?

    if (side == (Side)-1) {
        SetLum(32768)->BltTo(pbmDst, xDst, yDst, prcSrc);
        return;
    }

    // Normal blt

    gptam->BltTo(this, pbmDst, xDst, yDst, side, prcSrc);
}

void TBitmap::GetTextureSize(Size *psiz)
{
    psiz->cx = m_cx;
    psiz->cy = m_cy;
}

void TBitmap::GetSize(Size *psiz)
{
    psiz->cx = m_cxOrig;
    psiz->cy = m_cyOrig;
}

void TBitmap::GetPosition(Point *ppos)
{
    ppos->x = m_x;
    ppos->y = m_y;
}

int TBitmap::GetAtlas(Side side)
{
    return m_anSideMap[(int)side];
}

int TBitmap::GetBaseline()
{
    // Todo
    return 0;
}

void TBitmap::FillTo(class DibBitmap *pbmDst, int xDst, int yDst,
        int cxDst, int cyDst, int xOrigin, int yOrigin)
{
    Rect rcclp;

    // For every y point that is disivible by m_cy

    for (int y = 0; y < cyDst; y++) {
        if (!(y % m_cy)) {

            // For every x point that is disivible by m_cx

            for (int x = 0; x < cxDst; x++) {
                if (!(x % m_cx)) {

                    // Blt and clip appropriately

                    rcclp.Set(xDst + x, yDst + y, cxDst, cyDst);
                    BltTo(pbmDst, xDst + x, yDst + y);
                }
            }
        }
    }
}

DibBitmap *TBitmap::SetLum(word lum, Side side) {

    // Copy pixels into an DibBitmap

    DibBitmap *pbm = CreateDibBitmap(NULL, m_cxOrig, m_cyOrig, true);
    BltTo(pbm, 0, 0, side);
    Uint32 *i0p = (Uint32 *)pbm->GetBits();

    byte r, g, b, a;
    word h, s, l;

    // Loop helpers

    int npixels = m_cxOrig * m_cyOrig;
    dword *ppixels = pbm->GetBits();
    dword *ppixel = ppixels;
    dword *end = ppixels + npixels;

    for (; ppixel < end; ppixel++, i0p++) {
        SDL_GetRGBA(*i0p, pbm->GetSurface()->format, &r, &g, &b, &a);
        RgbToHsl(r, g, b, &h, &s, &l);

        if (lum > 32768)
            lum = 32768;
        if (lum < 0)
            lum = 0;

        // Change the pixel value

        HslToRgb(h, s, lum, &r, &g, &b);
        *ppixel = SDL_MapRGBA(pbm->GetSurface()->format, r, g, b, a);
    }

    return pbm;
}

} // namespace wi
